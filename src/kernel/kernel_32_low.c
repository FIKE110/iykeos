#include <stdint.h>
#include <stddef.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define OS_LOAD_ADDRESS 0xB00000

#define OS_BIN_LBA 512
#define OS_SECTOR_COUNT 128

#define OS_SELECTION_ADDR 0x90000

typedef void (*entry_t)(void);

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ __volatile__("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static uint16_t cursor_pos = 0;
static int selected_option = 0;

#define KEY_UP    0x48
#define KEY_DOWN  0x50
#define KEY_ENTER 0x1C

#define MENU_BG_COLOR 0xF0

void print_char(char c) {
    uint16_t* vga = (uint16_t*)0xB8000;
    
    if (c == '\n') {
        cursor_pos = ((cursor_pos / VGA_WIDTH) + 1) * VGA_WIDTH;
    } else {
        vga[cursor_pos++] = (c | (MENU_BG_COLOR << 8));
    }
}

void print_string(const char* str) {
    while (*str) {
        print_char(*str++);
    }
}

void print_hex(uint32_t n) {
    const char* hex = "0123456789ABCDEF";
    for (int i = 28; i >= 0; i -= 4) {
        print_char(hex[(n >> i) & 0xF]);
    }
}

void clear_screen(void) {
    uint16_t* vga = (uint16_t*)0xB8000;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = (' ' | (MENU_BG_COLOR << 8));
    }
    cursor_pos = 0;
}

void update_arrow(int selected);

void draw_bracket(int row) {
    uint16_t* vga = (uint16_t*)0xB8000;
    for (int i = 0; i < VGA_WIDTH; i++) {
        vga[i + (row * 80)] = '=' | (MENU_BG_COLOR << 8);
    }
}

void draw_vertical_bracket(int col, int size) {
    uint16_t* vga = (uint16_t*)0xB8000;
    for (int i = 0; i < size; i++) {
        vga[col + (80 * i)] = '=' | (MENU_BG_COLOR << 8);
    }
}

void draw_menu(void) {
    const char* title = "WELCOME TO FORTUNE KERNEL , OPTIONS TO MOVE FORWARD ARE BELOW";
    const char* opt1 = "1.  BOOT INTO FORTUNE_OS CLI";
    const char* opt2 = "2.  BOOT INTO FORTUNE_OS GRAPHICS";
    const char* opt3 = "3.  CHECK OUT KERNEL GAMES";
    const char* opt4 = "4.  QUIT/EXIT";
    
    const char* options[4] = {opt1, opt2, opt3, opt4};
    
    // Draw borders
    draw_bracket(0);
    draw_vertical_bracket(0, VGA_HEIGHT);
    draw_bracket(2);
    draw_vertical_bracket(VGA_WIDTH - 1, VGA_HEIGHT);
    draw_bracket(VGA_HEIGHT - 1);
    
    // Draw title
    cursor_pos = 86;
    for (int i = 0; title[i] != '\0'; i++) {
        uint16_t* vga = (uint16_t*)0xB8000;
        vga[cursor_pos + i] = (title[i] | (MENU_BG_COLOR << 8));
    }
    
    // Draw options
    for (int i = 0; i < 4; i++) {
        cursor_pos = (80 * 7) + 10 + (80 * i * 3);
        for (int j = 0; options[i][j] != '\0'; j++) {
            uint16_t* vga = (uint16_t*)0xB8000;
            vga[cursor_pos + j] = (options[i][j] | (MENU_BG_COLOR << 8));
        }
    }
    
    // Draw initial arrow
    update_arrow(0);
}

void update_arrow(int selected) {
    uint16_t* vga = (uint16_t*)0xB8000;
    
    // Clear all arrows
    for (int i = 0; i < 4; i++) {
        vga[(80 * 7 + 10) + (80 * i * 3) - 3] = (' ' | (MENU_BG_COLOR << 8));
    }
    
    // Draw arrow at selected position
    vga[(80 * 7 + 10) + (80 * selected * 3) - 3] = ('>' | (MENU_BG_COLOR << 8));
}

void wait_key_release(uint8_t scancode) {
    while (inb(0x60) == scancode);
}

uint8_t get_key(void) {
    uint8_t scancode = inb(0x60);
    if (scancode != 0) {
        wait_key_release(scancode);
        return scancode;
    }
    return 0;
}

void menu_loop(void) {
    draw_menu();
    
    while (1) {
        uint8_t scancode = get_key();
        
        if (scancode == KEY_UP) {
            if (selected_option > 0) {
                selected_option--;
            }
            update_arrow(selected_option);
            wait_key_release(KEY_UP);
        } else if (scancode == KEY_DOWN) {
            if (selected_option < 3) {
                selected_option++;
            }
            update_arrow(selected_option);
            wait_key_release(KEY_DOWN);
        } else if (scancode == KEY_ENTER) {
            wait_key_release(KEY_ENTER);
            return;
        }
    }
}

int ata_read_sector(uint32_t lba, uint8_t* buffer) {
    uint32_t timeout = 100000;
    while ((inb(0x1F7) & 0x80) && --timeout);
    if (timeout == 0) return -1;
    
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F1, 0);
    outb(0x1F2, 1);
    outb(0x1F3, lba & 0xFF);
    outb(0x1F4, (lba >> 8) & 0xFF);
    outb(0x1F5, (lba >> 16) & 0xFF);
    outb(0x1F7, 0x20);
    
    timeout = 100000;
    uint8_t status;
    do {
        status = inb(0x1F7);
        if (status & 0x01) return -2;
        if (status & 0x20) return -3;
        if (--timeout == 0) return -4;
    } while (!(status & 0x08) && (status & 0x80));
    
    for (int i = 0; i < 256; i++) {
        ((uint16_t*)buffer)[i] = inw(0x1F0);
    }
    
    return 0;
}

int load_os_binary(void) {
    uint8_t* os_buffer = (uint8_t*)OS_LOAD_ADDRESS;
    uint8_t sector_buffer[512];
    
    print_string("Loading OS.BIN...\n");
    
    for (int i = 0; i < OS_SECTOR_COUNT; i++) {
        int ret = ata_read_sector(OS_BIN_LBA + i, sector_buffer);
        
        if (ret != 0) {
            print_string("\nDisk error at sector ");
            print_hex(i);
            print_string("\n");
            return -1;
        }
        
        uint32_t* dst = (uint32_t*)(os_buffer + i * 512);
        uint32_t* src = (uint32_t*)sector_buffer;
        for (int j = 0; j < 128; j++) {
            dst[j] = src[j];
        }
        
        if ((i % 16) == 0) {
            print_char('.');
        }
    }
    
    print_string("\nOS loaded successfully\n");
    return 0;
}

void kernel_main(void) {
    clear_screen();
    
    print_string("IYKEOS Kernel v1.0\n\n");
    
    menu_loop();
    
    if (selected_option == 3) {
        clear_screen();
        print_string("System halted. Safe to power off.\n");
        while (1) __asm__ __volatile__("hlt");
    }
    
    clear_screen();
    
    print_string("Selected option: ");
    print_char('1' + selected_option);
    print_string("\n\n");
    
    if (load_os_binary() != 0) {
        print_string("\nERROR: Failed to load OS.BIN\n");
        while (1) __asm__ __volatile__("hlt");
    }
    
    *(volatile uint32_t*)OS_SELECTION_ADDR = selected_option;
    
    print_string("Starting OS...\n");
    
    __asm__ __volatile__("cli");
    entry_t os_entry = (entry_t)OS_LOAD_ADDRESS;
    os_entry();
    
    while (1) __asm__ __volatile__("hlt");
}
