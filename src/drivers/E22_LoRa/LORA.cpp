/*
 * LORA.cpp
 *
 * Implementação atualizada para o Módulo LORA - E22 (Série T)
 */

#include "LORA.hh"
#include <string.h>
#include <stdio.h>

// --- Construtor e Destrutor ---
LORA::LORA(UART_HandleTypeDef* huart, PinName aux, PinName m0, PinName m1)
{
    this->_huart = huart;
    this->_aux   = aux;
    this->_m0    = m0;
    this->_m1    = m1;

    this->dataAvailableFlag = false;
    this->txBusy            = false;
    this->auxFlag           = true;
    this->lastPacketSize    = 0;

    this->_rssiEnabled    = false;
	this->_lastRssiRaw    = 0;
	this->_lastPacketRssi = 0.0f;

    this->_targetBaudRate = 9600;
    this->_targetParity = UART_PARITY_NONE;

    this-> _currentMode = MODE_NORMAL;
}

LORA::~LORA() {}

//////////////////////////////////// Habilita o DMA de Recepção ////////////////////////////
HAL_StatusTypeDef LORA::enableReception()
{
    HAL_UART_AbortReceive(this->_huart);
    memset(this->rxTempBuffer, 0, E22_MAX_RX_BUFFER);
    return HAL_UARTEx_ReceiveToIdle_DMA(this->_huart, this->rxTempBuffer, E22_MAX_RX_BUFFER);
}

//////////////////////////////////// Desabilita o DMA de Recepção ////////////////////////////
HAL_StatusTypeDef LORA::disableReception()
{
    HAL_StatusTypeDef status = HAL_UART_DMAStop(_huart);
    HAL_UART_AbortReceive(_huart);
    __HAL_UART_FLUSH_DRREGISTER(_huart);
    return status;
}

//////////////////////////////////// Interrupção (Callback) ////////////////////////////
void LORA::handleRXInterrupt(uint16_t Size)
{
    // Informa o tamanho do pacote recebido pela HAL
    this->lastPacketSize = (Size > E22_MAX_RX_BUFFER) ? E22_MAX_RX_BUFFER : Size;
    // Copia os dados do buffer de recepcao para o de consumo
    memcpy(this->rxAppBuffer, this->rxTempBuffer, this->lastPacketSize);
    // Sinaliza dado Novo
    this->dataAvailableFlag = true;
    // Limpa buffer de recepcao
    memset(this->rxTempBuffer, 0, E22_MAX_RX_BUFFER);
    // Rearma o DMA
    __HAL_UART_FLUSH_DRREGISTER(this->_huart);
    HAL_UARTEx_ReceiveToIdle_DMA(this->_huart, this->rxTempBuffer, E22_MAX_RX_BUFFER);
    return;
}

//////////////////////////////////// Flag de Dado Disponível ////////////////////////////
bool LORA::isDataAvailable()
{
    return this->dataAvailableFlag;
}

//////////////////////////////////// Resgata o Dado do Buffer ////////////////////////////
uint16_t LORA::readData(uint8_t* userBuffer, uint16_t maxSize)
{
    if (!this->dataAvailableFlag) return 0;

    uint16_t actualPayloadLength = this->lastPacketSize;

    if (this->_rssiEnabled && actualPayloadLength > 0)
	{
		this->_lastRssiRaw = this->rxAppBuffer[actualPayloadLength - 1];
		// Fórmula: dBm = -RSSI / 2
		this->_lastPacketRssi = -((float)this->_lastRssiRaw / 2.0f);
		// Remove do dado
		actualPayloadLength -= 1;
	}
    // Garante tamanho que caiba
    uint16_t copySize = (actualPayloadLength > maxSize) ? maxSize : actualPayloadLength;
    // Copia o dado do buffer de recepcao para o de saída
    memcpy(userBuffer, this->rxAppBuffer, copySize);
    // Limpa a Flag
    dataAvailableFlag = false;
    // Retorna quantos dados foram copiados
    return copySize;
}

