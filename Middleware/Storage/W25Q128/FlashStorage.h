/*
 * FlashStorage.h
 *
 *  Created on: Aug 8, 2026
 *      Author: giova
 */

#ifndef MEMORY_W25QXX_FLASHSTORAGE_H_
#define MEMORY_W25QXX_FLASHSTORAGE_H_

#include "W25Qxx.h"
#include "main.h"
#include "lfs.h"
#include "pinName.h"
#include "FifoBuffer.hh"

#define MAX_OPEN_FILES      4
#define FLASH_FIFO_CAPACITY 32
#define FLASH_CHUNK_SIZE    130

struct FlashFile
{
    lfs_file_t file;
    bool isOpen = false;
};

struct FlashQueueItem
{
    FlashFile* fileHandle;
    uint8_t data[FLASH_CHUNK_SIZE];
    size_t size;
};

class FlashStorage
{
public:
    FlashStorage(SPI_HandleTypeDef* hspi, PinName cs, PinName wp = {nullptr, 0});

    bool mount(bool formatOnFail = true);
    bool unmount();
    bool format();
    uint32_t getFreeSpaceKB();
    bool isReady() const;

    lfs_t* getLfsInstance();
    W25Qxx_HandleTypeDef* getFlashInstance();

    bool openFile(FlashFile& fileHandle, const char* filename, int flags);
    bool removeFile(const char* filename);

    bool openFileForRead(FlashFile& fileHandle, const char* filename);
    bool openFileForWrite(FlashFile& fileHandle, const char* filename, bool append = false);

    void closeAllFiles();
    bool closeFile(FlashFile& fileHandle);
    bool syncFile(FlashFile& fileHandle);
    uint32_t getFileSize(FlashFile& fileHandle);

    char* readLine(FlashFile& fileHandle, char* buffer, int maxLen);

    bool openDir(const char* path = "/");
    bool readNextFile(char* outName, uint32_t* outSize);
    bool closeDir();

    lfs_ssize_t writeData(FlashFile& fileHandle, const void* buffer, lfs_size_t size);
    lfs_ssize_t readData(FlashFile& fileHandle, void* buffer, lfs_size_t size);

    bool getNextAvailableName(char* outName, size_t outNameSize, const char* fullFileName, int maxAttempts);

    // ==========================================
    // MÁQUINA DE ESTADOS / FIFO
    // ==========================================
    bool enqueueData(FlashFile& fileHandle, const void* buffer, size_t size);
    void update();
    void flushQueue();

private:
    W25Qxx_HandleTypeDef flashDev;
    lfs_t lfs;

    lfs_dir_t lfsDir;
    bool isDirOpen;
    struct lfs_config lfsConfig;

    bool isMounted;

    FlashFile* openFilesTrack[MAX_OPEN_FILES] = { nullptr };

    FifoBuffer<FlashQueueItem, FLASH_FIFO_CAPACITY> writeFifo;

    void trackFile(FlashFile& fileHandle);
    void untrackFile(FlashFile& fileHandle);
};

#endif /* MEMORY_W25QXX_FLASHSTORAGE_H_ */
