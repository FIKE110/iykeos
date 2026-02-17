#include <stdint.h>
#include <stddef.h>
#include <file.h>
#include <disk.h>
#include <memory_os.h>
#include <strings.h>
#include <graphics.h>
#include "../lib/vgraphics.h"

void start_shell();
void disable_cursor();
void enable_cursor();



static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb_s(uint16_t port) {
    uint8_t ret;
   __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void mouse_wait(uint8_t type) {
    uint32_t time_out = 100000;
    if (type == 0) {
        while (time_out--) {
            if ((inb_s(0x64) & 1) == 1) return;
        }
        return;
    } else {
        while (time_out--) {
            if ((inb_s(0x64) & 2) == 0) return;
        }
        return;
    }
}

static inline void mouse_write(uint8_t write) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, write);
}

static inline uint8_t mouse_read() {
    mouse_wait(0);
    return inb_s(0x60);
}

static int8_t mouse_byte[3];
static uint8_t mouse_cycle = 0;

void mouse_handler(int* mx, int* my, int* left_click, int* right_click) {
    uint8_t status = inb_s(0x64);
    if (!(status & 1)) return;
    if (!(status & 0x20)) return;

    uint8_t mouse_in = inb_s(0x60);

    switch (mouse_cycle) {
        case 0:
            if ((mouse_in & 0x08) == 0x08) {
                mouse_byte[0] = mouse_in;
                mouse_cycle++;
            }
            break;
        case 1:
            mouse_byte[1] = mouse_in;
            mouse_cycle++;
            break;
        case 2:
            mouse_byte[2] = mouse_in;
            mouse_cycle = 0;

            if (mouse_byte[0] & 1) *left_click = 1;
            if (mouse_byte[0] & 2) *right_click = 1;

            int dx = (char)mouse_byte[1];
            int dy = (char)mouse_byte[2];

            *mx += dx / 10;
            *my -= dy / 10;

            if (*mx < 0) *mx = 0;
            if (*mx > 79) *mx = 79;
            if (*my < 0) *my = 0;
            if (*my > 24) *my = 24;
            break;
    }
}

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_SIZE (VGA_WIDTH*VGA_HEIGHT)
#define VGA_ADDRESS 0xB8000

static uint16_t* vga_buffer = (uint16_t*) VGA_ADDRESS;
static uint16_t* temp_buffer = (uint16_t*) 0xC80000;
static uint8_t cursor_row = 0;
static uint8_t cursor_col = 0;
static uint8_t temp_cursor_col[50];
static uint8_t temp_cursor_row[50];
static uint8_t shell_color = 0x0F;

 int shift_pressed = 0;
 int capslock_on = 0;

 typedef void (*entry_t)();

#define KEY_UP          0x11
#define KEY_DOWN        0x12
#define KEY_LEFT        0x13
#define KEY_RIGHT       0x14

#define KEY_CTRL        0x15
#define KEY_ALT         0x16
#define KEY_SHIFT       0x17
#define KEY_CAPSLOCK    0x18
#define KEY_FN          0x19
#define KEY_ESC         0x1B
#define KEY_ENTER       '\n'
#define KEY_BACKSPACE   0x08
#define KEY_TAB         '\t'
#define KEY_SPACE       ' '
#define KEY_DELETE      0x7F

#define KEY_F1          0x80
#define KEY_F2          0x81
#define KEY_F3          0x82
#define KEY_F4          0x83
#define KEY_F5          0x84
#define KEY_F6          0x85
#define KEY_F7          0x86
#define KEY_F8          0x87
#define KEY_F9          0x88
#define KEY_F10         0x89
#define KEY_F11         0x8A
#define KEY_F12         0x8B

#define KEY_HOME        0x8C
#define KEY_END         0x8D
#define KEY_PAGEUP      0x8E
#define KEY_PAGEDOWN    0x8F
#define KEY_INSERT      0x90

#define COLOR_BLACK         0x0
#define COLOR_BLUE          0x1
#define COLOR_GREEN         0x2
#define COLOR_CYAN          0x3
#define COLOR_RED           0x4
#define COLOR_MAGENTA       0x5
#define COLOR_BROWN         0x6
#define COLOR_LIGHT_GRAY    0x7
#define COLOR_DARK_GRAY     0x8
#define COLOR_LIGHT_BLUE    0x9
#define COLOR_LIGHT_GREEN   0xA
#define COLOR_LIGHT_CYAN    0xB
#define COLOR_LIGHT_RED     0xC
#define COLOR_LIGHT_MAGENTA 0xD
#define COLOR_YELLOW        0xE
#define COLOR_WHITE         0xF


void screen_print_shell(const char* str);
void enable_cursor();
void screen_clear_shell();
void screen_putc_shell(char c);
void move_cursor(size_t row, size_t col);
char keyboard_read();
uint16_t vga_entry(char c, uint8_t color);
void load_vga(int page);
void save_vga(int page);
void rng_seed();
uint32_t get_random(uint32_t max);
void busy_delay(uint32_t count);
void set_color(uint8_t color);
void beep(uint32_t freq, uint32_t duration);
void set_vga_mode(uint8_t mode);
void draw_pixel(int x, int y, uint8_t color);
void save_file(const char* filename, uint8_t* buffer);
void change_cursor(uint8_t x, uint8_t y);
char keyboard_getchar();
void restore_vga_font_safe();

