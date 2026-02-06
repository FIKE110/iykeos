#include <stdint.h>
// #include <math.h>

#define VGA_GRAPHICS 0xA0000
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define KEY_UP    0x48
#define KEY_DOWN  0x50
#define KEY_ENTER 0x1C

#define VGA_COLOR_BLACK         0x00
#define VGA_COLOR_BLUE          0x01
#define VGA_COLOR_GREEN         0x02
#define VGA_COLOR_CYAN          0x03
#define VGA_COLOR_RED           0x04
#define VGA_COLOR_MAGENTA       0x05
#define VGA_COLOR_BROWN         0x06
#define VGA_COLOR_LIGHT_GREY    0x07
#define VGA_COLOR_DARK_GREY     0x08
#define VGA_COLOR_LIGHT_BLUE    0x09
#define VGA_COLOR_LIGHT_GREEN   0x0A
#define VGA_COLOR_LIGHT_CYAN    0x0B
#define VGA_COLOR_LIGHT_RED     0x0C
#define VGA_COLOR_LIGHT_MAGENTA 0x0D
#define VGA_COLOR_YELLOW        0x0E
#define VGA_COLOR_WHITE         0x0F
#define IMAGE_BUFFER 0xB5000

typedef struct {
    int x0,y0,x1,y1;
    uint8_t color;
} Line;

typedef struct {
    int x0,y0;
    uint8_t color;
} Point;

void clear_screen_graphics(uint8_t color) {
    uint8_t* vga = (uint8_t*) VGA_GRAPHICS;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        vga[i] = color;
    }
}

void putpixel(Point* p) {
    uint8_t* vga = (uint8_t*)VGA_GRAPHICS;
    vga[p->y0 * SCREEN_WIDTH + p->x0] = p->color;
}

void draw_line(Point* p0,Point* p1){
    int x0=p0->x0;
    int x1=p1->x0;
    int y0=p0->y0;
    int y1=p1->y0;
   int dx=(x1>x0)? (x1-x0):(x0-x1);
   int sx=(x0<x1) ? 1 :-1;
   int dy=(y1>y0) ? (y0-y1) : (y1-y0);
   int sy=(y0<y1) ? 1:-1;
   int err=dx-dy;
   int e2;

    while(1){
         Point p={x0,y0,VGA_COLOR_BLACK};
        putpixel(&p);
        if(x0==x1 && y0==y1) break;
        e2=2*err;
        if(e2>=dy){err+=dy;x0+=sx;}
        if(e2<=dx){err+=dx;y0+=sy;}
    }
}

void kernel_main(void) {
    Line ln={20,20,200,200,VGA_COLOR_BLACK};
    Point pn={20,20,VGA_COLOR_BLACK};
    Point pn2={50,20,VGA_COLOR_BLACK};
    putpixel(&pn);
    putpixel(&pn2);
    draw_line(&pn,&pn2);
    while (1) __asm__ __volatile__("hlt");
}

void render_image() {
    uint8_t* image = (uint8_t*) IMAGE_BUFFER;
    uint8_t* vga = (uint8_t*)VGA_GRAPHICS;

    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        vga[i] = image[i];
    }
}
