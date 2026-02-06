#include <stddef.h>
#include <stdint.h>

#define HEAP_START 0x500000
#define HEAP_SIZE  0x100000 


static uint8_t* heap_top = (uint8_t*)HEAP_START;
static uint8_t* heap_end = (uint8_t*)(HEAP_START + HEAP_SIZE);

void* simple_malloc(size_t size) {
    size = (size + 15) & ~15;

    if (heap_top + size > heap_end) {
        return NULL;
    }

    void* block = heap_top;
    heap_top += size;
    return block;
}
