#include <stddef.h>
#include <stdint.h>
#pragma once


int strcmp(const char* s1, const char* s2);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
int memcmp(const void* s1, const void* s2, size_t n);

// Replace memset
void memset(void* dest, uint8_t val, size_t n);
void memcpy(void* dest, const void* src, size_t n);


size_t strlen(const char* str);

char toupper(char c);
void hex_to_str(uint32_t n, char* dest);
void int_to_str(uint32_t n, char* dest);
int str_to_int(const char *s);