//////////////////////////////////// Inicializa o LORA E22 ////////////////////////////
HAL_StatusTypeDef LORA::init (uint16_t address, uint8_t netID, uint8_t channel, E22_AirRate airRate, E22_Power power, bool enableRssi)
{
    printf("LORA E22 Init...\n\r");

    this->_targetBaudRate = 9600;
    this->_rssiEnabled = enableRssi;

    if(waitForAuxHigh(1000) != HAL_OK) return HAL_TIMEOUT;

    E22_Config_s config;
    memset(&config, 0, sizeof(E22_Config_s)); // Limpa lixos de memória

    // 1. Identificação
    config.addh  = (uint8_t)(address >> 8);
    config.addl  = (uint8_t)(address & 0xFF); // endereço do dispositivo na rede
    config.netid = netID; // ID da rede (0 a 255) para distinguir no mesmo channel
    config.chan  = channel; // Frequencia é (850.125 + channel*1MHZ)

    // 2. REG0: Velocidades (Baud Rate, Paridade, Air Rate)
    config.reg0.uartParity   = E22_PARITY_8N1;
    config.reg0.uartBaudRate = E22_BAUD_9600;
    config.reg0.airDataRate  = airRate;

    // 3. REG1: Tamanho do pacote, Ruído, Potência
    config.reg1.packetSize        = E22_PACKET_240;
    config.reg1.rssiNoiseEnable   = enableRssi ? 1 : 0;
    config.reg1.reserved          = 0;
    config.reg1.transmissionPower = power;

    // 5. REG3: Opções avançadas (WOR, LBT, RSSI Byte)
    config.reg3.rssiByteEnable    = enableRssi ? 1 : 0;
    config.reg3.fixedTransmission = E22_MODE_FIXED;
    config.reg3.relayEnable       = 0;
    config.reg3.lbtEnable         = 0; // Listen Before Talk
    config.reg3.worRole           = 0; // Receptor
    config.reg3.worCycle          = 3; // 2000ms

    // 6. Criptografia
    config.crypt_h = 0x00;
    config.crypt_l = 0x00;

    if(setConfig(config) != HAL_OK)
    {
    	printf("ERRO: Falha ao aplicar configuracao inicial no E22.\n\r");
    	setMode(MODE_DEEP_SLEEP);
        return HAL_ERROR;
    }

    setMode(MODE_NORMAL);
    enableReception();
    this->auxFlag = getAuxStatus();
    return HAL_OK;
}

//////////////////////////////////// Pergunta se o TX acabou ////////////////////////////
bool LORA::isTxReady()
{
    return (!this->txBusy && getAuxStatus() == true);
}

//////////////////////////////////// Reseta flag de TX ////////////////////////////
void LORA::handleTxInterrupt()
{
    this->txBusy = false;
}

//////////////////////////////////// Envia uma mensagem ponto a ponto ////////////////////////////
HAL_StatusTypeDef LORA::sendMessage(uint16_t address, uint8_t channel, uint8_t* data, uint8_t length) 
{
    if (this->txBusy) return HAL_BUSY;

    if (!this->auxFlag || !getAuxStatus()) return HAL_BUSY;

    // Subtrai o cabecalho
    if (length > (E22_MAX_PAYLOAD_SIZE - 3)) return HAL_ERROR;

    // O E22 utiliza o mesmo padrão do E32 para transmissões fixas (3 bytes de cabeçalho)
    this->txBuffer[0] = (uint8_t)(address >> 8);
    this->txBuffer[1] = (uint8_t)(address & 0xFF);
    this->txBuffer[2] = channel;
    memcpy(&(this->txBuffer[3]), data, length);

    this->txBusy = true;
    this->auxFlag = false;

    if (HAL_UART_Transmit_DMA(_huart, this->txBuffer, length + 3) != HAL_OK)
    {
        this->txBusy = false;
        return HAL_ERROR;
    }

    return HAL_OK;
}

//////////////////////////////////// Envia mensagem Broadcast ////////////////////////////
HAL_StatusTypeDef LORA::sendBroadcastMessage(uint8_t channel, uint8_t* data, uint8_t length) 
{
    // O endereço de broadcast no E22 é 0xFFFF
    return sendMessage(0xFFFF, channel, data, length);
}

