/*
 * StorageLog.cpp
 *
 *  Created on: Apr 10, 2026
 *      Author: giova
 */

#include "Storage_Logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


// ==========================================
// Ring Buffer
// ==========================================

// Variáveis e defines

#define RING_BUFFER_SIZE 64 // 64 pacotes de ~60 bytes = ~3.8 KB de RAM SRAM

static Data dataBuffer[RING_BUFFER_SIZE];
static volatile uint16_t head = 0; // Onde escrevemos
static volatile uint16_t tail = 0; // Onde lemos

// ==========================================
// PONTE  PARA O LITTLEFS
// ==========================================

extern "C"
{
	int stmlfs_hal_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size)
	{
		W25Q128_HandleTypeDef* dev = (W25Q128_HandleTypeDef*)c->context;

		while(W25Q128_IsBusy(dev)); // Espera se houver operação pendente
		W25Q128_ReadData(dev, (block * c->block_size) + off, (uint8_t*)buffer, size);

		while(W25Q128_IsBusy(dev)); // Espera DMA terminar
		return LFS_ERR_OK;
	}

	int stmlfs_hal_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size)
	{
		W25Q128_HandleTypeDef* dev = (W25Q128_HandleTypeDef*)c->context;
		while(W25Q128_IsBusy(dev));

		W25Q128_WritePage(dev, (block * c->block_size) + off, (uint8_t*)buffer, size);
		while(W25Q128_IsBusy(dev));
		return LFS_ERR_OK;
	}

	int stmlfs_hal_erase(const struct lfs_config *c, lfs_block_t block)
	{
		W25Q128_HandleTypeDef* dev = (W25Q128_HandleTypeDef*)c->context;
		while(W25Q128_IsBusy(dev));

		W25Q128_EraseSector(dev, block * c->block_size);

		while(W25Q128_IsBusy(dev));
		return LFS_ERR_OK;
	}

	int stmlfs_hal_sync(const struct lfs_config *c)
	{
		return LFS_ERR_OK; // O hardware do W25Q128 sincroniza a cada instrução de Write/Erase.
	}

	uint32_t lfs_crc(uint32_t crc, const void* buffer, size_t size)
	{
	    static const uint32_t rtable[16] = {
	        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4,
	        0x4db26158, 0x5005713c, 0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
	        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c,
	    };

	    const uint8_t* data = (const uint8_t*)buffer;

	    for (size_t i = 0; i < size; i++) {
	        crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 0)) & 0xf];
	        crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 4)) & 0xf];
	    }

	    return crc;
	}

}

// ==========================================
// MÉTODOS DA CLASSE STORAGE_LOGGER
// ==========================================

void Storage_Logger::setupLfsConfig()
{
	memset(&lfsConfig, 0, sizeof(lfsConfig));
    lfsConfig.context = &flashDev; // Passa o driver para o LFS
    lfsConfig.read  = stmlfs_hal_read;
    lfsConfig.prog  = stmlfs_hal_prog;
    lfsConfig.erase = stmlfs_hal_erase;
    lfsConfig.sync  = stmlfs_hal_sync;

    // Configuração geométrica do chip W25Q128
    lfsConfig.read_size      = 256;
    lfsConfig.prog_size      = 256;
    lfsConfig.block_size     = 4096; // Tamanho do Setor (Erase)
    lfsConfig.block_count    = 4096; // W25Q128 = 16 MB -> 4096 blocos de 4KB
    lfsConfig.cache_size     = 256;
    lfsConfig.lookahead_size = 256;
    lfsConfig.block_cycles   = 500;
}

void Storage_Logger::getNextAvailableName(char* outName, const char* baseName, const char* ext, bool isSD) {
    int counter = 0;
    // Tenta primeiro o nome puro: "log_voo.bin"
    sprintf(outName, "%s%s", baseName, ext);

    while (true) {
        bool exists = false;
        if (isSD) {
            FILINFO fno;
            exists = (f_stat(outName, &fno) == FR_OK);
        } else {
            struct lfs_info info;
            exists = (lfs_stat(&lfs, outName, &info) == LFS_ERR_OK);
        }

        if (!exists) break; // Nome está livre!

        // Se já existe, tenta "log_voo(1).bin"
        counter++;
        sprintf(outName, "%s(%d)%s", baseName, counter, ext);

        if(counter > 99) break; // Trava de segurança
    }
}

// ==========================================
// CONSTRUTOR
// ==========================================

Storage_Logger::Storage_Logger(SPI_HandleTypeDef* hspi, PinName cs, PinName wp) : sdManager(SDPath)
{
    this->flashDev.spiHandle = hspi;
    this->flashDev.CS_Pin = cs;
    this->flashDev.WP_Pin = wp;
    this->isFlashOpen = false;
    this->isSDOpen = false;
    this->isFileOpen = false;

    memset(activeFlashName, 0, sizeof(activeFlashName));
	memset(activeSDName, 0, sizeof(activeSDName));
	memset(activeStatusName, 0, sizeof(activeStatusName));
}