typedef struct {
    void (*print_shell)(const char*);
    void (*print_shellc)(char);
    void (*load_vga)(int);
    void (*save_vga)(int);
    void (*screen_clear_shell)(void);
    char (*keyboard_read)(void);
    void (*move_cursor)(size_t,size_t);
    void (*change_cursor)(uint8_t,uint8_t);
    void (*save_file)(const char*, uint8_t*);
    void (*rng_seed)(void);
    uint32_t (*get_random)(uint32_t);
    void (*busy_delay)(uint32_t);
    void (*set_color)(uint8_t);
    char (*keyboard_getchar)(void);
    void (*beep)(uint32_t, uint32_t);
    void (*set_vga_mode)(uint8_t);
    void (*draw_pixel)(int, int, uint8_t);
    int (*load_file)(const char*, uint8_t*);
    int (*delete_file)(const char*);
    void (*list_files)(void);
    int (*get_file_list)(char* files);
    void (*disable_cursor)(void);
    void (*mouse_init)(void);
    void (*enable_cursor)(void);
    void (*keyboard_init)(void);
    void (*mouse_handler)(int*, int*, int*, int*);
    void (*run)(char*);
    void (*get_rtc_time)(uint8_t*, uint8_t*, uint8_t*);
    void (*get_rtc_date)(uint8_t*, uint8_t*, uint8_t*);
    void (*save_file_size)(char*, uint8_t*, uint32_t);
    size_t (*strlen)(const char*);
    void (*graphics_init)(void);
    int (*fat16_create_file)(const char *filename, uint32_t size);
    int (*fat16_mkdir)(const char *dirname);
    int (*fat16_file_load)(const char *filename, uint8_t *buffer);
    int (*fat16_file_exists)(const char *filename);
    int (*fat16_file_save)(const char *filename, uint8_t *buffer, uint32_t size);
    void (*debug_print)(const char* str);
    void (*debug_print_hex)(uint32_t n);
    void (*debug_putc)(char c);
    int (*strcmp)(const char* s1, const char* s2);
    char* (*strcpy)(char* dest, const char* src);
    char* (*strcat)(char* dest, const char* src);
    int (*memcmp)(const void* s1, const void* s2, size_t n);
    void (*memset)(void* dest, uint8_t val, size_t n);
    void (*memcpy)(void* dest, const void* src, size_t n);
    char (*toupper)(char c);
    void (*hex_to_str)(uint32_t n, char* dest);
    void (*int_to_str)(uint32_t n, char* dest);
    int (*fat16_delete_file)(const char* filename);
    int (*fat16_chdir)(const char* dirname);
    int (*fat16_rmdir)(const char* dirname);
    int (*fat16_list_root)(fat16_dir_entry* entries, int max_entries);
    void (*start_shell)();
    void (*graphics_loading_screen)();
    void (*graphics_clear_screen_g)(uint8_t color);
    void (*graphics_put_char)(int x, int y, char c, uint8_t color);
    void (*graphics_put_string)(int x, int y, const char* str, uint8_t color);
    void (*graphics_draw_box)(int x, int y, int w, int h, uint8_t color);
    void (*graphics_draw_button)(int x, int y, int w, int h, const char* label, uint8_t color);
    void (*graphics_draw_window)(int x, int y, int w, int h, const char* title, uint8_t color);
    void (*graphics_put_pixel)(int x, int y, uint8_t color);
    void (*vgraphics_init)(void);
    void (*vgraphics_clear)(uint8_t color);
    void (*vgraphics_repaint)(void);
    void (*vgraphics_put_char)(int x, int y, char c, uint8_t color);
    void (*vgraphics_put_string)(int x, int y, const char* str, uint8_t color);
    void (*vgraphics_draw_box)(int x, int y, int w, int h, uint8_t color);
    void (*vgraphics_draw_window)(int x, int y, int w, int h, const char* title, uint8_t color);
    void (*vgraphics_draw_rect_fill)(int x, int y, int w, int h, uint8_t color);
    int (*run_with_status)(char* filename,uint32_t address);
    uint32_t (*fat16_file_size)(char* filename);
    int (*str_to_int)(const char *s);

} os_api_t;



static uint32_t rng_state;
static uint32_t boot_cycles_low;
static uint32_t boot_cycles_high;
static int gfx_mode = 0;


void mouse_init() {
    uint8_t status;

    mouse_wait(1);
    outb(0x64, 0xA8);

    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    status = (inb_s(0x60) & ~2);
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);

    mouse_write(0xF6);
    mouse_read();

    mouse_write(0xF4);
    mouse_read();
}


uint8_t g_320x200x256[] = {
    0x63, 0x03, 0x01, 0x0F, 0x00, 0x0E, 0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
    0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9C, 0x8E, 0x8F, 0x28, 0x40, 0x96,
    0xB9, 0xA3, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF, 0x00, 0x01,
    0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x41, 0x00, 0x0F, 0x00, 0x00
};



uint8_t g_80x25_text[] = {
    0x67,
    0x03, 0x00, 0x03, 0x00, 0x02,
    0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
    0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x00,
    0x9C, 0x8E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
    0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
    0xFF,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x04, 0x00, 0x0F, 0x08, 0x00
};

uint8_t g_640x480x16[] = {
    0xE3,
    0x03, 0x01, 0x0F, 0x00, 0x02,
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
    0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xEA, 0x8C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
    0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0F,
    0xFF,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x01, 0x00, 0x0F, 0x00, 0x00
};

void write_vga_regs(uint8_t *regs) {
    unsigned i;

    outb(0x3C2, *regs++);
    for(i = 0; i < 5; i++) { outb(0x3C4, i); outb(0x3C5, *regs++); }

    outb(0x3D4, 0x03); outb(0x3D5, inb_s(0x3D5) | 0x80);
    outb(0x3D4, 0x11); outb(0x3D5, inb_s(0x3D5) & ~0x80);

    for(i = 0; i < 25; i++) { outb(0x3D4, i); outb(0x3D5, *regs++); }
    for(i = 0; i < 9; i++) { outb(0x3CE, i); outb(0x3CF, *regs++); }
    for(i = 0; i < 21; i++) { inb_s(0x3DA); outb(0x3C0, i); outb(0x3C0, *regs++); }
    inb_s(0x3DA); outb(0x3C0, 0x20);

    if(1) {
        uint16_t *vga_text = (uint16_t*)0xB8000;
        for(i = 0; i < 80*25; i++) vga_text[i] = 0x0720;

        outb(0x3D4, 0x0F); outb(0x3D5, 0x00);
        outb(0x3D4, 0x0E); outb(0x3D5, 0x00);
    }
}

static uint32_t boot_cycles_low;
static uint32_t boot_cycles_high;

uint8_t read_cmos(uint8_t reg) {
    outb(0x70, reg);
    return inb_s(0x71);
}

uint8_t bcd_to_bin(uint8_t bcd) {
    return ((bcd / 16) * 10) + (bcd % 16);
}

void get_rtc_time(uint8_t* h, uint8_t* m, uint8_t* s) {
    *s = bcd_to_bin(read_cmos(0x00));
    *m = bcd_to_bin(read_cmos(0x02));
    *h = bcd_to_bin(read_cmos(0x04));
}

void get_rtc_date(uint8_t* d, uint8_t* m, uint8_t* y) {
    *d = bcd_to_bin(read_cmos(0x07));
    *m = bcd_to_bin(read_cmos(0x08));
    *y = bcd_to_bin(read_cmos(0x09));
}

