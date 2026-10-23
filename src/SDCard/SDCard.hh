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
#include <string.h>
#include <stdio.h>
#include <stdarg.h>


class SDCard {
public:

	SDCard(const char* rootPath = SDPath);



	//inicializa o sistema de arquivos, checando se o mesmo está conectadp messe exato momento
    bool mount();
    //Fecha o acesso ao cartão SD
    bool unmount();
    //Calcula o espaço livre dentro do cartão
    uint32_t getFreeSpaceKB();
    //Escreve uma String de texto diretamente no arquivo
    bool writeLog(const char* filename, const char* data);
    //a mesma coisa que a anterior, so que funciona parecido com um printf
    bool printLog(const char* filename, const char* format, ...);
    // Cria uma pasta no cartão SD
    bool createPath(const char* dirPath);

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
