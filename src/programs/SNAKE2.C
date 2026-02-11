#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define OS_API_ADDR 0x5F0F0

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

#define GAME_X 2
#define GAME_Y 2
#define GAME_WIDTH 76
#define GAME_HEIGHT 21

#define COLOR_BLACK      0x0
#define COLOR_BLUE       0x1
#define COLOR_GREEN      0x2
#define COLOR_CYAN       0x3
#define COLOR_RED        0x4
#define COLOR_MAGENTA    0x5
#define COLOR_BROWN      0x6
#define COLOR_LGRAY      0x7
#define COLOR_DGRAY      0x8
#define COLOR_LBLUE      0x9
#define COLOR_LGREEN     0xA
#define COLOR_LCYAN      0xB
#define COLOR_LRED       0xC
#define COLOR_LMAGENTA   0xD
#define COLOR_YELLOW     0xE
#define COLOR_WHITE      0xF

#define VGA_COLOR(fg, bg) ((bg << 4) | (fg))

#define GAME_ID 4

typedef struct {
    char name[8];
    char ext[3];
    uint8_t attr;
    uint8_t reserved[10];
    uint16_t time;
    uint16_t date;
    uint16_t first_cluster;
    uint32_t size;
} __attribute__((packed)) fat16_dir_entry;

typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

#define SNAKE_MAX_LENGTH 100

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point body[SNAKE_MAX_LENGTH];
    int length;
    Direction facing;
    uint32_t head_x;
    uint32_t head_y;
} Snake;

#define SNAKE_HEAD_COLOR VGA_COLOR(COLOR_YELLOW, COLOR_BLACK)
#define SNAKE_BODY_COLOR VGA_COLOR(COLOR_LGREEN, COLOR_BLACK)

typedef struct {
    int x;
    int y;
    bool active;
} Food;

#define FOOD_COLOR VGA_COLOR(COLOR_RED, COLOR_BLACK)

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
    int (*fat16_list_root)(fat16_dir_entry* entries, int max_entries);
    void (*start_shell)();
    void (*graphics_loading_screen)();
    void (*graphics_clear_screen_g)(uint8_t color);
    void (*graphics_put_char)(int x, int y, char c, uint8_t color);
    void (*graphics_put_string)(int x, int y, const char* str, uint8_t color);
    void (*graphics_draw_box)(int x, int y, int w, int h, uint8_t color);
    void (*graphics_draw_button)(int x, int y, int w, int h, const char* label, uint8_t color);
    void (*graphics_draw_window)(int x, int y, int w, int h, const char* title, uint8_t color);
    void (*graphics_put_pixel)(int x, int y, uint8_t color);
    void (*vgraphics_init)(void);
    void (*vgraphics_clear)(uint8_t color);
    void (*vgraphics_repaint)(void);
    void (*vgraphics_put_char)(int x, int y, char c, uint8_t color);
    void (*vgraphics_put_string)(int x, int y, const char* str, uint8_t color);
    void (*vgraphics_draw_box)(int x, int y, int w, int h, uint8_t color);
} os_api_t;

os_api_t* os_api;
Snake snake;
Food food;
int score = 0;
int high_score = 0;

void init_api() {
    os_api = (os_api_t*)OS_API_ADDR;
}

int parse_number(const char* str, int* pos, char delimiter) {
    int num = 0;
    // Skip any leading delimiters first
    while (str[*pos] == delimiter) {
        (*pos)++;
    }
    // Now parse the number
    while (str[*pos] >= '0' && str[*pos] <= '9') {
        num = num * 10 + (str[*pos] - '0');
        (*pos)++;
    }
    // Skip trailing delimiter
    if (str[*pos] == delimiter) {
        (*pos)++;
    }
    return num;
}

void load_all_high_scores(int* scores, int num_games) {
    uint8_t buffer[64];
    
    if (!os_api->fat16_file_exists("GAMES.TXT")) {
        const char* default_scores = "|0|0|0|0|0|0|";
        os_api->fat16_create_file("GAMES.TXT", 0);
        os_api->fat16_file_save("GAMES.TXT", (uint8_t*)default_scores, 14);
        
        for (int i = 0; i < num_games; i++) {
            scores[i] = 0;
        }
        return;
    }
    
    int size = os_api->fat16_file_load("GAMES.TXT", buffer);
    if (size <= 0) {
        for (int i = 0; i < num_games; i++) {
            scores[i] = 0;
        }
        return;
    }
    
    buffer[size] = '\0';
    int pos = 0;
    for (int i = 0; i < num_games && pos < size; i++) {
        scores[i] = parse_number((char*)buffer, &pos, '|');
    }
}