void beep(uint32_t freq, uint32_t duration) {
    uint32_t div;
    uint8_t tmp;

    div = 1193180 / freq;
    outb(0x43, 0xB6);
    outb(0x42, (uint8_t)(div & 0xFF));
    outb(0x42, (uint8_t)((div >> 8) & 0xFF));

    tmp = inb_s(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }

    busy_delay(duration);

    tmp = inb_s(0x61) & 0xFC;
    outb(0x61, tmp);
}

void set_vga_mode(uint8_t mode) {
    if (mode == 0x12) {
        screen_print_shell("Switching to 640x480 mode\n");
        write_vga_regs(g_640x480x16);
        gfx_mode = 1;
        uint8_t* vga_mem = (uint8_t*)0xA0000;
        for(int i=0; i<640*480/8; i++) vga_mem[i] = 0;
    } else if (mode == 0x13) {
        screen_print_shell("Switching to 320x200 mode\n");
        write_vga_regs(g_320x200x256);
        gfx_mode = 1;
        uint8_t* vga_mem = (uint8_t*)0xA0000;
        for(int i=0; i<320*200; i++) vga_mem[i] = 0;
    } else {
        restore_vga_font_safe();
        screen_print_shell("Switching to text mode\n");
        write_vga_regs(g_80x25_text);
        gfx_mode = 0;
        screen_clear_shell();
    }
}

void putpixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= 640 || y < 0 || y >= 480) return;

    uint8_t* fb = (uint8_t*)0xA0000;
    uint32_t offset = (y * 80) + (x / 8);

    for (int plane = 0; plane < 4; plane++) {
        outb(0x3C4, 2);
        outb(0x3C5, 1 << plane);

        uint8_t bit = 7 - (x % 8);
        uint8_t val = (color >> plane) & 1;
        fb[offset] = (fb[offset] & ~(1 << bit)) | (val << bit);
    }
}

void draw_pixel(int x, int y, uint8_t color) {
    if (gfx_mode) {
       putpixel(x, y, color);
    } else {
        if (x >= 0 && x < 80 && y >= 0 && y < 25) {
            vga_buffer[y * 80 + x] = (uint16_t)(' ' | (color << 12));
        }
    }
}

void set_color(uint8_t color) {
    shell_color = color;
}

void busy_delay(uint32_t count) {
    for (volatile uint32_t i = 0; i < count * 1000; i++) {
        __asm__("nop");
    }
}

static inline uint32_t rdtsc(void) {
    uint32_t lo;
    __asm__ volatile ("rdtsc" : "=a"(lo) : : "edx");
    return lo;
}

void rng_seed(void) {
    rng_state = rdtsc() ^ 0xA5A5A5A5;
    if (rng_state == 0) rng_state = 1;
}

uint32_t get_random(uint32_t max) {
    if (max == 0) return 0;
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state % max;
}

#define OS_API_ADDR 0x5F0F0

os_api_t* os_api;

 uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | (uint16_t) color << 8;
}

static uint8_t prompt_row = 0;
static uint8_t prompt_col = 0;
#define INPUT_BUFFER_SIZE 128
static char input_buffer[INPUT_BUFFER_SIZE];
static int input_length = 0;
static int input_cursor = 0;

#define HISTORY_SIZE 10
static char history[HISTORY_SIZE][INPUT_BUFFER_SIZE];
static int history_count = 0;
static int history_index = -1;

void redraw_input_line() {
    cursor_row = prompt_row;
    cursor_col = prompt_col;
    move_cursor(cursor_row, cursor_col);

    for (int i = 0; i < input_length; i++) {
        screen_putc_shell(input_buffer[i]);
    }

    screen_putc_shell(' ');
    screen_putc_shell('\b');

    int target_col = prompt_col + input_cursor;
    int target_row = prompt_row + (target_col / VGA_WIDTH);
    target_col %= VGA_WIDTH;

    cursor_row = target_row;
    cursor_col = target_col;
    move_cursor(cursor_row, cursor_col);
}

uint8_t saved_font[4096];

void save_vga_font() {
    outb(0x3CE, 0x04); outb(0x3CF, 0x02);

    uint8_t *vga_mem = (uint8_t *)0xA0000;
    for (int i = 0; i < 4096; i++) {
        saved_font[i] = vga_mem[i];
    }

    outb(0x3CE, 0x04); outb(0x3CF, 0x00);
}

void restore_vga_font_safe() {
    inb_s(0x3DA);
    outb(0x3C4, 0x00); outb(0x3C5, 0x01);
    outb(0x3C4, 0x02); outb(0x3C5, 0x04);
    outb(0x3C4, 0x04); outb(0x3C5, 0x07);
    outb(0x3C4, 0x00); outb(0x3C5, 0x03);

    outb(0x3CE, 0x04); outb(0x3CF, 0x02);
    outb(0x3CE, 0x05); outb(0x3CF, 0x00);
    outb(0x3CE, 0x06); outb(0x3CF, 0x00);

    uint8_t *vga_mem = (uint8_t *)0xA0000;
    for (int i = 0; i < 4096; i++) {
        vga_mem[i] = saved_font[i];
    }

    outb(0x3C4, 0x00); outb(0x3C5, 0x01);
    outb(0x3C4, 0x02); outb(0x3C5, 0x03);
    outb(0x3C4, 0x04); outb(0x3C5, 0x03);
    outb(0x3C4, 0x00); outb(0x3C5, 0x03);

    outb(0x3CE, 0x04); outb(0x3CF, 0x00);
    outb(0x3CE, 0x05); outb(0x3CF, 0x10);
    outb(0x3CE, 0x06); outb(0x3CF, 0x0E);
}

void restore_vga_font() {
    outb(0x3C4, 0x02); outb(0x3C5, 0x04);
    outb(0x3C4, 0x04); outb(0x3C5, 0x07);

    outb(0x3CE, 0x04); outb(0x3CF, 0x02);
    outb(0x3CE, 0x05); outb(0x3CF, 0x00);
    outb(0x3CE, 0x06); outb(0x3CF, 0x00);

    uint8_t *vga_mem = (uint8_t *)0xA0000;
    for (int i = 0; i < 4096; i++) {
        vga_mem[i] = saved_font[i];
    }

    outb(0x3C4, 0x02); outb(0x3C5, 0x03);
    outb(0x3C4, 0x04); outb(0x3C5, 0x03);
    outb(0x3CE, 0x04); outb(0x3CF, 0x00);
    outb(0x3CE, 0x05); outb(0x3CF, 0x10);
    outb(0x3CE, 0x06); outb(0x3CF, 0x0E);
}