//////////////////////////////////// Grava a Configuração no Módulo ////////////////////////////
HAL_StatusTypeDef LORA::setConfig(E22_Config_s config, bool temporary)
{
    //disableReception();
	LoraMode prevMode = this->_currentMode;

    if (setMode(MODE_SLEEP_CONFIG) != HAL_OK)
    {
        printf("Falha ao entrar no modo de configuracao.\n\r");
        setMode(prevMode);
        return HAL_ERROR;
    }
    HAL_Delay(10);

    this->dataAvailableFlag = false;
	this->lastPacketSize = 0;
	enableReception(); // Inicia o DMA RX

    // Comando (0xC0) + Endereço Inicial (0x00) + Tamanho (9 bytes) + Payload (9 bytes)
    uint8_t cmd[12];
    cmd[0] = (temporary == true) ? E22_CMD_WRITE_TEMP_REG : E22_CMD_WRITE_REG; // 0xC0 ou 0xC2
    cmd[1] = 0x00;              // Inicia a gravação no ADDH
    cmd[2] = sizeof(E22_Config_s);
    memcpy(&cmd[3], &config, sizeof(E22_Config_s));

    if (HAL_UART_Transmit(_huart, cmd, 12, E22_TX_TIMEOUT) != HAL_OK)
    {
        printf("Falha ao enviar comando de configuracao.\n\r");
        setMode(prevMode);
        return HAL_ERROR;
    }

    // A resposta esperada tem 12 bytes: 0xC1 + 0x00 + 0x09 + [9 bytes gravados]
    uint32_t tickstart = HAL_GetTick();
	while (!this->dataAvailableFlag)
	{
		if ((HAL_GetTick() - tickstart) > E22_TX_TIMEOUT)
		{
			printf("Timeout DMA: Sem resposta do modulo na configuracao.\n\r");
			setMode(prevMode);
			return HAL_TIMEOUT;
		}
	}

    // Verifica se a resposta do módulo corresponde ao que enviamos
	if (this->rxAppBuffer[0] == E22_CMD_READ_REG && this->lastPacketSize >= 12)
	{
		if (memcmp(&cmd[3], &this->rxAppBuffer[3], 9) == 0)
		{
			printf("E22: Configuracao aplicada e confirmada!\n\r");
			setMode(prevMode);
			return HAL_OK;
		}
	}

	printf("Erro: Resposta de config invalida ou incompleta.\n\r");
	setMode(prevMode);
	return HAL_ERROR;
}

//////////////////////////////////// Configura a UART do LORA e do STM32 ////////////////////////////

HAL_StatusTypeDef LORA::setUartConfig(E22_BaudRate baudEnum, E22_Parity parity)
{
    E22_Config_s config;

    // Lemos a configuração atual
    if(readConfig(&config) != HAL_OK) return HAL_ERROR;

    // Alteramos apenas os parâmetros da UART
    config.reg0.uartBaudRate = baudEnum;
    config.reg0.uartParity   = parity;

    // Gravamos no módulo
    if(setConfig(config) != HAL_OK) return HAL_ERROR;

    // Agora para o STM32
    switch(baudEnum) {
        case E22_BAUD_1200:   this->_targetBaudRate = 1200; break;
        case E22_BAUD_2400:   this->_targetBaudRate = 2400; break;
        case E22_BAUD_4800:   this->_targetBaudRate = 4800; break;
        case E22_BAUD_9600:   this->_targetBaudRate = 9600; break;
        case E22_BAUD_19200:  this->_targetBaudRate = 19200; break;
        case E22_BAUD_38400:  this->_targetBaudRate = 38400; break;
        case E22_BAUD_57600:  this->_targetBaudRate = 57600; break;
        case E22_BAUD_115200: this->_targetBaudRate = 115200; break;
    }

    switch(parity)
    {
		case E22_PARITY_8N1: this->_targetParity = UART_PARITY_NONE; break;
		case E22_PARITY_8O1: this->_targetParity = UART_PARITY_ODD; break;
		case E22_PARITY_8E1: this->_targetParity = UART_PARITY_EVEN; break;
	}

    return HAL_OK;
}

//////////////////////////////////// Helper: Reconfigura a HAL ////////////////////////

void LORA::changeHardwareUartConfig(uint32_t baud, uint32_t parity)
{
    if (_huart->Init.BaudRate == baud && _huart->Init.Parity == parity) return;

    disableReception();

    _huart->Init.BaudRate = baud;
    _huart->Init.Parity = parity;

    if (parity == UART_PARITY_NONE) {
        _huart->Init.WordLength = UART_WORDLENGTH_8B;
    } else {
        _huart->Init.WordLength = UART_WORDLENGTH_9B;
    }

    if (HAL_UART_Init(_huart) != HAL_OK) {
        printf("ERRO: Falha ao reconfigurar HAL UART!\n\r");
    }

    enableReception();
}

//////////////////////////////////// Lê a Configuração do Módulo ////////////////////////////

HAL_StatusTypeDef LORA::readConfig(E22_Config_s* config)
{
	LoraMode prevMode = this->_currentMode;

	if (setMode(MODE_SLEEP_CONFIG) != HAL_OK) return HAL_ERROR;

	this->dataAvailableFlag = false;
	this->lastPacketSize = 0;
	enableReception();

	uint8_t cmd[3] = { E22_CMD_READ_REG, 0x00, 0x09 }; // 0xC1 0x00 0x09

	if (HAL_UART_Transmit(_huart, cmd, 3, 100) != HAL_OK)
	{
		setMode(prevMode);
		return HAL_ERROR;
	}

	uint32_t tickstart = HAL_GetTick();
	while (!this->dataAvailableFlag)
	{
		if ((HAL_GetTick() - tickstart) > E22_TX_TIMEOUT)
		{
			setMode(prevMode);
			return HAL_TIMEOUT;
		}
	}

	if (this->rxAppBuffer[0] == E22_CMD_READ_REG && this->lastPacketSize >= 12)
	{
		memcpy(config, &this->rxAppBuffer[3], sizeof(E22_Config_s));
		printConfig(*config);
		setMode(prevMode);
		return HAL_OK;
	}

	setMode(prevMode);
	return HAL_ERROR;
}

