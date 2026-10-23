/*
 * StorageLog.h
 *
 *  Created on: Apr 10, 2026
 *      Author: giova
 */

#ifndef STORAGE_LOGGER_H
#define STORAGE_LOGGER_H

#include "main.h"
#include "GlobalDefines.h"
#include "FC_Config.h" // Para saber a estrutura do pacote 'Data'

// Includes do SD Card (FatFS)
#include "SDCard.hh"

// Includes da Flash (LittleFS)
#include "lfs.h"
#include "W25Q128.h"

class Storage_Logger
{
public:
    // ==========================================
    // ESTRUTURAS DE HARDWARE E FILE SYSTEM
    // ==========================================

	W25Q128_HandleTypeDef flashDev;

	// LittleFS (Flash)
	lfs_t lfs;
	lfs_file_t lfsFile;
	struct lfs_config lfsConfig;

	// FatFS (SD Card)
	SDCard sdManager;

	// Estado da classe
	bool isFlashOpen;
	bool isSDOpen;
	bool isFileOpen;

	char activeFlashName[32];
	char activeSDName[32];
	char activeStatusName[32];

	static const uint16_t MAX_LINE_LENGTH = 256;

	Storage_Logger(SPI_HandleTypeDef* hspi, PinName cs, PinName wp);

	// ==========================================
	// METODOS GERAIS
	// ==========================================

	bool init(const char* fBase, const char* fExt,
		      const char* sBase, const char* sExt,
			  const char* msgBase, const char* msgExt);

	// ==========================================
	// METODOS PARA A FLASH
	// ==========================================

	bool openFlash();

	// Salva a struct bruta de dados em formato binário no Ring Buffer
	bool writeFlashStruct(Data* dataPacket);

	void processFlashQueue();

	bool syncFlash();
	bool closeFlash();
	bool formatFlash();

	uint32_t getFlashFreeSpaceKB();

	// ==========================================
	// METODOS PARA O SD CARD
	// ==========================================

	bool writeStatusMsg(bool append, const char* format, ...);
	bool openSD(const char* filename);
	bool closeSD();

	// ==========================================
	// METODOS DE TRANSFERÊNCIA (Recovery State)
	// ==========================================

	bool dumpFlashToSD();

private:
    // Configura os ponteiros da struct lfs_config para o driver W25Q128
	void setupLfsConfig();

	void getNextAvailableName(char* outName, const char* baseName, const char* ext, bool isSD);
};

#endif // STORAGE_LOGGER_H