void clear_input_line() {
    cursor_row = prompt_row;
    cursor_col = prompt_col;
    move_cursor(cursor_row, cursor_col);

    for (int i = 0; i < input_length; i++) {
        screen_putc_shell(' ');
    }

    cursor_row = prompt_row;
    cursor_col = prompt_col;
    move_cursor(cursor_row, cursor_col);

    input_length = 0;
    input_cursor = 0;
    input_buffer[0] = '\0';
}

void scroll_if_needed() {
    if (cursor_row >= VGA_HEIGHT) {
        for (int y = 1; y < VGA_HEIGHT; y++) {
            for (int x = 0; x < VGA_WIDTH; x++) {
                vga_buffer[(y - 1) * VGA_WIDTH + x] = vga_buffer[y * VGA_WIDTH + x];
            }
        }

        uint16_t blank = vga_entry(' ', (0x0 << 4 | 0xF));
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH +  x] = blank;
        }

        cursor_row = VGA_HEIGHT - 1;
    }
}

static char current_path[128] = "/";

void print_prompt() {
    screen_print_shell("IYKEOS:");
    screen_print_shell(current_path);
    screen_print_shell("> ");
    prompt_row = cursor_row;
    prompt_col = cursor_col;
    enable_cursor();
}

static const char scancode_ascii[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'','`',   0, '\\',
    'z','x','c','v','b','n','m',',','.','/',   0,   '*', 0, ' ',
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static const char shift_map[128] = {
    0, 27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0, 'A','S','D','F','G','H','J','K','L',':','"','~', 0,
    '|','Z','X','C','V','B','N','M','<','>','?',0, '*', 0, ' '
};

void keyboard_init() {
    // Ensure keyboard interface is enabled (command 0xAE)
    // mouse_wait(1);
    // outb(0x64, 0xAE);
    
    // // Clear any pending keyboard data
    // while (inb_s(0x64) & 1) {
    //     inb_s(0x60);
    // }
    // Reset keyboard state variables
    // shift_pressed = 0;
    // capslock_on = 0;
}

char scancode_to_ascii(uint8_t scancode) {

    char ascii = shift_pressed ? shift_map[scancode] : scancode_ascii[scancode];

    if (capslock_on) {
        if (ascii >= 'a' && ascii <= 'z') ascii -= 32;
        if (shift_pressed && ascii >= 'A' && ascii <= 'Z') ascii += 32;
    }

    return ascii;
}

char keyboard_getchar() {
    if (!(inb_s(0x64) & 1)) {
        return 0;
    }

    uint8_t scancode = inb_s(0x60);

    if (scancode == 0x2A || scancode == 0x36) shift_pressed = 1;
    if (scancode == 0xAA || scancode == 0xB6) shift_pressed = 0;

if (scancode == 0x3A) {
    capslock_on = !capslock_on;
    return 0;
}

    if (scancode & 0x80) {
        scancode &= 0x7F;
        if (scancode == 0x1D) return KEY_CTRL;
        if (scancode == 0x48) return KEY_UP;
        if (scancode == 0x50) return KEY_DOWN;
        if (scancode == 0x4B) return KEY_LEFT;
        if (scancode == 0x4D) return KEY_RIGHT;
        return 0;
    }

    // uint8_t code=scancode;
    // code=code & 0x7F;
    // if (code == 0x48) return KEY_UP;
    // if (code == 0x50) return KEY_DOWN;
    // if (code == 0x4B) return KEY_LEFT;
    // if (code == 0x4D) return KEY_RIGHT;

    return scancode_to_ascii(scancode);
}

char keyboard_read() {
    char c;
       while ((c = keyboard_getchar()) == 0) ;
       
    return c;
}

void save_vga(int page){
    uint16_t* temp = temp_buffer;
    for(int i=0;i<VGA_SIZE;i++){
        temp[i]=vga_buffer[i];
    }
    temp_cursor_col[page]=cursor_col;
    temp_cursor_row[page]=cursor_row;
}

void load_vga(int page){
    uint16_t* temp=temp_buffer;
    for(int i=0;i<VGA_SIZE;i++){
        vga_buffer[i]=temp[i];
    }
     cursor_col=temp_cursor_col[page];
    cursor_row= temp_cursor_row[page];
}

int starts_with(const char* str, const char* prefix) {
    while (*prefix) {
        if (*str != *prefix) {
            return 0;
        }
        str++;
        prefix++;
    }
    return 1;
}

int list_files_as_string(char *files)
{
    fat16_dir_entry entries[MAX_FILES];
    int count = fat16_list_root(entries, MAX_FILES);

    int idx = 0;

        for (int i = 0; i < count; i++) {
        if (entries[i].attr == 0x0F) {
            continue;
        }


        if (i > 0) {
            files[idx++] = '|';
        }

        for (int j = 0; j < 8; j++) {

            if (entries[i].name[j] != ' ') {
                files[idx++] = entries[i].name[j];
            }
        }

        int has_ext = 0;
        for (int j = 0; j < 3; j++) {
            if (entries[i].ext[j] != ' ') {
                has_ext = 1;
                break;
            }
        }

        if (has_ext) {
            files[idx++] = '.';
            for (int j = 0; j < 3; j++) {
                if (entries[i].ext[j] != ' ') {
                    files[idx++] = entries[i].ext[j];
                }
            }
        }

        if (entries[i].attr & 0x10) {
            files[idx++] = '/';
        }
    }

    files[idx] = '\0';

    return count;
}

void list_files() {
    fat16_dir_entry entries[MAX_FILES];
    int count = fat16_list_root(entries, MAX_FILES);
    int col = 0;


    for (int i = 0; i < count; i++) {

        if (entries[i].attr == 0x0F) {
            continue;
        }

        int chars_printed = 0;
        for (int j = 0; j < 8; j++) {
            if (entries[i].name[j] != ' ') {
                screen_putc_shell(entries[i].name[j]);
                chars_printed++;
            }
        }
        int has_ext = 0;
        for (int j = 0; j < 3; j++) {
            if (entries[i].ext[j] != ' ') { has_ext = 1; break; }
        }
        if (has_ext) {
            screen_putc_shell('.');
            chars_printed++;
            for (int j = 0; j < 3; j++) {
                if (entries[i].ext[j] != ' ') {
                    screen_putc_shell(entries[i].ext[j]);
                    chars_printed++;
                }
            }
        }
        if (entries[i].attr & 0x10) {
            screen_putc_shell('/');
            chars_printed++;
        }

        while (chars_printed < 16) {
            screen_putc_shell(' ');
            chars_printed++;
        }

        col++;
        if (col >= 5) {
            col = 0;
        }

    }


    if (col != 0) screen_putc_shell('\n');
}

void save_file(const char* filename, uint8_t* buffer) {
    fat16_file_save(filename, buffer, strlen((char*)buffer));
}

void save_file_size(char* filename, uint8_t* buffer, uint32_t size) {
    fat16_file_save(filename, buffer, size);
}

void run(char* filename){
        const char* ext = ".BIN";
        int filename_len = strlen(filename);
        int ext_len = strlen(ext);

        if (filename_len > ext_len && strcmp(filename + filename_len - ext_len, ext) == 0) {
            uint8_t* pointer = (uint8_t*) 0xD00000;
            uint32_t max_size = 65536;
            uint32_t file_size = fat16_file_size(filename);

            if (file_size == 0) {
                screen_print_shell("Error: File not found or is empty.\n");
            } else if (file_size > max_size) {
                screen_print_shell("Error: File is too large to execute.\n");
            } else {
                int res = fat16_file_load(filename, pointer);
                if (res == 0) {
                    keyboard_init();  // Reset keyboard before running program
                    entry_t entry = (entry_t) pointer;
                    entry();
                } else {
                    screen_print_shell("Error: Could not load file.\n");
                }
            }
        } else {
            screen_print_shell("Error: Invalid file type. Only .BIN files can be executed.\n");
        }

}


int run_with_status(char* filename,uint32_t address){
        const char* ext = ".BIN";
        int filename_len = strlen(filename);
        int ext_len = strlen(ext);


        if(address==0){
            address=0xD00000;
        }

        screen_print_shell("running program");
        if (filename_len > ext_len && strcmp(filename + filename_len - ext_len, ext) == 0) {
            uint8_t* pointer = (uint8_t*) address;
            uint32_t max_size = 65536;
            uint32_t file_size = fat16_file_size(filename);

            if (file_size == 0) {
                return -1;
            } else if (file_size > max_size) {
                return -1;
            } else {
                int res = fat16_file_load(filename, pointer);
                if (res == 0) {
                    keyboard_init();  // Reset keyboard before running program
                    entry_t entry = (entry_t) pointer;
                    entry();
                    return 0;
                } else {
                    return -1;
                }
            }

        } else {
            return -1;
        }

        return 0;

}

void split_args(const char* cmd, char* arg1, char* arg2) {
    while (*cmd && *cmd != ' ') cmd++;
    while (*cmd == ' ') cmd++;
    while (*cmd && *cmd != ' ') *arg1++ = *cmd++;
    *arg1 = '\0';
    while (*cmd == ' ') cmd++;
    while (*cmd && *cmd != ' ') *arg2++ = *cmd++;
    *arg2 = '\0';
}

void handle_command(const char* cmd) {
    if (cmd[0] == '\0') {
        return;
    }

    if (strcmp(cmd, "help") == 0) {
        screen_print_shell("Commands: help, clear, version, uptime, date, time, reboot, exit\n");
        screen_print_shell("Files:    ls, cat <f>, touch <f>, rm <f>, cp <s> <d>, mv <s> <d>\n");
        screen_print_shell("Dirs:     mkdir <d>, rmdir <d>, cd <d>, pwd\n");
        screen_print_shell("Apps:     edit <f>, basic <f>, gfx, text\n");
    }

    else if (strcmp(cmd, "clear") == 0) {
        screen_clear_shell();
        move_cursor(0, 0);
    }

    else if (strcmp(cmd, "version") == 0) {
        screen_print_shell("IYKEOS Shell v0.2 - Made by CHIHURUM FORTUNE!\n");
    }

    else if (starts_with(cmd, "echo")) {
        screen_print_shell(cmd + 5);
        screen_putc_shell('\n');
    }

    else if (strcmp(cmd, "window") == 0) {
        run_with_status("GUI.BIN", 0x01000000);
    }
     else if (starts_with(cmd, "random")) {
        char* number = (char*)(cmd + 7);
        int rand=str_to_int(number);
        if(rand==0){
            rand=10;
        }
        uint32_t r=get_random(rand);
        char c[10];
        int_to_str(r,c);
        screen_print_shell(c);
        screen_putc_shell('\n');
    }
    else if (strcmp(cmd, "games") == 0) {
        run_with_status("GAME.BIN", 0x01200000);
    }
    else if(starts_with(cmd,"edit ")){
        char* filename = (char*)(cmd + 5);
        uint8_t* pointer=(uint8_t*) 0xD00000;
        uint32_t max_size=65536;
        uint32_t file_size=fat16_file_size("EDITOR.ike");
        char* text_buffer=(char*) 0xBAB000;
        char* current_filename=text_buffer - 12;
        fat16_file_load(filename, (uint8_t *)text_buffer);
        uint8_t filename_counter=0;
        for (size_t i = 0; i < 8; i++){
            current_filename[i] = filename[i];
            filename_counter++;
            if(filename[i]=='.'){
                break;
            }
        }

        for (size_t i = 0; i < 3; i++)
        {
            current_filename[filename_counter] = filename[filename_counter];
            filename_counter++;
        }

        if(file_size==0){
            screen_print_shell("Error: File not found or is empty.\n");
        }else if(file_size>max_size){
            screen_print_shell("Error: File is too large to execute.\n");
        }else{
            int res=fat16_file_load("EDITOR.ike",pointer);
            if(res==0){
                entry_t entry=(entry_t) pointer;
                entry();
            }else{
                screen_print_shell("Error: Could not load file.\n");
            }
        }

    }

     else if(starts_with(cmd,"basic ")){
        char* filename = (char*)(cmd + 6);
        uint8_t* pointer=(uint8_t*) 0xD00000;
        uint32_t max_size=65536;
        uint32_t file_size=fat16_file_size("BASIC.ike");
        uint32_t file_size_exec=fat16_file_size(filename);
        if(file_size_exec==0){
             screen_print_shell("Error: File not found or is empty.\n");
        }
        else if (file_size>max_size){
            screen_print_shell("Error: File is too large to execute.\n");
        }

        char* text_buffer=(char*) 0xB00000;
        char* current_filename=text_buffer - 12;
        fat16_file_load(filename, (uint8_t *)text_buffer);
        text_buffer[file_size_exec] = '\0';
        uint8_t filename_counter=0;
        for (size_t i = 0; i < 8; i++){
            current_filename[i] = filename[i];
            filename_counter++;
            if(filename[i]=='.'){
                break;
            }
        }

        for (size_t i = 0; i < 3; i++)
        {
            current_filename[filename_counter] = filename[filename_counter];
            filename_counter++;
        }

        if(file_size==0){
            screen_print_shell("Error: File not found or is empty.\n");
        }else if(file_size>max_size){
            screen_print_shell("Error: File is too large to execute.\n");
        }else{
            int res=fat16_file_load("BASIC.ike",pointer);
            if(res==0){
                entry_t entry=(entry_t) pointer;
                entry();
            }else{
                screen_print_shell("Error: Could not load file.\n");
            }
        }

    }


    else if (strcmp(cmd, "reboot") == 0) {
        screen_print_shell("System rebooting...\n");
        busy_delay(100);
        outb(0x64, 0xFE);
    }

    else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "shutdown") == 0) {
        screen_print_shell("System halted. Safe to power off.\n");
        while(1) __asm__("hlt");
    }

    else if (strcmp(cmd, "gfx") == 0) {
        set_vga_mode(0x12);
        for(int y=0; y<480; y++) {
            for(int x=0; x<640; x++) {
                draw_pixel(x, y, (x + y) % 16);
            }
        }
        keyboard_read();

        set_vga_mode(0x03);

        uint16_t* text_buf = (uint16_t*)0xB8000;
        uint16_t blank = 0x0F20;
        for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
            text_buf[i] = blank;
        }

        vga_buffer = (uint16_t*) VGA_ADDRESS;
        cursor_row = 0;
        cursor_col = 0;
        enable_cursor();
        move_cursor(0, 0);
        screen_print_shell("Returned from graphics mode.\n");
    }

    else if (strcmp(cmd, "text") == 0) {
        set_vga_mode(0x03);
    }

    else if (strcmp(cmd, "uptime") == 0) {
        uint32_t current_low, current_high;
        __asm__ volatile ("rdtsc" : "=a"(current_low), "=d"(current_high));
        uint32_t diff = current_low - boot_cycles_low;

        char buf[32];
        hex_to_str(diff, buf);
        screen_print_shell("Uptime (CPU cycles): 0x");
        screen_print_shell(buf);
        screen_print_shell("\n");
    }

    else if (strcmp(cmd, "date") == 0) {
        uint8_t d, m, y;
        get_rtc_date(&d, &m, &y);
        char buf[4];
        int_to_str(m, buf); screen_print_shell(buf); screen_putc_shell('/');
        int_to_str(d, buf); screen_print_shell(buf); screen_print_shell("/20");
        int_to_str(y, buf); screen_print_shell(buf); screen_putc_shell('\n');
    }

    else if (strcmp(cmd, "time") == 0) {
        uint8_t h, m, s;
        get_rtc_time(&h, &m, &s);
        char buf[4];
        int_to_str(h, buf); screen_print_shell(buf); screen_putc_shell(':');
        if (m < 10) screen_putc_shell('0');
        int_to_str(m, buf); screen_print_shell(buf); screen_putc_shell(':');
        if (s < 10) screen_putc_shell('0');
        int_to_str(s, buf); screen_print_shell(buf); screen_putc_shell('\n');
    }

    else if (strcmp(cmd, "ls") == 0) {
        list_files();
    }

    else if (starts_with(cmd, "cat ")) {
        uint8_t* buffer = (uint8_t*)0xC20000;
        const char* filename = cmd + 4;
        uint32_t size = fat16_file_size(filename);

        if (size == 0) {
            if (fat16_file_exists(filename)) {
            } else {
                screen_print_shell("Error: File not found.\n");
            }
        } else {
            memset(buffer, 0, size + 1);
            int res = fat16_file_load(filename, buffer);
            if (res == 0) {
                for (uint32_t i = 0; i < size; i++) {
                    screen_putc_shell(buffer[i]);
                }

                if(buffer[size-1]!='\n' && buffer[size-1]!='\r'){
                    screen_putc_shell('\n');
                }
            } else {
                screen_print_shell("Error: Could not load file.\n");
            }
        }
    }
    else if(starts_with(cmd,"hello")){
            os_api->print_shell("HELLO WORLD\n");
        }

    else if(starts_with(cmd,"touch ")){
        const char* filename = cmd + 6;
        fat16_file_save(filename, (uint8_t*)"", 0);
        screen_print_shell("File created.\n");
    }
    else if(starts_with(cmd,"rm ")){
        const char* filename = cmd + 3;
        if (fat16_delete_file(filename) == 0) {
            screen_print_shell("File deleted.\n");
        } else {
            screen_print_shell("Error: Could not delete file.\n");
        }
    }
    else if(starts_with(cmd,"cp ")){
        char src[32], dest[32];
        split_args(cmd, src, dest);
        uint32_t size = fat16_file_size(src);
        if (size > 0) {
            uint8_t* buffer = (uint8_t*)0xC20000;
            if (fat16_file_load(src, buffer) == 0) {
                fat16_file_save(dest, buffer, size);
                screen_print_shell("File copied.\n");
            } else {
                screen_print_shell("Error: Could not load source file.\n");
            }
        } else {
            screen_print_shell("Error: Source file not found or empty.\n");
        }
    }
    else if(starts_with(cmd,"mv ")){
        char src[32], dest[32];
        split_args(cmd, src, dest);
        uint32_t size = fat16_file_size(src);
        if (size > 0) {
            uint8_t* buffer = (uint8_t*)0xC20000;
            if (fat16_file_load(src, buffer) == 0) {
                if (fat16_file_save(dest, buffer, size) == 0) {
                    fat16_delete_file(src);
                    screen_print_shell("File moved.\n");
                } else {
                    screen_print_shell("Error: Could not save destination file.\n");
                }
            } else {
                screen_print_shell("Error: Could not load source file.\n");
            }
        } else {
            screen_print_shell("Error: Source file not found or empty.\n");
        }
    }
    else if(starts_with(cmd,"mkdir ")){
        const char* dirname = cmd + 6;
        if (fat16_mkdir(dirname) == 0) {
            screen_print_shell("Directory created.\n");
        } else {
            screen_print_shell("Error: Could not create directory.\n");
        }
    }
    else if(starts_with(cmd,"rmdir ")){
        const char* dirname = cmd + 6;
        int res = fat16_rmdir(dirname);
        if (res == 0) {
            screen_print_shell("Directory removed.\n");
        } else if (res == -4) {
            screen_print_shell("Error: Directory not empty.\n");
        } else {
            screen_print_shell("Error: Could not remove directory.\n");
        }
    }
    else if(starts_with(cmd,"cd ")){
        const char* dirname = cmd + 3;
        if (fat16_chdir(dirname) == 0) {
            if (strcmp(dirname, "/") == 0 || strcmp(dirname, "..") == 0) {
                if (strcmp(dirname, "/") == 0) {
                    current_path[0] = '/';
                    current_path[1] = '\0';
                } else if (strcmp(dirname, "..") == 0) {
                    int len = strlen(current_path);
                    if (len > 1) {
                        int i = len - 1;
                        if (current_path[i] == '/') i--;
                        while (i > 0 && current_path[i] != '/') i--;
                        if (i == 0) current_path[1] = '\0';
                        else current_path[i+1] = '\0';
                    }
                }
            } else {
                int len = strlen(current_path);
                if (len > 1 && current_path[len-1] != '/') {
                    current_path[len] = '/';
                    len++;
                }
                int i = 0;
                while (dirname[i] && len < 127) current_path[len++] = dirname[i++];
                current_path[len] = '\0';
            }
        } else {
            screen_print_shell("Error: Could not change directory.\n");
        }
    }
    else if(strcmp(cmd,"pwd") == 0){
        screen_print_shell(current_path);
        screen_print_shell("\n");
    }
    else if (starts_with(cmd, "utils")) {
        const char* args = cmd + 5;
        if (*args == ' ') args++;
    }
    else if(starts_with(cmd,"iyke ")){
        const char* filename = cmd + 5;
        const char* ext = ".ike";
        int filename_len = strlen(filename);
        int ext_len = strlen(ext);

        if (filename_len > ext_len && strcmp(filename + filename_len - ext_len, ext) == 0) {
            uint8_t* pointer = (uint8_t*) 0xD00000;
            uint32_t max_size = 65536;
            uint32_t file_size = fat16_file_size(filename);

            if (file_size == 0) {
                screen_print_shell("Error: File not found or is empty.\n");
            } else if (file_size > max_size) {
                screen_print_shell("Error: File is too large to execute.\n");
            } else {
                int res = fat16_file_load(filename, pointer);
                if (res == 0) {
                    entry_t entry = (entry_t) pointer;
                    entry();
                } else {
                    screen_print_shell("Error: Could not load file.\n");
                }
            }
        } else {
            screen_print_shell("Error: Invalid file type. Only .ike files can be executed.\n");
        }
    }
     else if(starts_with(cmd,"run ")){
        const char* filename = cmd + 4;
        run((char*)filename);

    }
    else {
        screen_print_shell("Unknown command: ");
        screen_print_shell((char*)cmd);
        screen_putc_shell('\n');
    }
}

