/*
 * SDCard.hh
 *
 *  Created on: Jan 14, 2026
 *      Author: giova
 *
 * This class provides basic SD card management functionalities using the FatFs library.
 * To be effective, ensure that the FatFs library is properly integrated into your project:
 *
 * 		- active interrupt in NVIC, DMA RX and TX for SDIO, and active DMA template in FATFS Middleware.
 *
 * 		- In "Project Manager", in Code Generator, up the Minimum Heap and Stack Size in 10x
 *
 * 		- To change the TimeOut Time in FAT System error, go to "sd_diskio.c", and change the define "SD_TIMEOUT"
 */

#ifndef SDMANAGER_HPP
#define SDMANAGER_HPP

#include "fatfs.h"
#include "GlobalDefines.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>


class SDCard
{
public:

	SDCard(const char* rootPath = SDPath);

    bool mount();
    bool unmount();
    uint32_t getFreeSpaceKB();
    bool createPath(const char* dirPath);

    bool openFileForAppend(const char* filename);
    bool writeToFile(const char* data);
    bool printToFile(const char* format, ...);
    bool closeFile();

    bool writeLog(const char* filename, const char* data, bool apped = true);
	bool printLog(const char* filename, const char* format, ...);

	FRESULT getLastResult() const { return res; }

private:
    FATFS fs;      // Objeto do Sistema de Arquivos
    FIL fil;
    FRESULT res;
    char path[10];

    // Buffer interno para formatação de strings (ajuste o tamanho conforme necessidade)
    char buffer[256];
};

#endif // SDMANAGER_HPP