void save_all_high_scores(int* scores, int num_games) {
    char buffer[64];
    int pos = 0;
    
    for (int i = 0; i < num_games; i++) {
        buffer[pos++] = '|';
        
        char num_str[12];
        os_api->int_to_str(scores[i], num_str);
        
        int j = 0;
        while (num_str[j]) {
            buffer[pos++] = num_str[j++];
        }
    }
    buffer[pos++] = '|';
    buffer[pos] = '\0';
    
    os_api->fat16_create_file("GAMES.TXT", 0);
    os_api->fat16_file_save("GAMES.TXT", (uint8_t*)buffer, pos);
}

void save_high_score() {
    int scores[6];
    
    load_all_high_scores(scores, 6);
    
    if (score > scores[GAME_ID]) {
        scores[GAME_ID] = score;
        high_score = score;
        save_all_high_scores(scores, 6);
    }
}

int load_high_score() {
    int scores[6];
    
    load_all_high_scores(scores, 6);
    
    return scores[GAME_ID];
}

bool collide_with_snake(int x, int y) {
    for (int i = 0; i < snake.length; i++) {
        if (snake.body[i].x == x && snake.body[i].y == y) {
            return true;
        }
    }
    return false;
}

void place_food() {
    bool valid = false;
    while (!valid) {
        food.x = GAME_X + 1 + (os_api->get_random(GAME_WIDTH - 2));
        food.y = GAME_Y + 1 + (os_api->get_random(GAME_HEIGHT - 2));
        valid = !collide_with_snake(food.x, food.y);
    }
    food.active = true;
}

void draw_food() {
    if (food.active) {
        os_api->vgraphics_put_char(food.x, food.y, '*', FOOD_COLOR);
    }
}

bool check_eat_food() {
    if (food.active && snake.head_x == food.x && snake.head_y == food.y) {
        return true;
    }
    return false;
}

void grow_snake() {
    if (snake.length < SNAKE_MAX_LENGTH) {
        snake.length++;
    }
}

bool check_self_collision() {
    for (int i = 1; i < snake.length; i++) {
        if (snake.head_x == snake.body[i].x && snake.head_y == snake.body[i].y) {
            return true;
        }
    }
    return false;
}

bool check_wall_collision() {
    if (snake.head_x <= GAME_X || snake.head_x >= GAME_X + GAME_WIDTH - 1 ||
        snake.head_y <= GAME_Y || snake.head_y >= GAME_Y + GAME_HEIGHT - 1) {
        return true;
    }
    return false;
}

bool update_snake() {
    bool eating = check_eat_food();
    
    for (int i = snake.length - 1; i > 0; i--) {
        snake.body[i] = snake.body[i - 1];
    }
    
    switch (snake.facing) {
        case DIR_UP:
            snake.head_y--;
            break;
        case DIR_DOWN:
            snake.head_y++;
            break;
        case DIR_LEFT:
            snake.head_x--;
            break;
        case DIR_RIGHT:
            snake.head_x++;
            break;
    }
    
    snake.body[0].x = snake.head_x;
    snake.body[0].y = snake.head_y;
    
    // Check wall collision - GAME OVER!
    if (check_wall_collision()) {
        return false;
    }
    
    // Check self collision
    if (check_self_collision()) {
        return false;
    }
    
    if (eating) {
        grow_snake();
        score += 10;
        place_food();
        os_api->beep(1000, 50);
    }
    
    return true;
}

void handle_input(char c) {
    switch (c) {
        case 'w':
        case 'W':
        case 0x11:
            if (snake.facing != DIR_DOWN) snake.facing = DIR_UP;
            break;
        case 's':
        case 'S':
        case 0x12:
            if (snake.facing != DIR_UP) snake.facing = DIR_DOWN;
            break;
        case 'a':
        case 'A':
        case 0x13:
            if (snake.facing != DIR_RIGHT) snake.facing = DIR_LEFT;
            break;
        case 'd':
        case 'D':
        case 0x14:
            if (snake.facing != DIR_LEFT) snake.facing = DIR_RIGHT;
            break;
    }
}

void draw_bounds() {
    os_api->vgraphics_draw_box(GAME_X, GAME_Y, GAME_WIDTH, GAME_HEIGHT, 
                               VGA_COLOR(COLOR_LRED, COLOR_BLACK));
    
    os_api->vgraphics_put_string(GAME_X + (GAME_WIDTH - 12) / 2, GAME_Y, 
                                 " SNAKE II ", VGA_COLOR(COLOR_WHITE, COLOR_BLUE));
    
    os_api->vgraphics_put_string(2, 24, "WASD/Arrows: Move | Q: Quit", 
                                 VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));
}

