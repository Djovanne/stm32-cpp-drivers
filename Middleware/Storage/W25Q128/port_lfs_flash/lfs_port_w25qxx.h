/*
 * lfs_port_w25q128.h
 *
 *  Created on: Jul 20, 2026
 *      Author: giova
 */

#ifndef MEMORY_LITTLEFS_LFS_PORT_W25Qxx_H_
#define MEMORY_LITTLEFS_LFS_PORT_W25Qxx_H_

#include "W25Qxx.h"
#include "lfs.h"
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MANUFACTORY_ID 0xEF
#define MEM_TYPE_ID    0x40

int stmlfs_hal_init(struct lfs_config *c, W25Qxx_HandleTypeDef *dev);

int stmlfs_hal_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size);

int stmlfs_hal_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size);

int stmlfs_hal_erase(const struct lfs_config *c, lfs_block_t block);

int stmlfs_hal_sync(const struct lfs_config *c);

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_LITTLEFS_LFS_PORT_W25Qxx_H_ */
