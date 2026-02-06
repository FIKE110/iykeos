#include <stdint.h>

void keyboard_init();


char keyboard_getchar();

char keyboard_read();

char scancode_to_ascii(uint8_t scancode);