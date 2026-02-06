#include <file.h>
#include "disk.h"
#include <stdint.h>
#include <strings.h>


#define VGA_ADDRESS 0xB8000
static uint16_t* vga_buffer = (uint16_t*)VGA_ADDRESS;
static int debug_cursor_x = 0;
static int debug_cursor_y = 20;

void debug_putc(char c) {
    if (c == '\n') {
        debug_cursor_x = 0;
        debug_cursor_y++;
    } else if (c == '\r') {
        debug_cursor_x = 0;
    } else {
        if (debug_cursor_y < 25){
            vga_buffer[debug_cursor_y * 80 + debug_cursor_x] = (c | (0x0F << 8));
            debug_cursor_x++;
            if (debug_cursor_x >= 80) {
                debug_cursor_x = 0;
                debug_cursor_y++;
            }
        }
    }
}

void debug_print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        debug_putc(str[i]);
    }
}

void debug_print_hex(uint32_t n) {
    char hex_str[11];
    hex_to_str(n, hex_str);
    for (int i = 0; hex_str[i] != '\0'; i++) {
        debug_putc(hex_str[i]);
    }
}

fat16_dir_entry root_dir[MAX_ROOT_ENTRIES];
static uint16_t fat_table[65536]; // Support full FAT16 (128KB)
static uint32_t root_dir_sectors;
static uint32_t data_start_sector;
static uint8_t sectors_per_cluster;
static uint8_t temp[SECTOR_SIZE];
static uint16_t current_dir_cluster = 0; // 0 = Root

#define PARTITION_OFFSET 1

void fat16_init() {
    uint8_t sector[SECTOR_SIZE];
    
    if (read_sector(PARTITION_OFFSET, sector) != 0) return;

    uint16_t root_entries     = *(uint16_t*)&sector[17];
    uint16_t reserved_sectors = *(uint16_t*)&sector[14];
    uint8_t  fats             = sector[16];
    uint16_t sectors_per_fat  = *(uint16_t*)&sector[22];
    sectors_per_cluster = *(uint8_t*)&sector[0x0D];

    if (root_entries == 0) return;
    if (root_entries > MAX_ROOT_ENTRIES) root_entries = MAX_ROOT_ENTRIES;

    root_dir_sectors = ((root_entries * 32) + (SECTOR_SIZE - 1)) / SECTOR_SIZE;
    uint32_t root_start_sector = PARTITION_OFFSET + reserved_sectors + (fats * sectors_per_fat);
    data_start_sector = root_start_sector + root_dir_sectors;


    for (uint32_t i = 0; i < root_dir_sectors; i++) {
        uint8_t* sector_ptr = (uint8_t*)&root_dir[i * (SECTOR_SIZE / sizeof(fat16_dir_entry))];
        if (read_sector(root_start_sector + i, sector_ptr) != 0) break;

        int found_end = 0;
        for (int e = 0; e < (SECTOR_SIZE / sizeof(fat16_dir_entry)); e++) {
            if (sector_ptr[e * sizeof(fat16_dir_entry)] == 0x00) {
                found_end = 1;
                break;
            }
        }
        if (found_end) break;
    }


    uint32_t fat_size_in_uint16 = sectors_per_fat * (SECTOR_SIZE / 2);
    if (fat_size_in_uint16 > 65536) sectors_per_fat = 65536 / (SECTOR_SIZE / 2);

    for (uint32_t i = 0; i < sectors_per_fat; i++) {
        if (read_sector(PARTITION_OFFSET + reserved_sectors + i,
                        (uint8_t*)&fat_table[i * (SECTOR_SIZE / 2)]) != 0) break;
    }
}

void fat16_format_name(const char* filename, char out[11]) {
    memset(out, ' ', 11);
    int j = 0, i = 0;

    if (strcmp(filename, ".") == 0) {
        memcpy(out, ".          ", 11);
        return;
    }
    if (strcmp(filename, "..") == 0) {
        memcpy(out, "..         ", 11);
        return;
    }

    while (filename[i] && filename[i] != '.' && j < 8) {
        out[j++] = toupper(filename[i++]);
    }
    if (filename[i] == '.') i++;
    int k = 8;
    while (filename[i] && k < 11) {
        out[k++] = toupper(filename[i++]);
    }
}

