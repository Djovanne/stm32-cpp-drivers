/*
 * LORA.hh
 *
 * Atualizado para o Módulo LORA - E22 (Série T) no STM32CubeIDE
 * * Descrição:
 * Classe de gerenciamento do módulo LoRa E22 via UART com DMA.
 */

#ifndef LORA_E22_LORA_H_
#define LORA_E22_LORA_H_

// --- Includes --- //
#include "main.h"
#include "PinName.h"

// --- Comandos do E22 ---
#define E22_CMD_WRITE_REG       0xC0
#define E22_CMD_READ_REG        0xC1
#define E22_CMD_WRITE_TEMP_REG  0xC2

// --- Configurações de Buffer e Timeout ---
#define E22_MAX_TX_BUFFER    255
#define E22_MAX_RX_BUFFER    512
#define E22_TX_TIMEOUT       1000
#define E22_AUX_TIMEOUT      1000
#define E22_MAX_PAYLOAD_SIZE 240

// --- Modos de Operação (Pinos M0 e M1) ---
typedef enum {
    MODE_NORMAL       = 0, // M1=0, M0=0 (Transmissão Transparente/Fixa)
    MODE_WAKE_UP      = 1, // M1=0, M0=1 (Modo WOR)
    MODE_SLEEP_CONFIG = 2, // M1=1, M0=0 (Modo de Configuração de Registradores)
    MODE_DEEP_SLEEP   = 3  // M1=1, M0=1 (Deep Sleep)
} LoraMode;

// --- Enums para os Registradores do E22 ---

typedef enum {
    E22_BAUD_1200   = 0b000,
    E22_BAUD_2400   = 0b001,
    E22_BAUD_4800   = 0b010,
    E22_BAUD_9600   = 0b011, // Padrão
    E22_BAUD_19200  = 0b100,
    E22_BAUD_38400  = 0b101,
    E22_BAUD_57600  = 0b110,
    E22_BAUD_115200 = 0b111
} E22_BaudRate;

typedef enum {
    E22_PARITY_8N1 = 0b00, // Padrão (Sem paridade)
    E22_PARITY_8O1 = 0b01, // Ímpar (Odd)
    E22_PARITY_8E1 = 0b10  // Par (Even)
} E22_Parity;

typedef enum {
    E22_AIR_2_4K  = 0b010, // Padrão
    E22_AIR_4_8K  = 0b011,
    E22_AIR_9_6K  = 0b100,
    E22_AIR_19_2K = 0b101,
    E22_AIR_38_4K = 0b110,
    E22_AIR_62_5K = 0b111
} E22_AirRate;

typedef enum {
    E22_PACKET_240 = 0b00, // Padrão
    E22_PACKET_128 = 0b01,
    E22_PACKET_64  = 0b10,
    E22_PACKET_32  = 0b11
} E22_PacketSize;

typedef enum {
    E22_POWER_MAX   = 0b00, // Padrão (30dBm)
    E22_POWER_HIGH  = 0b01, // (27dBm)
    E22_POWER_MED   = 0b10, // (24dBm)
    E22_POWER_LOW   = 0b11  // (21dBm)
} E22_Power;

typedef enum {
    E22_MODE_TRANSPARENT = 0,
    E22_MODE_FIXED       = 1
} E22_FixedMode;

// --- Estruturas de Memória (Bitfields) ---
// Nota: Em compiladores GCC para ARM (STM32), os bitfields são alocados do LSB (bit 0) para o MSB (bit 7).

// Endereço 0x03
typedef struct {
    uint8_t airDataRate  : 3; // Bits 0-2
    uint8_t uartParity   : 2; // Bits 3-4
    uint8_t uartBaudRate : 3; // Bits 5-7
} REG0_s;

// Endereço 0x04
typedef struct {
    uint8_t transmissionPower : 2; // Bits 0-1
    uint8_t reserved          : 3; // Bits 2-4 (Uso interno/Reservado)
    uint8_t rssiNoiseEnable   : 1; // Bit 5 (Habilita leitura de ruído ambiente)
    uint8_t packetSize        : 2; // Bits 6-7
} REG1_s;