// ==========================================
// INICIALIZADOR
// ==========================================

bool Storage_Logger::init(const char* fBase, const char* fExt,
                          const char* sBase, const char* sExt,
                          const char* msgBase, const char* msgExt)
{
	// Inicia HW da Flash
	W25Q128_Init(&flashDev);
	setupLfsConfig();
	HAL_Delay(10);

	// Monta LittleFS
	int err = lfs_mount(&lfs, &lfsConfig);
	if (err) {
		lfs_format(&lfs, &lfsConfig);
		err = lfs_mount(&lfs, &lfsConfig);
	}

	isFlashOpen = (err == 0);
	if (!isFlashOpen)
	{
		LogPrint("Storage: Falha critica na Flash!\n\r");
		return false;
	}

	// Monta SD Card
	isSDOpen = sdManager.mount();
	if (!isSDOpen)
	{
		LogPrint("Storage: SD Card nao encontrado ou falha ao montar.\n\r");
		return false;
	}

	// Resolve os Nomes
	if (isFlashOpen) {
		getNextAvailableName(activeFlashName, fBase, fExt, false);
	}

	if (isSDOpen)
	{
		getNextAvailableName(activeSDName, sBase, sExt, true);
		sprintf(activeStatusName, "%s%s", msgBase, msgExt);

		writeStatusMsg(false, "======= LOG DE SISTEMA =======\n");
		writeStatusMsg(true, "Flash Log: %s\n", activeFlashName);
		writeStatusMsg(true, "SD Data Log: %s\n", activeSDName);
		writeStatusMsg(true, "SD Status Log: %s\n", activeStatusName);

		uint32_t freeSpace = getFlashFreeSpaceKB();
		writeStatusMsg(true, "Memoria Flash Livre: %u KB\n", freeSpace);

		// Alerta se a memória estiver acabando (ex: menos de 1MB)
		if (freeSpace < 1024) {
			writeStatusMsg(true, "AVISO: Memoria Flash quase cheia!\n");
		}

		writeStatusMsg(true, "Status: Inicializado com sucesso!\n");
		writeStatusMsg(true, "------------------------------\n");

		LogPrint("Flash Log: %s\n\r", activeFlashName);
		LogPrint("SD Data Log: %s\n\r", activeSDName);
		LogPrint("SD Status Log: %s\n\r", activeStatusName);
		LogPrint("Memoria Flash Livre: %lu KB\n\r", freeSpace);

	}

	return isFlashOpen; // Retornamos o status da Flash pois o SD é opcional em voo
}

// ==========================================
// FUNÇÕES DE VOO
// ==========================================

bool Storage_Logger::openFlash()
{
	if (!isFlashOpen) return false;

	    // Apenas RDWR e CREAT, sem TRUNC (nunca apaga dados de log anteriores!)
	    int err = lfs_file_open(&lfs, &lfsFile, activeFlashName, LFS_O_RDWR | LFS_O_CREAT);
	    if (err == 0) {
	        isFileOpen = true;
	        return true;
	    }
	    return false;
}

bool Storage_Logger::writeFlashStruct(Data* dataPacket)
{
    if (!isFlashOpen || !isFileOpen) return false;

    uint16_t next_head = (head + 1) % RING_BUFFER_SIZE;
    if (next_head == tail) return false;

    dataBuffer[head] = *dataPacket;
	head = next_head;
	return true;
}

void Storage_Logger::processFlashQueue()
{
    if (!isFlashOpen || !isFileOpen) return;

    // Se o buffer estiver acumulando, gravamos aos poucos
    while (head != tail) {
            lfs_file_write(&lfs, &lfsFile, &dataBuffer[tail], sizeof(Data));
            tail = (tail + 1) % RING_BUFFER_SIZE;
        }
}

bool Storage_Logger::syncFlash()
{
    if (!isFlashOpen || !isFileOpen) return false;
    processFlashQueue(); // Esvazia a RAM antes de sincronizar
    return (lfs_file_sync(&lfs, &lfsFile) == LFS_ERR_OK);
}

