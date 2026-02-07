#include "vgraphics.h"

// Virtual screen buffer (80x25 characters)
// Each entry: bits 15-8 = color, bits 7-0 = character
uint16_t virtual_buffer[VSCREEN_WIDTH * VSCREEN_HEIGHT];

// Initialize virtual graphics
void vgraphics_init(void) {
    // Clear buffer to black
    vgraphics_clear(VCOLOR_BLACK);
}

// Clear virtual buffer with color
void vgraphics_clear(uint8_t color) {
    uint16_t blank = ' ' | (color << 12);
    for (int i = 0; i < VSCREEN_SIZE; i++) {
        virtual_buffer[i] = blank;
    }
}

// Put character at position in virtual buffer
void vgraphics_put_char(int x, int y, char c, uint8_t color) {
    if (x < 0 || x >= VSCREEN_WIDTH || y < 0 || y >= VSCREEN_HEIGHT) {
        return;
    }
    virtual_buffer[y * VSCREEN_WIDTH + x] = (color << 8) | (uint8_t)c;
}

// Put string at position
void vgraphics_put_string(int x, int y, const char* str, uint8_t color) {
    int i = 0;
    while (str[i] != '\0' && (x + i) < VSCREEN_WIDTH) {
        vgraphics_put_char(x + i, y, str[i], color);
        i++;
    }
}

// Draw box outline
void vgraphics_draw_box(int x, int y, int w, int h, uint8_t color) {
    // Top and bottom borders
    for (int i = 0; i < w; i++) {
        vgraphics_put_char(x + i, y, '-', color);
        vgraphics_put_char(x + i, y + h - 1, '-', color);
    }
    
    // Left and right borders
    for (int i = 0; i < h; i++) {
        vgraphics_put_char(x, y + i, '|', color);
        vgraphics_put_char(x + w - 1, y + i, '|', color);
    }
    
    // Corners
    vgraphics_put_char(x, y, '+', color);
    vgraphics_put_char(x + w - 1, y, '+', color);
    vgraphics_put_char(x, y + h - 1, '+', color);
    vgraphics_put_char(x + w - 1, y + h - 1, '+', color);
}

// Draw filled rectangle
void vgraphics_draw_rect_fill(int x, int y, int w, int h, uint8_t color) {
    for (int row = y; row < y + h && row < VSCREEN_HEIGHT; row++) {
        for (int col = x; col < x + w && col < VSCREEN_WIDTH; col++) {
            vgraphics_put_char(col, row, ' ', color);
        }
    }
}

// Draw window with title
void vgraphics_draw_window(int x, int y, int w, int h, const char* title, uint8_t color) {
    // Draw box
    vgraphics_draw_box(x, y, w, h, color);
    
    // Draw title (centered in top border)
    if (title != NULL) {
        int title_len = 0;
        while (title[title_len] != '\0') title_len++;
        
        int title_x = x + (w - title_len) / 2;
        if (title_x < x + 1) title_x = x + 1;
        
        vgraphics_put_string(title_x, y, title, color);
    }
}

// CRITICAL FUNCTION: Copy virtual buffer to VGA memory (atomic - no flicker!)
void vgraphics_repaint(void) {
    uint16_t* vga = (uint16_t*)0xB8000;
    
    // Fast copy of entire buffer to VGA
    for (int i = 0; i < VSCREEN_SIZE; i++) {
        vga[i] = virtual_buffer[i];
    }
}
