/*
 * Storage_Logger.cpp
 *
 *  Created on: Apr 10, 2026
 *      Author: giova
 */

#include "Storage_Logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// ==========================================
// CONSTRUTOR
// ==========================================

Storage_Logger::Storage_Logger(SPI_HandleTypeDef* hspi, PinName cs, PinName wp)
    : sdManager(SDPath), flash(hspi, cs, wp)
{
    this->isSDOpen = false;

    memset(activeFlashName, 0, sizeof(activeFlashName));
    memset(activeStatusName, 0, sizeof(activeStatusName));

    for (int i = 0; i < MAX_DUMP_REGISTER; i++) dumpRegistries[i].active = false;
}

void Storage_Logger::getNextAvailableSDName(char* outName, const char* baseName, const char* ext)
{
    int counter = 0;
    sprintf(outName, "%s%s", baseName, ext);

    while (true) {
        FILINFO fno;
        if (f_stat(outName, &fno) != FR_OK) break;

        counter++;
        sprintf(outName, "%s(%d)%s", baseName, counter, ext);
        if (counter > 99) break;
    }
}

// ==========================================
// INICIALIZADOR
// ==========================================

bool Storage_Logger::init(bool format, const char* logName, const char* msgName)
{
    if (!flash.mount(format))
    {
        LogPrint("Storage: Falha critica na Flash!\n\r");
        return false;
    }

    flash.getNextAvailableName(activeFlashName, sizeof(activeFlashName), logName, 50);
    flash.getNextAvailableName(activeStatusName, sizeof(activeStatusName), msgName, 50);

    isSDOpen = sdManager.mount();
    if (isSDOpen)
    {
        writeStatusMsg(false, "======= LOG DE SISTEMA =======\n");
        writeStatusMsg(true, "Flash Log: %s\n", activeFlashName);
        writeStatusMsg(true, "SD Status Log: %s\n", activeStatusName);

        uint32_t freeSpace = getFlashFreeSpaceKB();
        writeStatusMsg(true, "Memoria Flash Livre: %u KB\n", freeSpace);

        if (freeSpace < 1024) {
            writeStatusMsg(true, "AVISO: Memoria Flash quase cheia!\n");
        }

        writeStatusMsg(true, "Status: Inicializado com sucesso!\n");
        writeStatusMsg(true, "------------------------------\n");

        LogPrint("Flash Log: %s\n\r", activeFlashName);
        LogPrint("SD Status Log: %s\n\r", activeStatusName);
        LogPrint("Memoria Flash Livre: %lu KB\n\r", freeSpace);
    }
    else
    {
        LogPrint("Storage: SD Card ausente na inicializacao (Modo Voo).\n\r");
    }

    return flash.isReady();
}

// ==========================================
// FUNÇÕES DE VOO (ASSÍNCRONAS / NÃO-BLOQUEANTES)
// ==========================================

bool Storage_Logger::openFlash()
{
    if (!flash.isReady()) return false;
    return flash.openFile(logFile, activeFlashName, LFS_O_RDWR | LFS_O_CREAT);
}

bool Storage_Logger::enqueuePacket(const FlashPacket& pkt)
{
    size_t actualSize = 2 + pkt.size;
    return flash.enqueueData(logFile, &pkt, actualSize);
}

void Storage_Logger::processFlashQueue()
{
    // Executa a máquina de estados consumindo a FIFO interna sem prender a CPU
    flash.update();
}

bool Storage_Logger::syncFlash()
{
    if (!flash.isReady() || !logFile.isOpen) return false;
    return flash.syncFile(logFile);
}

bool Storage_Logger::closeFlash()
{
    if (!flash.isReady() || !logFile.isOpen) return false;

    uint32_t fileSize = flash.getFileSize(logFile);
    bool success = flash.closeFile(logFile);

    if (success && isSDOpen) {
        float sizeKB = (float)fileSize / 1024.0f;
        writeStatusMsg(true, "Arquivo %s fechado com sucesso.\n", activeFlashName);
        writeStatusMsg(true, "Tamanho final: %lu bytes (%.2f KB)\n", fileSize, sizeKB);
        writeStatusMsg(true, "------------------------------\n");
    }

    return success;
}

bool Storage_Logger::formatFlash()
{
    if (flash.format())
    {
        debugPrint("Storage: Flash Formatada.\n\r");
        return true;
    }
    debugPrint("Storage: Erro na formatação.\n\r");
    return false;
}

uint32_t Storage_Logger::getFlashFreeSpaceKB()
{
    return flash.getFreeSpaceKB();
}

