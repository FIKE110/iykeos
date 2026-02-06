#include <stddef.h>
#include <stdint.h>
#include <disk.h>


#define ATA_DATA 0x1F0
#define ATA_SECTOR_COUNT 0x1F2
#define ATA_LBA_LOW 0x1F3
#define ATA_LBA_MID 0x1F4
#define ATA_LBA_HIGH 0x1F5
#define ATA_DRIVE 0x1F6
#define ATA_COMMAND 0x1F7
#define ATA_STATUS 0x1F7



static inline void outb_p(unsigned short port, unsigned char val) {
    __asm__ __volatile__ ("outb %0,%1" : : "a"(val), "Nd"(port));
}

static inline unsigned char inb_p(unsigned short port) {
    unsigned char ret;
    __asm__ __volatile__ ("inb %1,%0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline unsigned short inw_p(unsigned short port) {
    unsigned short ret;
    __asm__ __volatile__ ("inw %1,%0\n\tjmp 1f\n1:jmp 1f\n1:" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw_p(unsigned short port, unsigned short val) {
    __asm__ __volatile__ ("outw %0,%1" : : "a"(val), "Nd"(port));
}

static inline int ata_poll_drq(void) {
    while (inb_p(ATA_STATUS) & 0x80);
    while (!(inb_p(ATA_STATUS) & 0x08)) {
        if (inb_p(ATA_STATUS) & 0x01) return -1;
    }
    return 0;
}

static inline int ata_wait_flush(void) {
    while (inb_p(ATA_STATUS) & 0x80);
    if (inb_p(ATA_STATUS) & 0x01) return -1;
    return 0;
}

int read_sector(uint32_t lba, void* buffer) {
    outb_p(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb_p(ATA_SECTOR_COUNT, 1);
    outb_p(ATA_LBA_LOW, lba & 0xFF);
    outb_p(ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb_p(ATA_LBA_HIGH, (lba >> 16) & 0xFF);
    outb_p(ATA_COMMAND, 0x20);

    if (ata_poll_drq() != 0) {
        return -1;
    }

    uint16_t* buf16 = (uint16_t*)buffer;
    for(int i=0; i<256; i++) {
        buf16[i] = inw_p(ATA_DATA);
    }
    return 0;
}

int read_sector_fat(uint32_t lba, uint8_t* buffer) {
    outb_p(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb_p(ATA_SECTOR_COUNT, 1);
    outb_p(ATA_LBA_LOW, lba & 0xFF);
    outb_p(ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb_p(ATA_LBA_HIGH, (lba >> 16) & 0xFF);
    outb_p(ATA_COMMAND, 0x20);

    if (ata_poll_drq() != 0) return -1;

    for (int i = 0; i < 512; i += 2) {
        uint16_t word = inw_p(ATA_DATA);
        buffer[i] = word & 0xFF;
        buffer[i+1] = (word >> 8) & 0xFF;
    }

    return 0;
}

int write_sector(uint32_t lba, const void* buffer) {
    outb_p(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb_p(ATA_SECTOR_COUNT, 1);
    outb_p(ATA_LBA_LOW, lba & 0xFF);
    outb_p(ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb_p(ATA_LBA_HIGH, (lba >> 16) & 0xFF);
    outb_p(ATA_COMMAND, 0x30);

    if (ata_poll_drq() != 0) {
        return -1;
    }

    const uint16_t* buf16 = (const uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        outw_p(ATA_DATA, buf16[i]);
    }

    if (ata_wait_flush() != 0) {
        return -1;
    }

    return 0;
}
