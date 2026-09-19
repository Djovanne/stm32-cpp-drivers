/*
 * FlashStorage.cpp
 *
 *  Created on: Aug 8, 2026
 *      Author: giova
 */

#include "lfs_port_w25qxx.h"
#include "FlashStorage.h"
#include <stdio.h>
#include <string.h>

FlashStorage::FlashStorage(SPI_HandleTypeDef* hspi, PinName cs, PinName wp)
{
    this->flashDev.spiHandle = hspi;
    this->flashDev.CS_Pin = cs;
    this->flashDev.WP_Pin = wp;

    this->isMounted = false;
    this->isDirOpen = false;
}

// ------------------------------------------------------------------------- //
void FlashStorage::trackFile(FlashFile& fileHandle)
{
    for (int i = 0; i < MAX_OPEN_FILES; i++)
    {
        if (openFilesTrack[i] == &fileHandle) return;
    }
    for (int i = 0; i < MAX_OPEN_FILES; i++)
    {
        if (openFilesTrack[i] == nullptr)
        {
            openFilesTrack[i] = &fileHandle;
            return;
        }
    }
}

// ------------------------------------------------------------------------- //
void FlashStorage::untrackFile(FlashFile& fileHandle)
{
    for (int i = 0; i < MAX_OPEN_FILES; i++)
    {
        if (openFilesTrack[i] == &fileHandle)
        {
            openFilesTrack[i] = nullptr;
            return;
        }
    }
}

// ------------------------------------------------------------------------- //
bool FlashStorage::mount(bool formatOnFail)
{
    if (isMounted) return true;

    if (stmlfs_hal_init(&lfsConfig, &flashDev) != LFS_ERR_OK) return false;

    int err = lfs_mount(&lfs, &lfsConfig);

    if (err && formatOnFail)
    {
        format();
        err = lfs_mount(&lfs, &lfsConfig);
    }

    isMounted = (err == LFS_ERR_OK);
    return isMounted;
}

// ------------------------------------------------------------------------- //
bool FlashStorage::unmount()
{
    if (!isMounted) return true;

    flushQueue();
    closeAllFiles();

    int err = lfs_unmount(&lfs);
    isMounted = false;

    return (err == LFS_ERR_OK);
}

// ------------------------------------------------------------------------- //
bool FlashStorage::format()
{
    flushQueue();

    if (lfsConfig.read_size == 0)
    {
        if (stmlfs_hal_init(&lfsConfig, &flashDev) != LFS_ERR_OK)
            return false;
    }

    int err = lfs_format(&lfs, &lfsConfig);
    return (err == LFS_ERR_OK);
}

// ------------------------------------------------------------------------- //
uint32_t FlashStorage::getFreeSpaceKB()
{
    if (!isMounted) return 0;

    lfs_ssize_t used_blocks = lfs_fs_size(&lfs);
    if (used_blocks < 0) return 0;

    uint32_t free_blocks = lfsConfig.block_count - (uint32_t)used_blocks;
    return (free_blocks * lfsConfig.block_size) / 1024;
}

// ------------------------------------------------------------------------- //
bool FlashStorage::isReady() const
{
    return isMounted;
}

// ------------------------------------------------------------------------- //
lfs_t* FlashStorage::getLfsInstance()
{
    return &lfs;
}

// Retorna o ponteiro alinhado com o header
W25Qxx_HandleTypeDef* FlashStorage::getFlashInstance()
{
    return &flashDev;
}

// ------------------------------------------------------------------------- //
bool FlashStorage::openFile(FlashFile& fileHandle, const char* filename, int flags)
{
    if (!isMounted) return false;
    if (fileHandle.isOpen) closeFile(fileHandle);

    int err = lfs_file_open(&lfs, &fileHandle.file, filename, flags);
    if (err == LFS_ERR_OK) {
        fileHandle.isOpen = true;
        trackFile(fileHandle);
        return true;
    }
    return false;
}

// ------------------------------------------------------------------------- //
bool FlashStorage::removeFile(const char* filename)
{
    if (!isMounted) return false;
    flushQueue();
    int err = lfs_remove(&lfs, filename);
    return (err == LFS_ERR_OK);
}

// ------------------------------------------------------------------------- //
bool FlashStorage::openFileForRead(FlashFile& fileHandle, const char* filename)
{
    return openFile(fileHandle, filename, LFS_O_RDONLY);
}

// ------------------------------------------------------------------------- //
bool FlashStorage::openFileForWrite(FlashFile& fileHandle, const char* filename, bool append)
{
    int flags = LFS_O_WRONLY | LFS_O_CREAT;
    flags |= append ? LFS_O_APPEND : LFS_O_TRUNC;
    return openFile(fileHandle, filename, flags);
}

// ------------------------------------------------------------------------- //
bool FlashStorage::closeFile(FlashFile& fileHandle)
{
    if (!isMounted || !fileHandle.isOpen) return false;

    flushQueue();

    int err = lfs_file_close(&lfs, &fileHandle.file);
    fileHandle.isOpen = false;
    untrackFile(fileHandle);

    return (err == LFS_ERR_OK);
}

// ------------------------------------------------------------------------- //
void FlashStorage::closeAllFiles()
{
    flushQueue();
    for (int i = 0; i < MAX_OPEN_FILES; i++)
    {
        if (openFilesTrack[i] != nullptr)
        {
            closeFile(*openFilesTrack[i]);
            openFilesTrack[i] = nullptr;
        }
    }
}