// ==========================================
// FUNÇÕES PÓS-VOO
// ==========================================

bool Storage_Logger::openSD(const char* filename)
{
    if (isSDOpen) return true;

    if (sdManager.mount())
    {
        isSDOpen = true;
        strncpy(activeStatusName, filename, sizeof(activeStatusName) - 1);
        activeStatusName[sizeof(activeStatusName) - 1] = '\0';
        return true;
    }
    isSDOpen = false;
    return false;
}

bool Storage_Logger::writeStatusMsg(bool append, const char* format, ...)
{
    if (!flash.isReady()) return false;

    alignas(4) char textBuffer[MAX_LINE_LENGTH];
    va_list args;
    va_start(args, format);
    vsnprintf(textBuffer, sizeof(textBuffer), format, args);
    va_end(args);

    FlashFile statusFile;

    if (flash.openFileForWrite(statusFile, activeStatusName, append))
    {
        flash.writeData(statusFile, textBuffer, strlen(textBuffer));
        flash.closeFile(statusFile);
        return true;
    }

    return false;
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

const DumpRegistry* Storage_Logger::getDumpRegistry(uint8_t id)
{
    for (int i = 0; i < MAX_DUMP_REGISTER; i++) {
        if (dumpRegistries[i].active && dumpRegistries[i].id == id) {
            return &dumpRegistries[i];
        }
    }
    return nullptr;
}

bool Storage_Logger::registerDumpType(uint8_t id, const char* sdFileName, const char* csvHeader, FormatToCSV_Func formatter)
{
    for (int i = 0; i < MAX_DUMP_REGISTER; i++) {
        if (!dumpRegistries[i].active) {
            dumpRegistries[i].id = id;
            strncpy(dumpRegistries[i].fileName, sdFileName, 31);
            dumpRegistries[i].csvHeader = csvHeader;
            dumpRegistries[i].formatter = formatter;
            dumpRegistries[i].active = true;
            return true;
        }
    }
    return false;
}

bool Storage_Logger::dumpFlashToSD()
{
    if (!flash.isReady())
    {
        LogPrint("Storage: Flash nao esta pronta para dump.\n\r");
        return false;
    }

    if (!isSDOpen)
    {
        LogPrint("Storage: SD Card nao esta pronto para o Dump.\n\r");
        return false;
    }

    LogPrint("Iniciando extracao Binario -> Multiplos CSVs...\n\r");

    writeStatusMsg(true, "\n======= RELATORIO DE DUMP =======\n");
    writeStatusMsg(true, "Arquivo origem (Flash): %s\n", activeFlashName);
    writeStatusMsg(true, "---------------------------------\n");

    int pacotesTotaisGerais = 0;

    for (int i = 0; i < MAX_DUMP_REGISTER; i++)
    {
        if (!dumpRegistries[i].active) continue;

        DumpRegistry& reg = dumpRegistries[i];
        LogPrint(">> Extraindo pacote ID %d para arquivo %s...\n\r", reg.id, reg.fileName);

        char finalSDName[32];
        getNextAvailableSDName(finalSDName, reg.fileName, ".csv");

        if (!sdManager.openFileForAppend(finalSDName)) {
            LogPrint("Erro ao criar arquivo %s no SD.\n\r", finalSDName);
            writeStatusMsg(true, "[ERRO] ID %d: Falha ao criar %s no SD\n", reg.id, finalSDName);
            continue;
        }

        char headerLine[MAX_LINE_LENGTH];
        snprintf(headerLine, sizeof(headerLine), "%s\n", reg.csvHeader);
        sdManager.writeToFile(headerLine);

        if (!flash.openFile(logFile, activeFlashName, LFS_O_RDONLY))
        {
            sdManager.closeFile();
            LogPrint("Erro critico: Binario base nao encontrado na Flash.\n\r");
            writeStatusMsg(true, "[ERRO CRITICO] Arquivo %s nao encontrado na Flash!\n", activeFlashName);
            return false;
        }

        int pacotesLidos = 0;
        uint8_t meta[2];

        while (flash.readData(logFile, meta, 2) == 2)
        {
            uint8_t packetId = meta[0];
            uint8_t packetSize = meta[1];
            uint8_t payloadData[MAX_PAYLOAD_SIZE];

            if (packetSize > MAX_PAYLOAD_SIZE) {
                LogPrint("Erro: Tamanho de pacote corrompido lido na Flash. Abortando varredura.\n\r");
                writeStatusMsg(true, "  -> [AVISO] Corrupcao na Flash. Leitura do ID %d abortada.\n", reg.id);
                break;
            }

            if (flash.readData(logFile, payloadData, packetSize) != packetSize) {
                break;
            }

            if (packetId == reg.id)
            {
                alignas(4) char lineBuffer[MAX_LINE_LENGTH];
                memset(lineBuffer, 0, sizeof(lineBuffer));

                reg.formatter(payloadData, lineBuffer, sizeof(lineBuffer));

                size_t len = strlen(lineBuffer);
                if (len > 0 && lineBuffer[len - 1] != '\n') {
                    strncat(lineBuffer, "\n", sizeof(lineBuffer) - len - 1);
                }

                if (!sdManager.writeToFile(lineBuffer)) {
                    LogPrint("Storage: Erro de escrita no SD durante o Dump.\n\r");
                    writeStatusMsg(true, "  -> [ERRO] Escrita falhou (SD ejetado ou cheio?) durante ID %d.\n", reg.id);
                    break;
                }
                pacotesLidos++;
            }
        }

        sdManager.closeFile();
        flash.closeFile(logFile);

        LogPrint("<< Dump do ID %d finalizado. Pacotes gravados: %d\n\r", reg.id, pacotesLidos);
        writeStatusMsg(true, "[OK] ID %d -> %s gerado (%d linhas)\n", reg.id, finalSDName, pacotesLidos);
        pacotesTotaisGerais += pacotesLidos;
    }

    writeStatusMsg(true, "---------------------------------\n");
    writeStatusMsg(true, "Total de pacotes CSV gerados: %d\n", pacotesTotaisGerais);
    writeStatusMsg(true, "Dump finalizado com sucesso!\n");
    writeStatusMsg(true, "=================================\n");

    closeSD();
    LogPrint("Extracao concluida com sucesso!\n\r");
    return true;
}

// ========================== Transfere todos os dados para o SD =================================== //

bool Storage_Logger::dumpAllFiles()
{
    if (!flash.isReady())
    {
        LogPrint("Storage: Flash nao montada.\n\r");
        return false;
    }
    if (!isSDOpen && !openSD("backup_status.txt")) {
        LogPrint("Storage: SD Card nao esta presente para backup.\n\r");
        return false;
    }

    LogPrint("Iniciando Backup Raw: Flash -> SD Card...\n\r");
    writeStatusMsg(true, "\n======= BACKUP FLASH -> SD =======\n");

    if (!flash.openDir("/"))
    {
        LogPrint("Erro ao abrir diretorio raiz da Flash.\n\r");
        return false;
    }

    int arquivosCopiados = 0;
    char fileName[32];
    uint32_t fileSize;

    while (flash.readNextFile(fileName, &fileSize))
    {
        LogPrint("Copiando arquivo: %s (%lu bytes)...\n\r", fileName, fileSize);
        writeStatusMsg(true, "Copiando: %s (%lu bytes)... ", fileName, fileSize);

        if (flash.openFileForRead(logFile, fileName))
        {
            if (sdManager.openFileForWriteRaw(fileName))
            {
                uint8_t buffer[512];
                lfs_ssize_t bytesRead;
                UINT bytesWritten;
                bool falha = false;

                while ((bytesRead = flash.readData(logFile, buffer, sizeof(buffer))) > 0)
                {
                    if (!sdManager.writeBytes(buffer, (UINT)bytesRead, &bytesWritten) || bytesWritten != (UINT)bytesRead)
                    {
                        falha = true;
                        break;
                    }
                }

                sdManager.closeFile();

                if (falha) {
                    LogPrint(" Falha na gravacao no SD!\n\r");
                    writeStatusMsg(true, "[ERRO NO SD]\n");
                } else {
                    LogPrint(" Sucesso!\n\r");
                    writeStatusMsg(true, "[OK]\n");
                    arquivosCopiados++;
                }
            }
            else
            {
                LogPrint(" Nao foi possivel criar arquivo no SD.\n\r");
                writeStatusMsg(true, "[FALHA CRIACAO]\n");
            }

            flash.closeFile(logFile);
        }
    }

    flash.closeDir();

    writeStatusMsg(true, "----------------------------------\n");
    writeStatusMsg(true, "Total de arquivos copiados: %d\n", arquivosCopiados);
    writeStatusMsg(true, "==================================\n");
    LogPrint("Backup finalizado! %d arquivos copiados.\n\r", arquivosCopiados);

    return true;
}