void clear_game_area() {
    for (int y = GAME_Y + 1; y < GAME_Y + GAME_HEIGHT - 1; y++) {
        for (int x = GAME_X + 1; x < GAME_X + GAME_WIDTH - 1; x++) {
            os_api->vgraphics_put_char(x, y, ' ', VGA_COLOR(COLOR_BLACK, COLOR_BLACK));
        }
    }
}

void draw_snake() {
    os_api->vgraphics_put_char(snake.head_x, snake.head_y, '@', SNAKE_HEAD_COLOR);
    for (int i = 1; i < snake.length; i++) {
        os_api->vgraphics_put_char(snake.body[i].x, snake.body[i].y, 'o', SNAKE_BODY_COLOR);
    }
}

void draw_score() {
    char score_str[20];
    os_api->int_to_str(score, score_str);
    os_api->vgraphics_put_string(2, 23, "Score: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->vgraphics_put_string(9, 23, score_str, VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
    
    os_api->int_to_str(high_score, score_str);
    os_api->vgraphics_put_string(60, 23, "Hi: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->vgraphics_put_string(64, 23, score_str, VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));
}

void draw_game_over() {
    int box_x = 25;
    int box_y = 8;
    int box_w = 30;
    int box_h = 10;
    
    os_api->vgraphics_draw_box(box_x, box_y, box_w, box_h, VGA_COLOR(COLOR_RED, COLOR_BLACK));
    
    os_api->vgraphics_put_string(box_x + 8, box_y + 1, "GAME OVER!", VGA_COLOR(COLOR_LRED, COLOR_BLACK));
    
    char score_str[20];
    os_api->int_to_str(score, score_str);
    os_api->vgraphics_put_string(box_x + 3, box_y + 3, "Final Score: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->vgraphics_put_string(box_x + 16, box_y + 3, score_str, VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
    
    os_api->int_to_str(high_score, score_str);
    os_api->vgraphics_put_string(box_x + 3, box_y + 5, "High Score: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->vgraphics_put_string(box_x + 15, box_y + 5, score_str, VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));
    
    if (score > high_score && score > 0) {
        os_api->vgraphics_put_string(box_x + 6, box_y + 6, "NEW HIGH SCORE!", VGA_COLOR(COLOR_LGREEN, COLOR_BLACK));
    }
    
    os_api->vgraphics_put_string(box_x + 2, box_y + 8, "R: Restart  Q: Quit", VGA_COLOR(COLOR_LGRAY, COLOR_BLACK));
}

void init_snake() {
    snake.head_x = GAME_X + (GAME_WIDTH / 2);
    snake.head_y = GAME_Y + (GAME_HEIGHT / 2);
    
    snake.length = 3;
    snake.facing = DIR_RIGHT;
    
    snake.body[0].x = snake.head_x;
    snake.body[0].y = snake.head_y;
    snake.body[1].x = snake.head_x - 1;
    snake.body[1].y = snake.head_y;
    snake.body[2].x = snake.head_x - 2;
    snake.body[2].y = snake.head_y;
}

void game_loop() {
    bool playing = true;
    
    high_score = load_high_score();
    
    while (playing) {
        os_api->vgraphics_init();
        os_api->vgraphics_clear(COLOR_BLACK);
        
        draw_bounds();
        
        init_snake();
        score = 0;
        place_food();
        
        int base_speed = 50;
        bool game_over = false;
        
        while (!game_over) {
            clear_game_area();
            
            draw_food();
            draw_snake();
            draw_score();
            
            char c = os_api->keyboard_getchar();
            if (c == 'q' || c == 'Q' || c == 0x1B) {
                playing = false;
                break;
            }
            handle_input(c);
            
            if (!update_snake()) {
                game_over = true;
                break;
            }
            
            int current_speed = base_speed - (snake.length / 8);
            if (current_speed < 25) current_speed = 25;
            
            os_api->vgraphics_repaint();
            for (volatile int i = 0; i < current_speed * 100000; i++);
        }
        
        if (!playing) break;
        
        save_high_score();
        draw_game_over();
        os_api->vgraphics_repaint();
        
        bool waiting = true;
        while (waiting) {
            char key = os_api->keyboard_getchar();
            if (key != 0) {
                if (key == 'r' || key == 'R') {
                    waiting = false;
                } else if (key == 'q' || key == 'Q' || key == 0x1B) {
                    playing = false;
                    waiting = false;
                }
            }
        }
    }
}

int main(void) {
    init_api();
    
    os_api->disable_cursor();
    game_loop();
    os_api->enable_cursor();
    
    os_api->vgraphics_clear(COLOR_BLACK);
    
    return 0;
}