void move_cursor(size_t row, size_t col) {
    uint16_t pos = row * 80 + col;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void shell() {
     uint16_t* vga_buffer = (uint16_t*) VGA_ADDRESS;
    vga_buffer[cursor_row * VGA_WIDTH + cursor_col] = vga_entry('C', 0x0F);
    screen_print_shell("Welcome to IYKEOS!\n\n");
    print_prompt();

    input_length = 0;
    input_cursor = 0;

    while (1) {
        char c = keyboard_read();

        if (c != 0) {
            if (c == '\n') {
                screen_putc_shell('\n');
                input_buffer[input_length] = '\0';

                if (input_length > 0) {
                    for (int i = HISTORY_SIZE - 1; i > 0; i--) {
                        memcpy(history[i], history[i-1], INPUT_BUFFER_SIZE);
                    }
                    memcpy(history[0], input_buffer, INPUT_BUFFER_SIZE);
                    if (history_count < HISTORY_SIZE) history_count++;
                }
                history_index = -1;

                handle_command(input_buffer);

                input_length = 0;
                input_cursor = 0;
                print_prompt();
            } else if (c == '\b') {
                if (input_cursor > 0) {
                    for (int i = input_cursor - 1; i < input_length - 1; i++) {
                        input_buffer[i] = input_buffer[i+1];
                    }
                    input_length--;
                    input_cursor--;
                    redraw_input_line();
                }
            } else if (c == KEY_UP) {
                if (history_count > 0 && history_index < history_count - 1) {
                    history_index++;
                    clear_input_line();
                    memcpy(input_buffer, history[history_index], INPUT_BUFFER_SIZE);
                    input_length = strlen(input_buffer);
                    input_cursor = input_length;
                    screen_print_shell(input_buffer);
                }
            } else if (c == KEY_DOWN) {
                if (history_index > 0) {
                    history_index--;
                    clear_input_line();
                    memcpy(input_buffer, history[history_index], INPUT_BUFFER_SIZE);
                    input_length = strlen(input_buffer);
                    input_cursor = input_length;
                    screen_print_shell(input_buffer);
                } else if (history_index == 0) {
                    history_index = -1;
                    clear_input_line();
                }
            } else if (c == KEY_LEFT) {
                if (input_cursor > 0) {
                    input_cursor--;
                    redraw_input_line();
                }
            } else if (c == KEY_RIGHT) {
                if (input_cursor < input_length) {
                    input_cursor++;
                    redraw_input_line();
                }
            } else {
                if (input_length < INPUT_BUFFER_SIZE - 1) {
                    for (int i = input_length; i > input_cursor; i--) {
                        input_buffer[i] = input_buffer[i-1];
                    }
                    input_buffer[input_cursor] = c;
                    input_length++;
                    input_cursor++;
                    redraw_input_line();
                }
            }
        }

    }
}

void screen_clear_shell() {
    uint16_t blank = vga_entry(' ', shell_color);
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = blank;
        }
    }
    cursor_row = 0;
    cursor_col = 0;
}