bool Storage_Logger::closeFlash()
{
	if (!isFlashOpen || !isFileOpen) return false;
	    processFlashQueue();

	    // Captura o tamanho exato antes de fechar o arquivo
	    lfs_soff_t fileSize = lfs_file_size(&lfs, &lfsFile);
	    int err = lfs_file_close(&lfs, &lfsFile);
	    isFileOpen = false;

	    if (err == LFS_ERR_OK && isSDOpen) {
	        uint32_t numPacotes = (uint32_t)(fileSize / sizeof(Data));
	        float sizeKB = (float)fileSize / 1024.0f;

	        writeStatusMsg(true, "Arquivo %s fechado com sucesso.\n", activeFlashName);
	        writeStatusMsg(true, "Tamanho final: %ld bytes (%.2f KB)\n", (long)fileSize, sizeKB);
	        writeStatusMsg(true, "Total de pacotes: %u\n", numPacotes);
	        writeStatusMsg(true, "Espaco livre restante na Flash: %u KB\n", getFlashFreeSpaceKB());
	        writeStatusMsg(true, "------------------------------\n");
	    }

	    return (err == LFS_ERR_OK);
}

bool Storage_Logger::formatFlash()
{
	if (lfsConfig.read_size == 0) setupLfsConfig();

    int err = lfs_format(&lfs, &lfsConfig);

    if (err == LFS_ERR_OK)
    {
    	debugPrint("Storage: Flash Formatada.\n\r");
    	return true;
    }

    debugPrint("Storage: Erro na formatação.\n\r");
    return false;
}

uint32_t Storage_Logger::getFlashFreeSpaceKB()
{
    if (!isFlashOpen) return 0;

    // lfs_fs_size retorna o número de blocos usados
    lfs_ssize_t used_blocks = lfs_fs_size(&lfs);

    if (used_blocks < 0) return 0;

    uint32_t free_blocks = lfsConfig.block_count - (uint32_t)used_blocks;

    // (blocos * tamanho_do_bloco) / 1024 para KB
    return (free_blocks * lfsConfig.block_size) / 1024;
}

// ==========================================
// FUNÇÕES PÓS-VOO
// ==========================================

bool Storage_Logger::openSD(const char* filename)
{
    if (sdManager.mount()) {
        isSDOpen = true;
        // Salva o nome do arquivo atual para a função writeSDText
        strncpy(activeStatusName, filename, sizeof(activeStatusName) - 1);
        activeStatusName[sizeof(activeStatusName) - 1] = '\0';
        return true;
    }
    isSDOpen = false;
    return false;
}

bool Storage_Logger::writeStatusMsg(bool append, const char* format, ...)
{
	if (!isSDOpen) return false;

	alignas(4) char textBuffer[MAX_LINE_LENGTH];
	va_list args;
	va_start(args, format);
	vsnprintf(textBuffer, sizeof(textBuffer), format, args);
	va_end(args);

	return sdManager.writeLog(activeStatusName, textBuffer, append);
}

bool Storage_Logger::closeSD()
{
	if (sdManager.unmount())
	{
		isSDOpen = false;
		return true;
	}
	return false;
}

