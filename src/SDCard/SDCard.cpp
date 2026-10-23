/*
 * SDCard.cpp
 *
 *  Created on: Jan 14, 2026
 *      Author: giova
 */

#include <SDCard.hh>

SDCard::SDCard(const char* rootPath) {
    // Copia o caminho raiz. No STM32Cube geralmente é definido em retSD
    // Se estiver vazio, o FATFS assume o drive padrão.
    strncpy(this->path, rootPath, sizeof(this->path));
}

bool SDCard::mount() {
    // 1 = Montagem imediata (força verificação do hardware agora)
    res = f_mount(&fs, path, 1);
    return (res == FR_OK);
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

bool SDCard::writeLog(const char* filename, const char* data) {
    UINT bytesWritten;

    // Abre ou cria arquivo.
    // FA_OPEN_ALWAYS: Abre se existir, cria se não existir.
    // FA_WRITE: Permissão de escrita.
    // FA_OPEN_APPEND: Move o ponteiro para o fim do arquivo automaticamente.
    if (f_open(&fil, filename, FA_OPEN_ALWAYS | FA_WRITE | FA_OPEN_APPEND) != FR_OK) return false;

    // Escreve os dados
    res = f_write(&fil, data, strlen(data), &bytesWritten);

    // Fecha o arquivo para salvar a FAT
    f_close(&fil);

    return (res == FR_OK);
}

bool SDCard::printLog(const char* filename, const char* format, ...) {
    va_list args;
    va_start(args, format);

    // Formata a string no buffer interno
    vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    // Reutiliza a função de escrita simples
    return writeLog(filename, buffer);
}

bool SDCard::createPath(const char* dirPath)
{
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