// Endereço 0x06
typedef struct {
    uint8_t worCycle          : 3; // Bits 0-2 (Tempo de ciclo WOR)
    uint8_t worRole           : 1; // Bit 3 (0 = Receptor, 1 = Transmissor)
    uint8_t lbtEnable         : 1; // Bit 4 (Habilita Listen Before Talk)
    uint8_t relayEnable       : 1; // Bit 5 (Habilita função de Repetidor)
    uint8_t fixedTransmission : 1; // Bit 6 (0 = Transparente, 1 = Fixo/Endereçado)
    uint8_t rssiByteEnable    : 1; // Bit 7 (Anexa o byte de RSSI no final do dado recebido)
} REG3_s;

// Estrutura principal mapeando os 9 bytes de configuração do E22 (0x00 ao 0x08)
typedef struct __attribute__((packed)){
    uint8_t addh;     // 0x00 - Endereço Alto
    uint8_t addl;     // 0x01 - Endereço Baixo
    uint8_t netid;    // 0x02 - ID da Rede
    REG0_s  reg0;     // 0x03 - BaudRate, Paridade, AirRate
    REG1_s  reg1;     // 0x04 - Tamanho do pacote, RSSI Ruído, Potência
    uint8_t chan;     // 0x05 - Canal de Comunicação
    REG3_s  reg3;     // 0x06 - Opções avançadas (WOR, LBT, Relay, etc)
    uint8_t crypt_h;  // 0x07 - Chave de Criptografia Alta
    uint8_t crypt_l;  // 0x08 - Chave de Criptografia Baixa
} E22_Config_s;

// --- Classe LORA --- //

class LORA
{
public:
    // Construtor e Destrutor
    LORA(UART_HandleTypeDef* huart, PinName aux, PinName m0, PinName m1);
    ~LORA();

    // Inicialização
    HAL_StatusTypeDef init(uint16_t address, uint8_t netID, uint8_t channel, E22_AirRate airRate, E22_Power power, bool enableRssi);

    // Transmissão
    HAL_StatusTypeDef sendMessage(uint16_t address, uint8_t channel, uint8_t* data, uint8_t length);
    HAL_StatusTypeDef sendBroadcastMessage(uint8_t channel, uint8_t* data, uint8_t length);
    bool isTxReady();
    void handleTxInterrupt();

    // Recepção
    HAL_StatusTypeDef enableReception();
    HAL_StatusTypeDef disableReception();
    uint16_t readData(uint8_t* userBuffer, uint16_t maxSize);
    bool isDataAvailable();
    void handleRXInterrupt(uint16_t Size);

    // Gerenciamento de Configurações do E22
    HAL_StatusTypeDef setConfig(E22_Config_s config, bool temporary = true);
    HAL_StatusTypeDef setUartConfig(E22_BaudRate baudEnum, E22_Parity parity);

    HAL_StatusTypeDef readConfig(E22_Config_s* config);
    HAL_StatusTypeDef resetModule();

    // RSSI
    bool isRssiEnabled() const { return this->_rssiEnabled; }
    float getLastPacketRssi() const { return this->_lastPacketRssi; }
    uint8_t getLastPacketRssiRaw() const { return this->_lastRssiRaw; }

    bool getAuxStatus();
    void handleAuxInterrupt();
    bool hasAuxChanged(bool* finalState);

private:
    UART_HandleTypeDef* _huart;
    PinName             _aux;
    PinName             _m0;
    PinName             _m1;

    uint8_t rxTempBuffer[E22_MAX_RX_BUFFER]; //Buffer temporário de recepcao
    uint8_t rxAppBuffer[E22_MAX_RX_BUFFER];  // Buffer de consumo do dado
    uint8_t txBuffer[E22_MAX_TX_BUFFER];

    volatile bool txBusy;
    volatile bool dataAvailableFlag;
    volatile bool auxFlag;
    volatile uint16_t lastPacketSize;

    uint32_t _targetBaudRate; // Velocidade configurada UART
    uint32_t _targetParity; // Paridade UART

    LoraMode _currentMode; // Guarda modo atual

    bool _rssiEnabled;
	uint8_t _lastRssiRaw;
	float _lastPacketRssi;

    // Funções auxiliares internas
    HAL_StatusTypeDef waitForAuxHigh(uint32_t timeout);
    HAL_StatusTypeDef setMode(LoraMode mode);
    void printConfig(E22_Config_s& config);
    void changeHardwareUartConfig(uint32_t baud, uint32_t parity);
};

#endif /* LORA_E22_LORA_H_ */


