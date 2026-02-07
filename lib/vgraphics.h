#ifndef VGRAPHICS_H
#define VGRAPHICS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Virtual screen dimensions (match VGA text mode)
#define VSCREEN_WIDTH 80
#define VSCREEN_HEIGHT 25
#define VSCREEN_SIZE (VSCREEN_WIDTH * VSCREEN_HEIGHT)

// Virtual buffer (declared in .c file)
extern uint16_t virtual_buffer[VSCREEN_SIZE];

// Color macro
#define VGA_COLOR(fg, bg) ((bg << 4) | (fg))

// Color constants
#define VCOLOR_BLACK      0x0
#define VCOLOR_BLUE       0x1
#define VCOLOR_GREEN      0x2
#define VCOLOR_CYAN       0x3
#define VCOLOR_RED        0x4
#define VCOLOR_MAGENTA    0x5
#define VCOLOR_BROWN      0x6
#define VCOLOR_LGRAY      0x7
#define VCOLOR_DGRAY      0x8
#define VCOLOR_LBLUE      0x9
#define VCOLOR_LGREEN     0xA
#define VCOLOR_LCYAN      0xB
#define VCOLOR_LRED       0xC
#define VCOLOR_LMAGENTA   0xD
#define VCOLOR_YELLOW     0xE
#define VCOLOR_WHITE      0xF

// Initialization and buffer management
void vgraphics_init(void);
void vgraphics_clear(uint8_t color);
void vgraphics_repaint(void);  // Copy virtual buffer to VGA (atomic)

// Drawing functions (same as graphics.c but buffered)
void vgraphics_put_char(int x, int y, char c, uint8_t color);
void vgraphics_put_string(int x, int y, const char* str, uint8_t color);
void vgraphics_draw_box(int x, int y, int w, int h, uint8_t color);
void vgraphics_draw_window(int x, int y, int w, int h, const char* title, uint8_t color);
void vgraphics_draw_rect_fill(int x, int y, int w, int h, uint8_t color);

#endif // VGRAPHICS_H