// ==========================================
// Dump Flash -> SD Card
// ==========================================
// No RecoveryState, chame essa função para ler o binário da Flash e jogar pro SD.
bool Storage_Logger::dumpFlashToSD()
{
    if (!isFlashOpen) return false;

    // 1. Abre a Flash para leitura
    lfs_file_t fileToRead;
    if (lfs_file_open(&lfs, &fileToRead, activeFlashName, LFS_O_RDONLY) != 0) {
        LogPrint("Storage: Ficheiro binário não encontrado na Flash!\n\r");
        return false;
    }

    // 2. Tenta Montar o SD Card
    if (!openSD(activeSDName) || !sdManager.openFileForAppend(activeSDName))
    {
		lfs_file_close(&lfs, &fileToRead);
		LogPrint("Storage: Falha ao montar o SD Card ou abrir arquivo para o Dump!\n\r");
		return false;
	}

    LogPrint("Iniciando extração Binário -> CSV...\n\r");

    // ==========================================
	// CABEÇALHO (HEADER)
	// ==========================================

    alignas(4) char headerBuffer[MAX_LINE_LENGTH];
    int hLen = 0;

    hLen += snprintf(headerBuffer + hLen, sizeof(headerBuffer) - hLen, "ID,Status,Time(ms)");

#ifdef ENABLE_IMU
  #ifdef ENABLE_IMU_GYRO
    hLen += snprintf(headerBuffer + hLen, sizeof(headerBuffer) - hLen, ",wX,wY,wZ");
  #endif
  #ifdef ENABLE_IMU_LINEAR_ACCEL
    hLen += snprintf(headerBuffer + hLen, sizeof(headerBuffer) - hLen, ",laX,laY,laZ");
  #endif
  #ifdef ENABLE_IMU_ACCEL
    hLen += snprintf(headerBuffer + hLen, sizeof(headerBuffer) - hLen, ",aX,aY,aZ");
  #endif
  #ifdef ENABLE_IMU_MAG
    hLen += snprintf(headerBuffer + hLen, sizeof(headerBuffer) - hLen, ",mX,mY,mZ");
  #endif
  #ifdef ENABLE_IMU_GRAVITY
    hLen += snprintf(headerBuffer + hLen, sizeof(headerBuffer) - hLen, ",gX,gY,gZ");
  #endif
  #ifdef ENABLE_IMU_QUATERNION
    hLen += snprintf(headerBuffer + hLen, sizeof(headerBuffer) - hLen, ",Qi,Qj,Qk,Qr");
  #endif
#endif

#ifdef ENABLE_BAROMETER
    hLen += snprintf(headerBuffer + hLen, sizeof(headerBuffer) - hLen, ",Press,Alt");
#endif

#ifdef ENABLE_ADC_TEMP
    hLen += snprintf(headerBuffer + hLen, sizeof(headerBuffer) - hLen, ",Temp");
#endif

#ifdef ENABLE_ADC_BATTERY
    hLen += snprintf(headerBuffer + hLen, sizeof(headerBuffer) - hLen, ",Bat");
#endif

    snprintf(headerBuffer + hLen, sizeof(headerBuffer) - hLen, "\n");

    // Grava o cabeçalho e verifica sucesso
    if (!sdManager.writeToFile(headerBuffer)) {
        LogPrint("Storage: Erro ao gravar cabecalho no SD!\n\r");
        sdManager.closeFile();
        lfs_file_close(&lfs, &fileToRead);
        return false;
    }

    Data readData;
    int pacotesLidos = 0;

    // 3. Lê pacotes da Flash até ao fim do arquivo
    while (lfs_file_read(&lfs, &fileToRead, &readData, sizeof(Data)) == sizeof(Data))
    {
        // Buffer único e alinhado para formar a linha toda antes de chamar o DMA
        alignas(4) char lineBuffer[MAX_LINE_LENGTH];
        int len = 0;

        // Escreve IDs básicos
        len += snprintf(lineBuffer + len, sizeof(lineBuffer) - len, "%u,%d,%lu", readData.dataID, readData.status, readData.timestamp);

#ifdef ENABLE_IMU
  #ifdef ENABLE_IMU_GYRO
        len += snprintf(lineBuffer + len, sizeof(lineBuffer) - len, ",%.2f,%.2f,%.2f", readData.w[0], readData.w[1], readData.w[2]);
  #endif
  #ifdef ENABLE_IMU_LINEAR_ACCEL
        len += snprintf(lineBuffer + len, sizeof(lineBuffer) - len, ",%.2f,%.2f,%.2f", readData.la[0], readData.la[1], readData.la[2]);
  #endif
  #ifdef ENABLE_IMU_ACCEL
        len += snprintf(lineBuffer + len, sizeof(lineBuffer) - len, ",%.2f,%.2f,%.2f", readData.a[0], readData.a[1], readData.a[2]);
  #endif
  #ifdef ENABLE_IMU_MAG
        len += snprintf(lineBuffer + len, sizeof(lineBuffer) - len, ",%.2f,%.2f,%.2f", readData.m[0], readData.m[1], readData.m[2]);
  #endif
  #ifdef ENABLE_IMU_GRAVITY
        len += snprintf(lineBuffer + len, sizeof(lineBuffer) - len, ",%.2f,%.2f,%.2f", readData.g[0], readData.g[1], readData.g[2]);
  #endif
  #ifdef ENABLE_IMU_QUATERNION
        len += snprintf(lineBuffer + len, sizeof(lineBuffer) - len, ",%.2f,%.2f,%.2f,%.2f", readData.Q[0], readData.Q[1], readData.Q[2], readData.Q[3]);
  #endif
#endif

#ifdef ENABLE_BAROMETER
        len += snprintf(lineBuffer + len, sizeof(lineBuffer) - len, ",%.2f,%.2f", readData.pressure, readData.altitude);
#endif

#ifdef ENABLE_ADC_TEMP
        len += snprintf(lineBuffer + len, sizeof(lineBuffer) - len, ",%.2f", readData.temperature);
#endif

#ifdef ENABLE_ADC_BATTERY
        len += snprintf(lineBuffer + len, sizeof(lineBuffer) - len, ",%.2f", readData.battery);
#endif

        // Adiciona a quebra de linha no final
        snprintf(lineBuffer + len, sizeof(lineBuffer) - len, "\n");

        // Envia a linha formatada
        if (!sdManager.writeToFile(lineBuffer))
		{
        	LogPrint("Storage: Erro Crítico dutante o Dump! interrompido no pacote: %d. \n\r.", pacotesLidos);
        	break;
		}

        pacotesLidos++;
    }

    sdManager.closeFile();
	lfs_file_close(&lfs, &fileToRead);
	closeSD(); // Desmonta o SD

	LogPrint("Dump CSV concluído! Total de pacotes extraídos: %d\n\r", pacotesLidos);
	return true;
}