int fat16_get_dir_entries(fat16_dir_entry* entries, int max_entries) {
    if (current_dir_cluster == 0) {
        int count = 0;
        for (int i = 0; i < MAX_ROOT_ENTRIES && count < max_entries; i++) {
            if (root_dir[i].name[0] == 0) break;
            entries[count++] = root_dir[i];
        }
        return count;
    } else {
        int count = 0;
        uint16_t cluster = current_dir_cluster;
        while (cluster >= 2 && cluster < 0xFFF8 && count < max_entries) {
            uint32_t sector = data_start_sector + (uint32_t)(cluster - 2) * sectors_per_cluster;
            for (uint8_t s = 0; s < sectors_per_cluster && count < max_entries; s++) {
                if (read_sector(sector + s, temp) != 0) return count;
                fat16_dir_entry* sector_entries = (fat16_dir_entry*)temp;
                for (int e = 0; e < (SECTOR_SIZE / sizeof(fat16_dir_entry)) && count < max_entries; e++) {
                    if (sector_entries[e].name[0] == 0) return count;
                    entries[count++] = sector_entries[e];
                }
            }
            cluster = fat_table[cluster];
        }

        return count;
    }
}

int fat16_file_exists(const char* filename) {
    char fatname[11];
    fat16_format_name(filename, fatname);

    fat16_dir_entry entries[MAX_FILES];
    int count = fat16_get_dir_entries(entries, MAX_FILES);

    for (int i = 0; i < count; i++) {
        if ((uint8_t)entries[i].name[0] == 0xE5) continue;
        if (memcmp(entries[i].name, fatname, 11) == 0) return 1;
    }
    return 0;
}



int fat16_file_load(const char* filename, uint8_t* buffer) {
    char fatname[11];
    fat16_format_name(filename, fatname);

    fat16_dir_entry entries[MAX_FILES];
    int count = fat16_get_dir_entries(entries, MAX_FILES);

    for (int i = 0; i < count; i++) {
        fat16_dir_entry* e = &entries[i];
        if ((uint8_t)e->name[0] == 0xE5) continue;

        if (memcmp(e->name, fatname, 11) != 0) continue;
        if (e->size == 0 && !(e->attr & 0x10)) return 0;

        uint16_t cluster = e->first_cluster;
        uint32_t bytes_left = e->size;
        if (e->attr & 0x10) bytes_left = 0xFFFFFFFF;

        uint8_t* ptr = buffer;

        while (cluster >= 2 && cluster < 0xFFF8 && bytes_left > 0) {
            uint32_t first_sector = data_start_sector + ((uint32_t)(cluster - 2) * sectors_per_cluster);

            for (uint8_t s = 0; s < sectors_per_cluster && bytes_left > 0; s++) {
                uint32_t sector = first_sector + s;
                if (read_sector_fat(sector, temp) != 0) return -2;

                uint32_t to_copy = (bytes_left < SECTOR_SIZE) ? bytes_left : SECTOR_SIZE;
                memcpy(ptr, temp, to_copy);
                ptr += to_copy;
                if (bytes_left != 0xFFFFFFFF) bytes_left -= to_copy;
            }

            uint16_t next_cluster = fat_table[cluster];
            if (next_cluster == cluster || next_cluster >= 0xFFF8) break;
            cluster = next_cluster;
        }
        return 0;
    }
    return -1;
}


int fat16_list_root(fat16_dir_entry* entries, int max_entries) {
    return fat16_get_dir_entries(entries, max_entries);
}

int fat16_find_free_entry() {
    if (current_dir_cluster == 0) {
        for (int i = 0; i < MAX_FILES; i++) {
            if (root_dir[i].name[0] == 0 || (uint8_t)root_dir[i].name[0] == 0xE5) return i;
        }
    } else {
        uint16_t cluster = current_dir_cluster;
        int entry_index = 0;
        while (cluster >= 2 && cluster < 0xFFF8) {
            uint32_t sector = data_start_sector + (uint32_t)(cluster - 2) * sectors_per_cluster;
            for (uint8_t s = 0; s < sectors_per_cluster; s++) {
                if (read_sector(sector + s, temp) != 0) return -1;
                fat16_dir_entry* sector_entries = (fat16_dir_entry*)temp;
                for (int e = 0; e < (SECTOR_SIZE / sizeof(fat16_dir_entry)); e++) {
                    if (sector_entries[e].name[0] == 0 || (uint8_t)sector_entries[e].name[0] == 0xE5) {
                        return entry_index + e;
                    }
                }
                entry_index += (SECTOR_SIZE / sizeof(fat16_dir_entry));
            }
            cluster = fat_table[cluster];
        }
    }
    return -1;
}

void fat16_update_entry(int slot, fat16_dir_entry* entry) {
    if (current_dir_cluster == 0) {
        root_dir[slot] = *entry;
    } else {
        uint16_t cluster = current_dir_cluster;
        int current_slot = 0;
        while (cluster >= 2 && cluster < 0xFFF8) {
            uint32_t sector = data_start_sector + (uint32_t)(cluster - 2) * sectors_per_cluster;
            for (uint8_t s = 0; s < sectors_per_cluster; s++) {
                if (read_sector(sector + s, temp) != 0) return;
                int entries_in_sector = (SECTOR_SIZE / sizeof(fat16_dir_entry));
                if (slot >= current_slot && slot < current_slot + entries_in_sector) {
                    fat16_dir_entry* sector_entries = (fat16_dir_entry*)temp;
                    sector_entries[slot - current_slot] = *entry;
                    write_sector(sector + s, temp);
                    return;
                }
                current_slot += entries_in_sector;
            }
            cluster = fat_table[cluster];
        }
    }
}

