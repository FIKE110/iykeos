#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define OS_API_ADDR 0x5F0F0

// Screen constants
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

// Colors
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

// Game IDs
#define GAME_SNAKE      0
#define GAME_TETRIS     1
#define GAME_PONG       2
#define GAME_BREAKOUT   3
#define GAME_SNAKE2     4
#define GAME_INVADERS   5
#define MAX_GAMES       6

// Game names
const char* game_names[MAX_GAMES] = {
    "Snake",
    "Tetris",
    "Pong",
    "Breakout",
    "Snake II",
    "Space Invaders"
};

// OS API structure
typedef struct {
    void (*print_shell)(const char*);
    void (*print_shellc)(char);
    void (*load_vga)(int);
    void (*save_vga)(int);
    void (*screen_clear_shell)(void);
    char (*keyboard_read)(void);
    void (*move_cursor)(size_t,size_t);
    void (*change_cursor)(uint8_t,uint8_t);
    void (*save_file)(const char*, uint8_t*);
    void (*rng_seed)(void);
    uint32_t (*get_random)(uint32_t);
    void (*busy_delay)(uint32_t);
    void (*set_color)(uint8_t);
    char (*keyboard_getchar)(void);
    void (*beep)(uint32_t, uint32_t);
    void (*set_vga_mode)(uint8_t);
    void (*draw_pixel)(int, int, uint8_t);
    int (*load_file)(const char*, uint8_t*);
    int (*delete_file)(const char*);
    void (*list_files)(void);
    int (*get_file_list)(char* files);
    void (*disable_cursor)(void);
    void (*mouse_init)(void);
    void (*enable_cursor)(void);
    void (*keyboard_init)(void);
    void (*mouse_handler)(int*, int*, int*, int*);
    void (*run)(char*);
    void (*get_rtc_time)(uint8_t*, uint8_t*, uint8_t*);
    void (*get_rtc_date)(uint8_t*, uint8_t*, uint8_t*);
    void (*save_file_size)(char*, uint8_t*, uint32_t);
    size_t (*strlen)(const char*);
    void (*graphics_init)(void);
    int (*fat16_create_file)(const char *filename, uint32_t size);
    int (*fat16_mkdir)(const char *dirname);
    int (*fat16_file_load)(const char *filename, uint8_t *buffer);
    int (*fat16_file_exists)(const char *filename);
    int (*fat16_file_save)(const char *filename, uint8_t *buffer, uint32_t size);
    void (*debug_print)(const char* str);
    void (*debug_print_hex)(uint32_t n);
    void (*debug_putc)(char c);
    int (*strcmp)(const char* s1, const char* s2);
    char* (*strcpy)(char* dest, const char* src);
    char* (*strcat)(char* dest, const char* src);
    int (*memcmp)(const void* s1, const void* s2, size_t n);
    void (*memset)(void* dest, uint8_t val, size_t n);
    void (*memcpy)(void* dest, const void* src, size_t n);
    char (*toupper)(char c);
    void (*hex_to_str)(uint32_t n, char* dest);
    void (*int_to_str)(uint32_t n, char* dest);
    int (*fat16_delete_file)(const char* filename);
    int (*fat16_chdir)(const char* dirname);
    int (*fat16_rmdir)(const char* dirname);
    void (*start_shell)();
    void (*graphics_loading_screen)();
    void (*graphics_clear_screen_g)(uint8_t color);
    void (*graphics_put_char)(int x, int y, char c, uint8_t color);
    void (*graphics_put_string)(int x, int y, const char* str, uint8_t color);
    void (*graphics_draw_box)(int x, int y, int w, int h, uint8_t color);
    void (*graphics_draw_button)(int x, int y, int w, int h, const char* label, uint8_t color);
    void (*graphics_draw_window)(int x, int y, int w, int h, const char* title, uint8_t color);
    void (*graphics_put_pixel)(int x, int y, uint8_t color);
} os_api_t;

static os_api_t* os_api;
static uint16_t* vga_buffer = (uint16_t*)0xB8000;

// Current selection
int selected_game = 0;

// Initialize API
void init_api() {
    os_api = (os_api_t*)OS_API_ADDR;
}

// Clear screen with color
void clear_screen(uint8_t color) {
    uint16_t blank = ' ' | (color << 12);
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        vga_buffer[i] = blank;
    }
}

// Put character at position
void put_char(int x, int y, char c, uint8_t color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    vga_buffer[y * SCREEN_WIDTH + x] = (color << 8) | c;
}

