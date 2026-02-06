#pragma once
#include <stdint.h>
#include "disk.h"
#include <stddef.h>

#define SECTOR_SIZE 512
#define MAX_ROOT_ENTRIES 512
#define MAX_FILES MAX_ROOT_ENTRIES

typedef struct {
    uint8_t status;
    uint8_t first_chs[3];
    uint8_t type;
    uint8_t last_chs[3];
    uint32_t lba_begin;
    uint32_t size_in_sectors;
} __attribute__((packed)) partition_table_entry;

typedef struct {
    void (*print_shell)(char*);
    void (*load_vga)(int);
    void (*save_vga)(int);
    void (*screen_clear_shell)(void);
    char (*keyboard_read)(void);
} __attribute__((packed)) os_api_t_file;

typedef struct {
    char name[8];         // Filename
    char ext[3];          // Extension
    uint8_t attr;         // File attributes
    uint8_t reserved[10]; // Reserved
    uint16_t time;        // Last write time
    uint16_t date;        // Last write date
    uint16_t first_cluster; // First cluster of file
    uint32_t size;        // File size in bytes
} __attribute__((packed)) fat16_dir_entry;

extern fat16_dir_entry root_dir[MAX_ROOT_ENTRIES];

void fat16_init();
int fat16_file_exists(const char* filename);
int fat16_file_load(const char* filename, uint8_t* buffer);
int fat16_list_root(fat16_dir_entry* entries, int max_entries);
int fat16_create_file(const char* filename, uint32_t size);
int fat16_delete_file(const char* filename);
uint32_t fat16_file_size(const char* filename);
int fat16_file_save(const char* filename, uint8_t* buffer, uint32_t size);
int fat16_chdir(const char* dirname);
int fat16_mkdir(const char* dirname);
int fat16_rmdir(const char* dirname);

void my_memset(void* dest, uint8_t val, size_t n);
void my_memcpy(void* dest, const void* src, size_t n);
int my_memcmp(const void* a, const void* b, size_t n);