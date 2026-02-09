#pragma once
#include <stddef.h>
#include <stdint.h>

#define SECTOR_SIZE 512

int read_sector(uint32_t lba, void* buffer);
int read_sector_fat(uint32_t lba, uint8_t* buffer);
int write_sector(uint32_t lba, const void* buffer);