void screen_putc_shell(char c) {
     uint16_t* vga_buffer = (uint16_t*) VGA_ADDRESS;
    if(c=='\t' || c==KEY_CTRL){
        return;
    }
    enable_cursor();
    if (c == '\n') {
        cursor_row++;
        cursor_col = 0;
    } else if (c == '\b') {
        if ((cursor_row > prompt_row) || (cursor_row == prompt_row && cursor_col > prompt_col)) {
            if (cursor_col > 0) {
                cursor_col--;
            } else {
                cursor_row--;
                cursor_col = VGA_WIDTH - 1;
            }
            vga_buffer[cursor_row * VGA_WIDTH + cursor_col] = vga_entry(' ', shell_color);
        }
    } else {
        vga_buffer[cursor_row * VGA_WIDTH + cursor_col] = vga_entry(c, shell_color);
        cursor_col++;
        if (cursor_col >= VGA_WIDTH) {
            cursor_col = 0;
            cursor_row++;
        }
    }
    scroll_if_needed();
    move_cursor(cursor_row, cursor_col);
}

void change_cursor(uint8_t x,uint8_t y){
    cursor_row=x;
    cursor_col=y;
}

void disable_cursor() {
    outb(0x3D4, 0x0A);
    outb(0x3D5, -1);

    outb(0x3D4, 0x0B);
    outb(0x3D5, -1);
}

