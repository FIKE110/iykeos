#pragma once
#include <stdint.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

#define VGA_MEM 0xB8000

void graphics_init();
void loading_screen();
void clear_screen_g(uint8_t color);
void put_char(int x, int y, char c, uint8_t color);
void put_string(int x, int y, const char* str, uint8_t color);
void draw_box(int x, int y, int w, int h, uint8_t color);
void draw_button(int x, int y, int w, int h, const char* label, uint8_t color);
void draw_window(int x, int y, int w, int h, const char* title, uint8_t color);