// ------------------------------------------------------------------------- //
bool FlashStorage::syncFile(FlashFile& fileHandle)
{
    if (!isMounted || !fileHandle.isOpen) return false;

    flushQueue();

    return (lfs_file_sync(&lfs, &fileHandle.file) == LFS_ERR_OK);
}

// ------------------------------------------------------------------------- //
uint32_t FlashStorage::getFileSize(FlashFile& fileHandle)
{
    if (!isMounted || !fileHandle.isOpen) return 0;
    flushQueue();
    return lfs_file_size(&lfs, &fileHandle.file);
}

// ------------------------------------------------------------------------- //
char* FlashStorage::readLine(FlashFile& fileHandle, char* buffer, int maxLen)
{
    if (!isMounted || !fileHandle.isOpen || maxLen <= 1) return NULL;

    flushQueue();

    int i = 0;
    while (i < maxLen - 1)
    {
        char c;
        lfs_ssize_t bytesRead = readData(fileHandle, &c, 1);

        if (bytesRead <= 0) break;

        buffer[i++] = c;
        if (c == '\n') break;
    }

    if (i == 0) return NULL;

    buffer[i] = '\0';
    return buffer;
}

// ------------------------------------------------------------------------- //
bool FlashStorage::openDir(const char* path)
{
    if (!isMounted) return false;
    if (isDirOpen) closeDir();

    flushQueue();

    int err = lfs_dir_open(&lfs, &lfsDir, path);
    if (err == LFS_ERR_OK) {
        isDirOpen = true;
        return true;
    }
    return false;
}

// ------------------------------------------------------------------------- //
bool FlashStorage::readNextFile(char* outName, uint32_t* outSize)
{
    if (!isMounted || !isDirOpen) return false;

    struct lfs_info info;

    while (lfs_dir_read(&lfs, &lfsDir, &info) > 0)
    {
        if (info.type == LFS_TYPE_REG)
        {
            strcpy(outName, info.name);
            *outSize = info.size;
            return true;
        }
    }
    return false;
}

// ------------------------------------------------------------------------- //
bool FlashStorage::closeDir()
{
    if (!isMounted || !isDirOpen) return false;

    int err = lfs_dir_close(&lfs, &lfsDir);
    isDirOpen = false;

    return (err == LFS_ERR_OK);
}

// ------------------------------------------------------------------------- //
lfs_ssize_t FlashStorage::writeData(FlashFile& fileHandle, const void* buffer, lfs_size_t size)
{
    if (!isMounted || !fileHandle.isOpen) return LFS_ERR_IO;
    return lfs_file_write(&lfs, &fileHandle.file, buffer, size);
}

// ------------------------------------------------------------------------- //
lfs_ssize_t FlashStorage::readData(FlashFile& fileHandle, void* buffer, lfs_size_t size)
{
    if (!isMounted || !fileHandle.isOpen) return LFS_ERR_IO;
    flushQueue();
    return lfs_file_read(&lfs, &fileHandle.file, buffer, size);
}

// ------------------------------------------------------------------------- //
bool FlashStorage::getNextAvailableName(char* outName, size_t outNameSize, const char* fullFileName, int maxAttempts)
{
    if (!isMounted || !fullFileName || !outName || outNameSize == 0) return false;

    flushQueue();

    char baseName[64];
    const char* ext = strrchr(fullFileName, '.');

    if (ext != nullptr)
    {
        size_t baseLen = ext - fullFileName;
        if (baseLen >= sizeof(baseName)) baseLen = sizeof(baseName) - 1;

        strncpy(baseName, fullFileName, baseLen);
        baseName[baseLen] = '\0';
    }
    else
    {
        strncpy(baseName, fullFileName, sizeof(baseName) - 1);
        baseName[sizeof(baseName) - 1] = '\0';
        ext = "";
    }

    snprintf(outName, outNameSize, "%s%s", baseName, ext);

    struct lfs_info info;
    if (lfs_stat(&lfs, outName, &info) != LFS_ERR_OK) return true;

    for (int counter = 1; counter <= maxAttempts; counter++)
    {
        snprintf(outName, outNameSize, "%s(%d)%s", baseName, counter, ext);
        if (lfs_stat(&lfs, outName, &info) != LFS_ERR_OK) return true;
    }

    outName[0] = '\0';
    return false;
}

// ------------------------------------------------------------------------- //
bool FlashStorage::enqueueData(FlashFile& fileHandle, const void* buffer, size_t size)
{
    if (!isMounted || !fileHandle.isOpen || size > FLASH_CHUNK_SIZE) return false;

    FlashQueueItem item;
    item.fileHandle = &fileHandle;
    item.size = size;
    memcpy(item.data, buffer, size);

    return writeFifo.push(item);
}

// ------------------------------------------------------------------------- //
void FlashStorage::update()
{
    if (!isMounted || writeFifo.isEmpty()) return;

    if (W25Qxx_IsBusy(&flashDev)) return;

    FlashQueueItem item;
    if (writeFifo.pop(item))
    {
        if (item.fileHandle && item.fileHandle->isOpen)
        {
            lfs_file_write(&lfs, &item.fileHandle->file, item.data, item.size);
        }
    }
}

// ------------------------------------------------------------------------- //
void FlashStorage::flushQueue()
{
    while (!writeFifo.isEmpty())
    {
        while (W25Qxx_IsBusy(&flashDev));

        FlashQueueItem item;
        if (writeFifo.pop(item))
        {
            if (item.fileHandle && item.fileHandle->isOpen)
            {
                lfs_file_write(&lfs, &item.fileHandle->file, item.data, item.size);
            }
        }
    }

    while (W25Qxx_IsBusy(&flashDev));
}