//////////////////////////////////// Reinicia o Módulo (Reset Software) ////////////////////////////

HAL_StatusTypeDef LORA::resetModule() 
{
    disableReception();

    if (setMode(MODE_SLEEP_CONFIG) != HAL_OK)
    {
        setMode(MODE_DEEP_SLEEP);
        return HAL_ERROR;
    }

    uint8_t cmd[3] = {0xC4, 0xC4, 0xC4};
    HAL_UART_Transmit(_huart, cmd, sizeof(cmd), E22_TX_TIMEOUT);
    
    HAL_Delay(100);

    HAL_StatusTypeDef status = waitForAuxHigh(E22_AUX_TIMEOUT);
    setMode(MODE_NORMAL);

	enableReception();

    return status;
}

//////////////////////////////////// Printa o Log da Configuração ////////////////////////////
void LORA::printConfig(E22_Config_s& config)
{
    printf("--- Lora E22 Config ---\n\r");
    printf("ADDR: %d (0x%02X%02X)\n\r", (config.addh << 8) | config.addl, config.addh, config.addl);
    printf("NETID: 0x%02X\n\r", config.netid);
    printf("CHAN: %d\n\r", config.chan);
    printf("UART Parity: %d\n\r", config.reg0.uartParity);
    printf("UART Baud: %d\n\r", config.reg0.uartBaudRate);
    printf("Air Rate: %d\n\r", config.reg0.airDataRate);
    printf("Fixed Tx: %d\n\r", config.reg3.fixedTransmission);
    printf("LBT Enable: %d\n\r", config.reg3.lbtEnable);
    printf("Power (0=Max): %d\n\r", config.reg1.transmissionPower);
    printf("-----------------------\n\r");
}

//////////////////////////////////// Modifica o Estado dos Pinos M0 e M1 ////////////////////////////
HAL_StatusTypeDef LORA::setMode(LoraMode mode)
{
	if (this->_currentMode == mode) return HAL_OK;

	if (mode == MODE_SLEEP_CONFIG || mode == MODE_DEEP_SLEEP)
	{
		changeHardwareUartConfig(9600, UART_PARITY_NONE);
	}
	else if (mode == MODE_NORMAL || mode == MODE_WAKE_UP)
	{
		changeHardwareUartConfig(this->_targetBaudRate, this->_targetParity);
	}

    switch (mode) {
        case MODE_NORMAL:
            HAL_GPIO_WritePin(_m0.port, _m0.pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(_m1.port, _m1.pin, GPIO_PIN_RESET);
            break;
        case MODE_WAKE_UP: // Transmissor/Receptor WOR
            HAL_GPIO_WritePin(_m0.port, _m0.pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(_m1.port, _m1.pin, GPIO_PIN_RESET);
            break;
        case MODE_SLEEP_CONFIG: // Modo de comandos AT e Registradores
            HAL_GPIO_WritePin(_m0.port, _m0.pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(_m1.port, _m1.pin, GPIO_PIN_SET);
            break;
        case MODE_DEEP_SLEEP:
            HAL_GPIO_WritePin(_m0.port, _m0.pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(_m1.port, _m1.pin, GPIO_PIN_SET);
            break;
    }

    this-> _currentMode = mode;

    return waitForAuxHigh(E22_AUX_TIMEOUT);
}

//////////////////////////////////// Aguarda Pino AUX sinalizar "Livre" ////////////////////////////

HAL_StatusTypeDef LORA::waitForAuxHigh(uint32_t timeout) 
{
	HAL_Delay(3);

    uint32_t start_tick = HAL_GetTick();

    while (getAuxStatus() == false)
    {
        if ((HAL_GetTick() - start_tick) > timeout)
        {
            return HAL_TIMEOUT;
        }
    }

    HAL_Delay(2);

    this->auxFlag = true;
    return HAL_OK;
}

//////////////////////////////////// Interrupções AUX ////////////////////////////

bool LORA::getAuxStatus()
{
	return (HAL_GPIO_ReadPin(_aux.port, _aux.pin) == GPIO_PIN_SET);
}

void LORA::handleAuxInterrupt()
{
    this->auxFlag = getAuxStatus();
}

bool LORA::hasAuxChanged(bool* finalState)
{
    if (this->auxFlag)
    {
        this->auxFlag = false;
        if (finalState != nullptr)
        	*finalState = getAuxStatus();

        return true;
    }
    return false;
}