int fat16_find_free_cluster() {
    for (int i = 2; i < 0xFFF7; i++) {
        if (fat_table[i] == 0) return i;
    }
    return -1;
}

int fat16_create_file(const char* filename, uint32_t size) {
    if (fat16_file_exists(filename)) return -1;

    int slot = fat16_find_free_entry();
    if (slot < 0) return -1;

    int clusters_needed = (size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    if (size == 0) clusters_needed = 1; // Always allocate at least one cluster for simplicity

    uint16_t prev = 0;
    uint16_t first_cluster = 0;

    for (int i = 0; i < clusters_needed; i++) {
        int cluster = fat16_find_free_cluster();
        if (cluster < 0) return -1;
        if (i == 0) first_cluster = cluster;
        if (prev != 0) fat_table[prev] = cluster;
        prev = cluster;
        fat_table[cluster] = 0xFFFF;
    }

    fat16_dir_entry new_entry;
    memset(&new_entry, 0, sizeof(fat16_dir_entry));
    char fatname[11];
    fat16_format_name(filename, fatname);
    memcpy(new_entry.name, fatname, 11);
    new_entry.first_cluster = first_cluster;
    new_entry.size = size;
    new_entry.attr = 0;

    fat16_update_entry(slot, &new_entry);
    return 0;
}

int fat16_delete_file(const char* filename) {
    char fatname[11];
    fat16_format_name(filename, fatname);

    fat16_dir_entry entries[MAX_FILES];
    int count = fat16_get_dir_entries(entries, MAX_FILES);

    for (int i = 0; i < count; i++) {
        fat16_dir_entry* e = &entries[i];
        if ((uint8_t)e->name[0] == 0xE5) continue;
        if (memcmp(e->name, fatname, 11) == 0) {

            uint16_t cluster = e->first_cluster;
            while (cluster >= 2 && cluster < 0xFFF8) {
                uint16_t next = fat_table[cluster];
                fat_table[cluster] = 0;
                cluster = next;
            }
            e->name[0] = 0xE5;
            fat16_update_entry(i, e);
            return 0;
        }
    }
    return -1;
}

uint32_t fat16_file_size(const char* filename) {
    char fatname[11];
    fat16_format_name(filename, fatname);

    for (int i = 0; i < MAX_FILES; i++) {
        fat16_dir_entry* e = &root_dir[i];
        if (e->name[0] == 0) break;
        if ((uint8_t)e->name[0] == 0xE5) continue;
        if (memcmp(e->name, fatname, 11) == 0) return e->size;
    }
    return 0;
}




int fat16_file_save(const char* filename, uint8_t* buffer, uint32_t size) {
    int slot = -1;
    char fatname[11];
    fat16_format_name(filename, fatname);

    fat16_dir_entry entries[MAX_FILES];
    int count = fat16_get_dir_entries(entries, MAX_FILES);

    for (int i = 0; i < count; i++) {
        if (memcmp(entries[i].name, fatname, 11) == 0) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        slot = fat16_find_free_entry();
    }

    if (slot == -1) return -1;

    fat16_dir_entry e;
    if (slot < count) e = entries[slot];
    else memset(&e, 0, sizeof(fat16_dir_entry));
    if (e.first_cluster != 0) {
        uint16_t cluster = e.first_cluster;
        while (cluster >= 2 && cluster < 0xFFF7) {
            uint16_t next = fat_table[cluster];
            fat_table[cluster] = 0;
            cluster = next;
        }
    }


    uint16_t first_cluster = 0;
    uint16_t prev_cluster = 0;
    uint32_t bytes_left = size;
    uint8_t* ptr = buffer;

    if (size == 0) {
        first_cluster = fat16_find_free_cluster();
        if (first_cluster > 0) {
            fat_table[first_cluster] = 0xFFFF;
            memset(temp, 0, SECTOR_SIZE);
            write_sector(data_start_sector + (first_cluster - 2) * sectors_per_cluster, temp);
        }
    }

    while (bytes_left > 0) {
        int cluster = fat16_find_free_cluster();
        if (cluster == -1) return -2;

        if (first_cluster == 0) first_cluster = (uint16_t)cluster;
        if (prev_cluster != 0) fat_table[prev_cluster] = (uint16_t)cluster;
        
        fat_table[cluster] = 0xFFFF;
        prev_cluster = (uint16_t)cluster;

        uint32_t cluster_sector = data_start_sector + (cluster - 2) * sectors_per_cluster;
        for (int s = 0; s < sectors_per_cluster && bytes_left > 0; s++) {
            uint32_t to_copy = (bytes_left < SECTOR_SIZE) ? bytes_left : SECTOR_SIZE;
            memset(temp, 0, SECTOR_SIZE);
            memcpy(temp, ptr, to_copy);
            write_sector(cluster_sector + s, temp);
            ptr += to_copy;
            bytes_left -= to_copy;
        }
    }


    memcpy(e.name, fatname, 11);
    e.first_cluster = first_cluster;
    e.size = size;
    e.attr = 0;

    fat16_update_entry(slot, &e);


    uint8_t bpb_sector[SECTOR_SIZE];
    read_sector(PARTITION_OFFSET, bpb_sector);
    uint16_t reserved_sectors = *(uint16_t*)&bpb_sector[14];
    uint8_t  fats             = bpb_sector[16];
    uint16_t sectors_per_fat  = *(uint16_t*)&bpb_sector[22];


    for (int f = 0; f < fats; f++) {
        for (int i = 0; i < sectors_per_fat; i++) {
            write_sector(PARTITION_OFFSET + reserved_sectors + f * sectors_per_fat + i, 
                         (uint8_t*)&fat_table[i * (SECTOR_SIZE / 2)]);
        }
    }


    uint32_t root_start_sector = PARTITION_OFFSET + reserved_sectors + (fats * sectors_per_fat);
    for (int i = 0; i < root_dir_sectors; i++) {
        write_sector(root_start_sector + i, 
                     (uint8_t*)&root_dir[i * (SECTOR_SIZE / sizeof(fat16_dir_entry))]);
    }

    return 0;
}

int fat16_chdir(const char* dirname) {
    if (strcmp(dirname, "/") == 0 || strcmp(dirname, "\\") == 0) {
        current_dir_cluster = 0;
        return 0;
    }

    if (strcmp(dirname, ".") == 0) return 0;

    char fatname[11];
    fat16_format_name(dirname, fatname);

    fat16_dir_entry entries[MAX_FILES];
    int count = fat16_get_dir_entries(entries, MAX_FILES);

    for (int i = 0; i < count; i++) {
        if (memcmp(entries[i].name, fatname, 11) == 0) {
            if (entries[i].attr & 0x10) {
                current_dir_cluster = entries[i].first_cluster;
                return 0;
            }
            return -2;
        }
    }
    return -1;
}

int fat16_mkdir(const char* dirname) {
    if (fat16_file_exists(dirname)) return -1;

    int slot = fat16_find_free_entry();
    if (slot < 0) return -1;

    int cluster = fat16_find_free_cluster();
    if (cluster < 0) return -1;

    fat_table[cluster] = 0xFFFF;

    memset(temp, 0, SECTOR_SIZE);
    fat16_dir_entry* dot = (fat16_dir_entry*)temp;
    memcpy(dot[0].name, ".          ", 11);
    dot[0].attr = 0x10;
    dot[0].first_cluster = cluster;
    
    memcpy(dot[1].name, "..         ", 11);
    dot[1].attr = 0x10;
    dot[1].first_cluster = current_dir_cluster;

    uint32_t sector = data_start_sector + (uint32_t)(cluster - 2) * sectors_per_cluster;
    write_sector(sector, temp);

    fat16_dir_entry new_entry;
    memset(&new_entry, 0, sizeof(fat16_dir_entry));
    char fatname[11];
    fat16_format_name(dirname, fatname);
    memcpy(new_entry.name, fatname, 11);
    new_entry.first_cluster = cluster;
    new_entry.size = 0;
    new_entry.attr = 0x10;

    fat16_update_entry(slot, &new_entry);
    return 0;
}

int fat16_rmdir(const char* dirname) {
    char fatname[11];
    fat16_format_name(dirname, fatname);

    fat16_dir_entry entries[MAX_FILES];
    int count = fat16_get_dir_entries(entries, MAX_FILES);

    for (int i = 0; i < count; i++) {
        if (memcmp(entries[i].name, fatname, 11) == 0) {
            if (!(entries[i].attr & 0x10)) return -2;
            
            uint16_t cluster = entries[i].first_cluster;
            uint32_t sector = data_start_sector + (uint32_t)(cluster - 2) * sectors_per_cluster;
            if (read_sector(sector, temp) != 0) return -3;
            fat16_dir_entry* sub_entries = (fat16_dir_entry*)temp;
            for (int j = 2; j < (SECTOR_SIZE / sizeof(fat16_dir_entry)); j++) {
                if (sub_entries[j].name[0] != 0 && (uint8_t)sub_entries[j].name[0] != 0xE5) {
                    return -4;
                }
            }

            fat_table[cluster] = 0;
            entries[i].name[0] = 0xE5;
            fat16_update_entry(i, &entries[i]);
            return 0;
        }
    }
    return -1;
}