// Put string at position
void put_string(int x, int y, const char* str, uint8_t color) {
    int i = 0;
    while (str[i] && x + i < SCREEN_WIDTH) {
        put_char(x + i, y, str[i], color);
        i++;
    }
}

// Draw box
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

// Draw the game menu
void draw_menu() {
    os_api->graphics_init();  // Initialize graphics mode
    clear_screen(COLOR_BLACK);
    
    // Title
    const char* title = "GAME MENU";
    put_string(35, 3, title, VGA_COLOR(COLOR_LIGHT_GREEN, COLOR_BLACK));
    
    // Decorative line
    for (int i = 20; i < 60; i++) {
        put_char(i, 4, '=', VGA_COLOR(COLOR_GREEN, COLOR_BLACK));
    }
    
    // Draw box around menu
    draw_box(15, 6, 50, 15, VGA_COLOR(COLOR_LIGHT_GRAY, COLOR_BLACK));
    
    // Menu header
    put_string(20, 7, "Select a game:", VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
    
    // Draw game list
    int start_y = 9;
    for (int i = 0; i < MAX_GAMES; i++) {
        int y = start_y + i * 2;
        
        // Draw selection arrow
        if (i == selected_game) {
            put_string(20, y, ">", VGA_COLOR(COLOR_LIGHT_GREEN, COLOR_BLACK));
            put_string(22, y, game_names[i], VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
        } else {
            put_string(20, y, " ", VGA_COLOR(COLOR_BLACK, COLOR_BLACK));
            put_string(22, y, game_names[i], VGA_COLOR(COLOR_LIGHT_GRAY, COLOR_BLACK));
        }
    }
    
    // Instructions at bottom
    put_string(20, 22, "Controls: UP/DOWN to navigate, ENTER to select, Q to quit", 
               VGA_COLOR(COLOR_LIGHT_CYAN, COLOR_BLACK));
}

// Handle menu input
// Returns: -1 to quit, -2 to redraw, 0+ to select game
int handle_menu_input() {
    char c = os_api->keyboard_read();
    
    switch (c) {
        case 0x11: // KEY_UP
            if (selected_game > 0) {
                selected_game--;
            }
            return -2; // Redraw
            
        case 0x12: // KEY_DOWN
            if (selected_game < MAX_GAMES - 1) {
                selected_game++;
            }
            return -2; // Redraw
            
        case '\n': // ENTER
        case '\r':
            return selected_game;
            
        case 'q':
        case 'Q':
        case 0x1B: // ESC
            return -1; // Quit
    }
    
    return -2; // No action, just redraw
}


// ============================================================================
// MAIN GAME MENU
// ============================================================================

void show_coming_soon(int game_id) {
    os_api->graphics_init();  // Initialize graphics mode
    clear_screen(COLOR_BLACK);
    
    put_string(30, 10, "COMING SOON!", VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
    put_string(25, 12, game_names[game_id], VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    put_string(20, 14, "This game is not yet implemented.", VGA_COLOR(COLOR_LIGHT_GRAY, COLOR_BLACK));
    put_string(20, 20, "Press any key to return to menu...", VGA_COLOR(COLOR_LIGHT_CYAN, COLOR_BLACK));
    
    os_api->keyboard_read();
}

int main_game_menu() {
    selected_game = 0;
    
    while (1) {
        draw_menu();
        
        int result = handle_menu_input();
        
        if (result == -1) {
            // Quit
            return 0;
        } else if (result >= 0) {
            // Game selected
            int game_result = 0;
            
            switch (result) {
                case GAME_SNAKE:
                    os_api->run("SNAKE.BIN");
                    break;
                
                case GAME_TETRIS:
                    os_api->run("TETRIS.BIN");
                    break;
                case GAME_PONG:
                    os_api->run("PONG.BIN");
                    break;
                case GAME_BREAKOUT:
                    os_api->run("BREAKOUT.BIN");
                    break;
                case GAME_SNAKE2:
                    os_api->run("SNAKE2.BIN");
                    break;
                case GAME_INVADERS:
                    os_api->run("SPACEINV.BIN");
                    // show_coming_soon(result);
                    break;
            }
            
            if (game_result == -1) {
                return 0; // Quit entirely
            }
            // Otherwise return to menu
        }
        // result == -2 means just redraw
    }
}

// Entry point
void main(void) {
    init_api();
    os_api->disable_cursor();
    os_api->screen_clear_shell();
    
    main_game_menu();
    
    os_api->screen_clear_shell();

}
