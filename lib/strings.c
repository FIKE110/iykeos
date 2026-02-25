#include <stddef.h>
#include <stdint.h>
#include <strings.h>


int strcmp(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        if (*s1 != *s2)
            return (*s1 - *s2); // return difference of first mismatched char
        s1++;
        s2++;
    }
    return *s1 - *s2; // handles different lengths
}


void reverseString(char str[]) {
    int length = strlen(str);
    int start = 0;
    int end = length - 1;
    char temp;

    while (start < end) {
        temp = str[start];
        str[start] = str[end];
        str[end] = temp;

        start++;
        end--;
    }
}

char* strcpy(char* dest, const char* src) {
    char* temp = dest;
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
    return temp;
}

char* strcat(char* dest, const char* src) {
    char* temp = dest;
    while (*dest) {
        dest++;
    }
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
    return temp;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const uint8_t* p1 = (const uint8_t*)s1;
    const uint8_t* p2 = (const uint8_t*)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return (int)p1[i] - (int)p2[i];
        }
    }
    return 0;
}





// Replace memset
void memset(void* dest, uint8_t val, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    for (size_t i = 0; i < n; i++) d[i] = val;
}

// Replace memcpy
void memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = dest;
    const uint8_t* s = src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
}

char toupper(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - 32;
    }
    return c;
}


int str_to_int(const char* s) {
    int result = 0;
    int sign = 1;
    if (*s == '-') {
        sign = -1;
        s++;
    }

    while (*s) {
        if (*s < '0' || *s > '9') break; // stop at non-digit
        result = result * 10 + (*s - '0');
        s++;
    }
    return result * sign;
}

void hex_to_str(uint32_t n, char* dest) {
    if (n == 0) {
        dest[0] = '0';
        dest[1] = '\0';
        return;
    }

    char hex_chars[] = "0123456789ABCDEF";
    int i = 0;
    uint32_t temp = n;

    // Calculate the number of digits
    while (temp > 0) {
        temp /= 16;
        i++;
    }

    dest[i] = '\0';
    i--;

    while (n > 0) {
        dest[i] = hex_chars[n % 16];
        n /= 16;
        i--;
    }
}



size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}
void int_to_str(uint32_t n, char* dest) {
    if (n == 0) {
        dest[0] = '0';
        dest[1] = '\0';
        return;
    }

    int i = 0;
    uint32_t temp = n;

    while (temp > 0) {
        temp /= 10;
        i++;
    }

    dest[i] = '\0';
    i--;

    while (n > 0) {
        dest[i] = (n % 10) + '0';
        n /= 10;
        i--;
    }
}
