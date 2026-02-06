#include "graphics.h"
#include <stdint.h>

uint16_t* vga_mem;


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

#define VGA_COLOR(fg, bg) ((bg << 4) | (fg))

void graphics_init() {
    vga_mem = (uint16_t*) 0xB8000;
}

void loading_screen() {
    // Clear screen with dark blue background
    clear_screen_g(COLOR_BLUE);
    
    uint8_t title_color = VGA_COLOR(COLOR_WHITE, COLOR_BLUE);
    uint8_t text_color = VGA_COLOR(COLOR_LIGHT_CYAN, COLOR_BLUE);
    uint8_t border_color = VGA_COLOR(COLOR_LIGHT_GRAY, COLOR_BLUE);
    uint8_t bar_fg = VGA_COLOR(COLOR_WHITE, COLOR_BLUE);
    uint8_t bar_bg = VGA_COLOR(COLOR_DARK_GRAY, COLOR_BLUE);
    uint8_t version_color = VGA_COLOR(COLOR_LIGHT_GRAY, COLOR_BLUE);
    
    // Draw decorative border
    int box_x = 15, box_y = 6, box_w = 50, box_h = 13;
    draw_box(box_x, box_y, box_w, box_h, border_color);
    
    // Corner decorations
    put_char(box_x, box_y, '#', border_color);
    put_char(box_x + box_w - 1, box_y, '#', border_color);
    put_char(box_x, box_y + box_h - 1, '#', border_color);
    put_char(box_x + box_w - 1, box_y + box_h - 1, '#', border_color);
    
    // IYKEOS Logo
    const char* logo_line1 = "  III   Y   Y   K   K   EEEEE   OOO   SSSS ";
    const char* logo_line2 = "   I     Y Y    K  K    E      O   O  S    ";
    const char* logo_line3 = "   I      Y     KKK     EEEE   O   O  SSSS ";
    const char* logo_line4 = "   I      Y     K  K    E      O   O     S ";
    const char* logo_line5 = "  III     Y     K   K   EEEEE   OOO   SSSS ";
    
    put_string(20, 8, logo_line1, title_color);
    put_string(20, 9, logo_line2, title_color);
    put_string(20, 10, logo_line3, title_color);
    put_string(20, 11, logo_line4, title_color);
    put_string(20, 12, logo_line5, title_color);
    
    // Version and author info
    put_string(30, 14, "v0.2 - Made by CHIHURUM FORTUNE", version_color);
    
    // Progress bar area
    int bar_y = 16;
    int bar_width = 40;
    int bar_x = 20;
    
    // Draw progress bar background
    for (int i = 0; i < bar_width; i++) {
        put_char(bar_x + i, bar_y, 176, bar_bg);  // Shaded block
    }
    
    // Animate progress bar filling
    const char* loading_text = "Loading...";
    put_string(35, bar_y - 2, loading_text, text_color);
    
    for (int progress = 0; progress <= bar_width; progress++) {
        // Fill progress bar
        for (int i = 0; i < progress; i++) {
            char bar_char;
            uint8_t bar_color;
            
            // Gradient effect
            if (i < bar_width / 3) {
                bar_char = 219;  // Full block
                bar_color = VGA_COLOR(COLOR_GREEN, COLOR_BLUE);
            } else if (i < (bar_width * 2) / 3) {
                bar_char = 219;
                bar_color = VGA_COLOR(COLOR_YELLOW, COLOR_BLUE);
            } else {
                bar_char = 219;
                bar_color = VGA_COLOR(COLOR_LIGHT_CYAN, COLOR_BLUE);
            }
            
            put_char(bar_x + i, bar_y, bar_char, bar_color);
        }
        
        // Percentage indicator
        int percent = (progress * 100) / bar_width;
        char percent_str[5];
        percent_str[0] = (percent / 100) + '0';
        percent_str[1] = ((percent / 10) % 10) + '0';
        percent_str[2] = (percent % 10) + '0';
        percent_str[3] = '%';
        percent_str[4] = '\0';
        
        if (percent < 10) {
            percent_str[0] = percent_str[1];
            percent_str[1] = percent_str[2];
            percent_str[2] = percent_str[3];
            percent_str[3] = '\0';
        } else if (percent < 100) {
            percent_str[0] = percent_str[1];
            percent_str[1] = percent_str[2];
            percent_str[2] = percent_str[3];
            percent_str[3] = '\0';
        }
        
        put_string(62, bar_y, percent_str, text_color);
        
        // Delay for animation
        for (volatile int d = 0; d < 800000; d++);
    }
    
    // Show "Ready!" message
    put_string(36, bar_y + 2, "Ready!", VGA_COLOR(COLOR_LIGHT_GREEN, COLOR_BLUE));
    
    // Pause for 2 seconds to let user see the loading screen
    for (volatile int d = 0; d < 12000000; d++);
}




void clear_screen_g(uint8_t color) {
    uint8_t attr = VGA_COLOR(COLOR_WHITE, color);
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
        vga_mem[i] = (attr << 8) | ' ';
}

void put_char(int x, int y, char c, uint8_t color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    vga_mem[y * SCREEN_WIDTH + x] = (color << 8) | c;
}

void put_string(int x, int y, const char* str, uint8_t color) {
    int cx = x;
    while (*str) {
        put_char(cx++, y, *str++, color);
    }
}

void draw_box(int x, int y, int w, int h, uint8_t color) {
    for (int i = 0; i < w; i++) {
        put_char(x + i, y, '-', color);
        put_char(x + i, y + h - 1, '-', color);
    }
    for (int i = 0; i < h; i++) {
        put_char(x, y + i, '|', color);
        put_char(x + w - 1, y + i, '|', color);
    }
    put_char(x, y, '+', color);
    put_char(x + w - 1, y, '+', color);
    put_char(x, y + h - 1, '+', color);
    put_char(x + w - 1, y + h - 1, '+', color);
}

void draw_button(int x, int y, int w, int h, const char* label, uint8_t color) {
    uint8_t attr = VGA_COLOR(COLOR_WHITE, color);
    draw_box(x, y, w, h, attr);
    int lx = x + 2;
    int ly = y + h / 2;
    put_string(lx, ly, label, attr);
}

void draw_window(int x, int y, int w, int h, const char* title, uint8_t color) {
    uint8_t attr = VGA_COLOR(COLOR_WHITE, color);
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            put_char(x + i, y + j, ' ', attr);
        }
    }
    
    draw_box(x, y, w, h, attr);
    
    if (title) {
        int title_len = 0;
        const char* p = title;
        while (*p++) title_len++;
        
        if (title_len > w - 2) title_len = w - 2;
        
        int title_x = x + (w - title_len) / 2;
        put_string(title_x, y, title, attr);
    }
}