void enable_cursor() {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0);

    outb(0x3D4, 0x0B);
    outb(0x3D5, 15);
}
void screen_print_shell(const char* str) {
    while (*str) {
        screen_putc_shell(*str++);
    }
}

void run_utils(const char* args) {
    if (args == NULL || args[0] == '\0') {
        screen_print_shell("Usage: utils <option>\n");
        return;
    }
    screen_print_shell("Utils command not implemented yet: ");
    screen_print_shell(args);
    screen_putc_shell('\n');
}

void print_shellc(char c){
    screen_putc_shell(c);
}

void debug_print(const char* str);
void debug_print_hex(uint32_t n);
void debug_putc(char c);
void graphics_init();

void init_api(){
    os_api=(os_api_t*) OS_API_ADDR;
    os_api->print_shell=&screen_print_shell;
    os_api->load_vga=&load_vga;
    os_api->save_vga=&save_vga;
    os_api->screen_clear_shell=&screen_clear_shell;
    os_api->keyboard_read=&keyboard_read;
    os_api->print_shellc=&print_shellc;
    os_api->move_cursor=&move_cursor;
    os_api->change_cursor=&change_cursor;
    os_api->save_file=&save_file;
    os_api->rng_seed=&rng_seed;
    os_api->get_random=&get_random;
    os_api->busy_delay=&busy_delay;
    os_api->set_color=&set_color;
    os_api->keyboard_getchar=&keyboard_getchar;
    os_api->beep=&beep;
    os_api->set_vga_mode=&set_vga_mode;
    os_api->draw_pixel=&draw_pixel;
    os_api->load_file=&fat16_file_load;
    os_api->delete_file=&fat16_delete_file;
    os_api->list_files=&list_files;
    os_api->get_file_list=&list_files_as_string;
    os_api->mouse_init=&mouse_init;
    os_api->disable_cursor=&disable_cursor;
    os_api->enable_cursor=&enable_cursor;
    os_api->keyboard_init=&keyboard_init;
    os_api->mouse_handler=&mouse_handler;
    os_api->run=&run;
    os_api->get_rtc_time=&get_rtc_time;
    os_api->get_rtc_date=&get_rtc_date;
    os_api->save_file_size=&save_file_size;
    os_api->strlen=&strlen;
    os_api->graphics_init=&graphics_init;
    os_api->fat16_create_file=&fat16_create_file;
    os_api->fat16_mkdir=&fat16_mkdir;
    os_api->fat16_file_load=&fat16_file_load;
    os_api->fat16_file_exists=&fat16_file_exists;
    os_api->fat16_file_save=&fat16_file_save;
    os_api->debug_print=&debug_print;
    os_api->debug_print_hex=&debug_print_hex;
    os_api->debug_putc=&debug_putc;
    os_api->strcat=&strcat;
    os_api->memset=&memset;
    os_api->strcpy=&strcpy;
    os_api->fat16_chdir=&fat16_chdir;
    os_api->fat16_rmdir=&fat16_rmdir;
    os_api->fat16_list_root=&fat16_list_root;
    os_api->start_shell=&start_shell;
    os_api->graphics_clear_screen_g=&clear_screen_g;
    os_api->graphics_draw_box=&draw_box;
    os_api->graphics_draw_button=&draw_button;
    os_api->graphics_draw_window=&draw_window;
    os_api->graphics_loading_screen=&loading_screen;
    os_api->graphics_put_char=&put_char;
    os_api->graphics_put_string=&put_string;
    os_api->graphics_put_pixel=&putpixel;
    os_api->int_to_str=&int_to_str;
    os_api->memcmp=&memcmp;
    os_api->vgraphics_init = &vgraphics_init;
    os_api->vgraphics_clear = &vgraphics_clear;
    os_api->vgraphics_repaint = &vgraphics_repaint;
    os_api->vgraphics_put_char = &vgraphics_put_char;
    os_api->vgraphics_put_string = &vgraphics_put_string;
    os_api->vgraphics_draw_box = &vgraphics_draw_box;
    os_api->vgraphics_draw_window = &vgraphics_draw_window;
    os_api->vgraphics_draw_rect_fill = &vgraphics_draw_rect_fill;
    os_api->run_with_status=&run_with_status;
    os_api->fat16_file_size=&fat16_file_size;
    os_api->str_to_int=&str_to_int;
}

void start_shell(){
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    (void)inb_s(0x60);  // Read and discard status
    mouse_wait(1);
    outb(0x64, 0xA7);

    enable_cursor();
    screen_clear_shell();
    shell();
}

#define OS_SELECTION_ADDR 0x90000

void os_main(void){
    save_vga_font();
    init_api();
    __asm__ volatile ("rdtsc" : "=a"(boot_cycles_low), "=d"(boot_cycles_high));
    rng_seed();
    fat16_init();

    uint32_t selection = *(volatile uint32_t*)OS_SELECTION_ADDR;

    if(selection == 0){
        start_shell();
    }
    else if(selection == 1){
        run_with_status("GUI.BIN", 0x01000000);
    }
    else if(selection == 2){
        run_with_status("GAME.BIN", 0x01200000);
        start_shell();
    }

    while (1) __asm__ __volatile__("hlt");
}
