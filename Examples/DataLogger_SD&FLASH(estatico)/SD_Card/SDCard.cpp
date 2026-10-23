/*
 * SDCard.cpp
 *
 *  Created on: Jan 14, 2026
 *      Author: giova
 */

#include <SDCard.hh>

SDCard::SDCard(const char* rootPath)
{
    strncpy(this->path, rootPath, sizeof(this->path));
}

bool SDCard::mount()
{
    // 1 = Montagem imediata
    res = f_mount(&fs, path, 1);
    if (res == FR_OK)
    {
    	debugPrint("Montagem feita com sucesso!\n\r");
    	return true;
    }
    // 2 = Tenta formatar
    else if (res == FR_NO_FILESYSTEM)
    {
    	debugPrint("Tentando formatar SD Card...\r\n");
    	BYTE work[512];
    	res = f_mkfs(path, FM_ANY, 0, work, sizeof(work));
    	if (res == FR_OK)
		{
			res = f_mount(&fs, path, 1);
			if (res == FR_OK)
			{
				debugPrint("Formatação concluida!\r\n");
				return true;
			}
		}
    	debugPrint("Falha na formatação\r\n");
    }
    return false;
}

bool SDCard::unmount() {
    res = f_mount(NULL, path, 0);
    return (res == FR_OK);
}

uint32_t SDCard::getFreeSpaceKB() {
    DWORD fre_clust;
    FATFS* pfs;

    res = f_getfree(path, &fre_clust, &pfs);
    if (res != FR_OK) return 0;

    // Calcula tamanho total e livre em setores
    uint32_t fre_sect = fre_clust * pfs->csize;
    // Retorna espaço livre em KB (assumindo setor de 512 bytes)
    return (fre_sect / 2);
}

// ==========================================
// MÉTODOS DE ESCRITA CONTÍNUA
// ==========================================

bool SDCard::openFileForAppend(const char* filename) {
    // FA_OPEN_ALWAYS: Abre se existir, cria se não existir.
    // FA_WRITE: Permissão de escrita.
    // FA_OPEN_APPEND: Move o ponteiro para o fim automaticamente.
    res = f_open(&fil, filename, FA_OPEN_ALWAYS | FA_WRITE | FA_OPEN_APPEND);
    return (res == FR_OK);
}

bool SDCard::writeToFile(const char* data) {
    UINT bytesWritten;
    res = f_write(&fil, data, strlen(data), &bytesWritten);
    return (res == FR_OK);
}

bool SDCard::printToFile(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    return writeToFile(buffer);
}

bool SDCard::closeFile() {
    // Fecha o arquivo forçando a atualização da FAT
    res = f_close(&fil);
    return (res == FR_OK);
}

// ==========================================
// MÉTODOS DE ESCRITA SIMPLES
// ==========================================

bool SDCard::writeLog(const char* filename, const char* data, bool append) {
    UINT bytesWritten;
    BYTE mode;

    if (append) {
        // Abre e move o ponteiro para o fim
        mode = FA_OPEN_ALWAYS | FA_WRITE | FA_OPEN_APPEND;
    }
    else
    {
        // Cria um novo arquivo
        mode = FA_CREATE_ALWAYS | FA_WRITE;
    }

    if (f_open(&fil, filename, mode) != FR_OK) return false;

    res = f_write(&fil, data, strlen(data), &bytesWritten);
    f_close(&fil);

    return (res == FR_OK);
}

bool SDCard::printLog(const char* filename, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    return writeLog(filename, buffer, true);
}

// ==========================================
// UTILITÁRIOS DE DIRETÓRIO
// ==========================================

bool SDCard::createPath(const char* dirPath) {
    char tempPath[256];
    char* p = NULL;

    strncpy(tempPath, path, sizeof(tempPath));
    p = strchr(tempPath + 1, '/');

    while (p != NULL) {
        *p = '\0';
        res = f_mkdir(tempPath);
        if (res != FR_OK && res != FR_EXIST) {
            return false;
        }
        *p = '/';
        p = strchr(p + 1, '/');
    }
    res = f_mkdir(tempPath);

    return (res == FR_OK || res == FR_EXIST);
}
