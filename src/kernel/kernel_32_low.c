#include <stdint.h>
#include <stddef.h>
#include <file.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define KEY_UP    0x48
#define KEY_DOWN  0x50
#define KEY_ENTER 0x1C
#define SECTOR_SIZE 512

void init();
void hide_cursor();
void draw_bracket(int row);
void draw_vertical_bracket(int col, int size);
void screen_options();
void keyboard_poll(int* selected);
void draw_arrow_select();
void update_arrow(int selected);
void wait_key_release(uint8_t scancode);
uint8_t get_key();
int main();

typedef struct {
    void* (*simple_malloc)(size_t);
    void (*clear_screen)(uint8_t);
    void (*clear_screen_white)(uint8_t);
    void (*block)();
    char (*keyboard_getchar)();
    char (*keyboard_read)();
    void (*screen_clear)();
    void (*screen_putc)(char);
    void (*screen_print)(char*);
} kernel_api_t;

#define KERNEL_API_ADDR 0x20000

void clear_screen(uint8_t color) {
    uint16_t* vga = (uint16_t*)0xB8000;
    uint16_t blank = (' ' | (color << 8));
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = blank;
    }
}

void block(){
    while(1){
         __asm__ __volatile__("hlt");
    }
}

void clear_screen_white(uint8_t color) {
    uint16_t* vga = (uint16_t*)0xB8000;
    uint16_t blank = (' ' | ((0xF << 4 | 0x0) << 8));
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = blank;
    }
}

void kernel_main() {
    init();
    int selected=0;
    int* ptr_selected=&selected;
    hide_cursor();
    char* name = "WELCOME TO FORTUNE KERNEL , OPTIONS TO MOVE FORMARD ARE BELOW";
    uint8_t color = 0x0F;
    clear_screen_white(color);
    uint16_t* vga = (uint16_t*)0xB8000;
    draw_bracket(0);
    draw_vertical_bracket(0,VGA_HEIGHT);
    for (int i = i; name[i] != '\0'; i++) {
        vga[i+86] = (name[i] | ((0xF << 4 | 0x0) << 8));
    }
    draw_bracket(2);
    draw_vertical_bracket(VGA_WIDTH-1,VGA_HEIGHT);
    draw_bracket(VGA_HEIGHT-1);
    screen_options();
    while(1) {
        keyboard_poll(ptr_selected);
    }
    while (1) __asm__ __volatile__("hlt");
}

void draw_bracket(int row){
    uint16_t* vga = (uint16_t*)0xB8000;
    uint8_t color = 0x07;
    for (int i = 0; i<VGA_WIDTH; i++) {
        vga[i+(row*80)]='=' | ((0xF << 4 | 0x0) << 8);
    }
}

void draw_vertical_bracket(int col,int size){
    uint16_t* vga = (uint16_t*)0xB8000;
    uint8_t color = 0x07;
    for (int i = 0; i<size; i++) {
        vga[col+(80*i)]='=' | ((0xF << 4 | 0x0) << 8);
    }
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void hide_cursor() {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void screen_options(){
    uint16_t* vga = (uint16_t*) 0xB8000;
    char* first="1.  BOOT INTO FORTUNE_OS CLI";
    char* second="2.  BOOT INTO FORTUNE_OS GRAPHICS";
    char* explore="3.  CHECK OUT KERNEL GAMES";
    char* quit="4.  QUIT/EXIT";
    uint8_t color = 0x07;

    char* ptrs[4];
    ptrs[0]=first;
    ptrs[1]=second;
    ptrs[2]=explore;
    ptrs[3]=quit;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; ptrs[i][j] != '\0'; j++) {
            vga[j+(80*7+10)+(80*i*3)] = (ptrs[i][j]  | ((0xF << 4 | 0x0) << 8));
        }
    }

    draw_arrow_select();

}

void draw_arrow_select(){
    uint8_t color = 0x07;
    uint16_t* vga = (uint16_t*) 0xB8000;
    uint16_t arrow = ('>' | ((0xF << 4 | 0x0) << 8));
    vga[(80*7+10)+(80*0*3)-3]=arrow;
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void wait_key_release(uint8_t scancode) {
    while (inb(0x60) == scancode);
}

uint8_t get_key() {
    uint8_t scancode = inb(0x60);
    wait_key_release(scancode);
    return scancode;
}

void update_arrow(int selected) {
    uint16_t* vga = (uint16_t*)0xB8000;
    uint8_t color = 0x07;
    uint16_t arrow = ('>' | ((0xF << 4 | 0x0) << 8));

    for (int i = 0; i < 4; i++) {
        vga[(80*7+10)+(80*i*3)-3] = (' ' | ((0xF << 4 | 0x0) << 8));
    }

    vga[(80*7+10)+(80*(selected)*3)-3] = arrow;
}

 typedef void (*entry_t)();


void keyboard_poll(int* selected) {
    uint8_t scancode = get_key();
    if (scancode == KEY_UP) {
        if (*selected > 0) {
            (*selected)--;
        }
        update_arrow(*selected);
        wait_key_release(KEY_UP);
    } else if (scancode == KEY_DOWN) {
        if (*selected < 3) {
            (*selected)++;
        }
        update_arrow(*selected);
        wait_key_release(KEY_DOWN);
    } else if (scancode == KEY_ENTER) {
        int num=*selected;
        main(num);
    }
}

void init(){
    kernel_api_t* api=(kernel_api_t*) KERNEL_API_ADDR;
    api->clear_screen=&clear_screen;
    api->block=&block;
    api->clear_screen_white=&clear_screen_white;
}
