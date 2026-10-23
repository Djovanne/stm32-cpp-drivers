/*
 * Storage_Logger.h
 *
 *  Created on: Apr 10, 2026
 *      Author: giova
 */

#ifndef STORAGE_LOGGER_H
#define STORAGE_LOGGER_H

#include "main.h"
#include "GlobalDefines.h"

// Includes do SD Card (FatFS)
#include "SDCard.hh"

// Includes da Flash (LittleFS)
#include "FlashStorage.h"

#define MAX_PAYLOAD_SIZE  128
#define MAX_LINE_LENGTH   256
#define MAX_DUMP_REGISTER 5

struct __attribute__((packed)) FlashPacket
{
    uint8_t id;
    uint8_t size;
    uint8_t payload[MAX_PAYLOAD_SIZE];
};

typedef void (*FormatToCSV_Func)(const uint8_t* rawPayload, char* outBuffer, size_t maxLen);

struct DumpRegistry
{
    uint8_t id;
    bool active;
    char fileName[32];
    const char* csvHeader;
    FormatToCSV_Func formatter;
};

class Storage_Logger
{
public:
    SDCard sdManager;
    bool isSDOpen;

    FlashStorage flash;

    Storage_Logger(SPI_HandleTypeDef* hspi, PinName cs, PinName wp);

    // ==========================================
    // METODOS GERAIS
    // ==========================================
    bool init(bool format, const char* logName, const char* msgName);

    // ==========================================
    // METODOS PARA A FLASH
    // ==========================================
    bool openFlash();
    void processFlashQueue();
    bool syncFlash();
    bool closeFlash();
    bool formatFlash();
    uint32_t getFlashFreeSpaceKB();

    lfs_t* getLfsInstance() { return flash.getLfsInstance(); }
    const DumpRegistry* getDumpRegistry(uint8_t id);
    bool isFlashReady() { return flash.isReady(); }

    // ==========================================
    // METODOS PARA O SD CARD
    // ==========================================
    void getNextAvailableSDName(char* outName, const char* baseName, const char* ext);
    bool writeStatusMsg(bool append, const char* format, ...);
    bool openSD(const char* filename);
    bool closeSD();

    // ==========================================
    // METODOS DE TRANSFERÊNCIA (Recovery State)
    // ==========================================
    template <typename T>
    bool writeData(uint8_t id, const T* data)
    {
        if (!flash.isReady() || !logFile.isOpen) return false;

        static_assert(sizeof(T) <= MAX_PAYLOAD_SIZE, "Struct excede o MAX_PAYLOAD_SIZE configurado para a Flash!");

        FlashPacket pkt;
        pkt.id = id;
        pkt.size = sizeof(T);
        memcpy(pkt.payload, data, sizeof(T));

        return enqueuePacket(pkt);
    }

    bool registerDumpType(uint8_t id, const char* sdFileName, const char* csvHeader, FormatToCSV_Func formatter);
    bool dumpFlashToSD();
    bool dumpAllFiles();

private:
    FlashFile logFile;

    char activeFlashName[32];
    char activeStatusName[32];

    DumpRegistry dumpRegistries[MAX_DUMP_REGISTER];
    bool enqueuePacket(const FlashPacket& pkt);
};

#endif // STORAGE_LOGGER_H
