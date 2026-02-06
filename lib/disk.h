    #pragma once
#include <stddef.h>
#include <stdint.h>

#define SECTOR_SIZE 512

static inline void outb_p(unsigned short port, unsigned char val);
static inline unsigned char inb_p(unsigned short port);
static inline unsigned short inw_p(unsigned short port);

int read_sector(uint32_t lba, void* buffer);
int read_sector_fat(uint32_t lba, uint8_t* buffer);
int write_sector(uint32_t lba, const void* buffer);

