#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define OS_API_ADDR 0x5F0F0

// Screen dimensions
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

// Game bounds
#define GAME_X 2
#define GAME_Y 2
#define GAME_WIDTH 76
#define GAME_HEIGHT 21

// Colors
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

// FAT16 directory entry (needed for OS API)
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

// Direction enum
typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

// Snake entity
#define SNAKE_MAX_LENGTH 100

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point body[SNAKE_MAX_LENGTH];  // Snake body segments
    int length;                     // Current length
    Direction facing;              // Which direction snake is facing
    uint32_t head_x;                    // Head X coordinate
    uint32_t head_y;                    // Head Y coordinate
} Snake;

// Snake colors - bright and shiny
#define SNAKE_HEAD_COLOR VGA_COLOR(COLOR_YELLOW, COLOR_BLACK)    // Bright yellow head
#define SNAKE_BODY_COLOR VGA_COLOR(COLOR_LGREEN, COLOR_BLACK)    // Light green body

// Food (Apple)
typedef struct {
    int x;
    int y;
    bool active;
} Food;

#define FOOD_COLOR VGA_COLOR(COLOR_RED, COLOR_BLACK)    // Bright red apple

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

// Initialize API
void init_api() {
    os_api = (os_api_t*)OS_API_ADDR;
}

// Global snake and food
Snake snake;
Food food;
int score = 0;
int high_score = 0;  // Cache high score to avoid reloading every frame

// Check if position collides with snake body
bool collide_with_snake(int x, int y) {
    for (int i = 0; i < snake.length; i++) {
        if (snake.body[i].x == x && snake.body[i].y == y) {
            return true;
        }
    }
    return false;
}

// Place food at random position
void place_food() {
    bool valid = false;
    while (!valid) {
        // Random position inside game bounds
        food.x = GAME_X + 1 + (os_api->get_random(GAME_WIDTH - 2));
        food.y = GAME_Y + 1 + (os_api->get_random(GAME_HEIGHT - 2));
        
        // Check not on snake
        valid = !collide_with_snake(food.x, food.y);
    }
    food.active = true;
}

// Draw the food
void draw_food() {
    if (food.active) {
        os_api->vgraphics_put_char(food.x, food.y, '*', FOOD_COLOR);
    }
}

// Check if snake eats food
bool check_eat_food() {
    if (food.active && snake.head_x == food.x && snake.head_y == food.y) {
        return true;
    }
    return false;
}

// Grow snake
void grow_snake() {
    if (snake.length < SNAKE_MAX_LENGTH) {
        snake.length++;
    }
}

// Check if snake collides with itself
bool check_self_collision() {
    for (int i = 1; i < snake.length; i++) {
        if (snake.head_x == snake.body[i].x && snake.head_y == snake.body[i].y) {
            return true;
        }
    }
    return false;
}

// Parse number from string until delimiter
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

// Load high scores from GAMES.TXT
// Format: game1|game2|game3|game4|game5|game6|
// Snake uses position 0 (first game)
void load_all_high_scores(int* scores, int num_games) {
    uint8_t buffer[64];
    
    // Check if file exists
    if (!os_api->fat16_file_exists("GAMES.TXT")) {
        // Create file with default values: |0|0|0|0|0|0|
        const char* default_scores = "|0|0|0|0|0|0|";
        os_api->fat16_create_file("GAMES.TXT", 0);
        os_api->fat16_file_save("GAMES.TXT", (uint8_t*)default_scores, 14);
        
        // Initialize all scores to 0
        for (int i = 0; i < num_games; i++) {
            scores[i] = 0;
        }
        return;
    }
    
    // Load existing file
    int size = os_api->fat16_file_load("GAMES.TXT", buffer);
        // File exists but couldn't load, use zeros
        for (int i = 0; i < num_games; i++) {
            scores[i] = 0;
        }
        return;
    
    // Parse scores
    buffer[size] = '\0';
    int pos = 0;
    for (int i = 0; i < num_games && pos < size; i++) {
        scores[i] = parse_number((char*)buffer, &pos, '|');
    }
}

// Save high scores to GAMES.TXT
void save_all_high_scores(int* scores, int num_games) {
    char buffer[64];
    int pos = 0;
    
    // Build string: |score1|score2|score3|score4|score5|score6|
    for (int i = 0; i < num_games; i++) {
        buffer[pos++] = '|';
        
        // Convert number to string
        char num_str[12];
        os_api->int_to_str(scores[i], num_str);
        
        // Copy number
        int j = 0;
        while (num_str[j]) {
            buffer[pos++] = num_str[j++];
        }
    }
    buffer[pos++] = '|';
    buffer[pos] = '\0';
    
    // Save file
    os_api->fat16_create_file("GAMES.TXT", 0);
    os_api->fat16_file_save("GAMES.TXT", (uint8_t*)buffer, pos);
}

// Save high score for Snake (game 0)
void save_high_score() {
    int scores[6];  // 6 games total
    
    // Load all scores
    load_all_high_scores(scores, 6);
    
    // Update Snake score (position 0) if current score is higher
    if (score > scores[0]) {
        scores[0] = score;
        high_score = score;  // Update cached high score
        save_all_high_scores(scores, 6);
    }
}

// Load high score for Snake (game 0)
int load_high_score() {
    int scores[6];  // 6 games total
    
    // Load all scores
    load_all_high_scores(scores, 6);
    
    // Return Snake score (position 0)
    return scores[0];
}

// Initialize snake at center of game area
void init_snake() {
    // Start at center of game area
    snake.head_x = GAME_X + (GAME_WIDTH / 2);
    snake.head_y = GAME_Y + (GAME_HEIGHT / 2);
    
    // Initial length of 3
    snake.length = 3;
    
    // Start facing right
    snake.facing = DIR_RIGHT;
    
    // Initialize body segments behind head
    for (int i = 0; i < snake.length; i++) {
        snake.body[i].x = snake.head_x - i;
        snake.body[i].y = snake.head_y;
    }
    
    // Place initial food
    place_food();
    
    // Reset score
    score = 0;
}

// Draw the snake
void draw_snake() {
    // Draw body segments first (so head appears on top)
    for (int i = 1; i < snake.length; i++) {
        os_api->vgraphics_put_char(snake.body[i].x, snake.body[i].y, 'o', SNAKE_BODY_COLOR);
    }
    
    // Draw head with bright color
    os_api->vgraphics_put_char(snake.head_x, snake.head_y, 'o', SNAKE_HEAD_COLOR);
}

// Update snake position - returns false if game over
bool update_snake() {
    // Check if eating food before moving
    bool eating = check_eat_food();
    
    // Move body segments (from tail to head)
    for (int i = snake.length - 1; i > 0; i--) {
        snake.body[i] = snake.body[i - 1];
    }
    
    // Move head based on direction
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
    
    // Update body[0] to be the new head position
    snake.body[0].x = snake.head_x;
    snake.body[0].y = snake.head_y;
    
    // Wrap around game boundaries
    if (snake.head_x < GAME_X + 1) snake.head_x = GAME_X + GAME_WIDTH - 2;
    if (snake.head_x >= GAME_X + GAME_WIDTH - 1) snake.head_x = GAME_X + 1;
    if (snake.head_y < GAME_Y + 1) snake.head_y = GAME_Y + GAME_HEIGHT - 2;
    if (snake.head_y >= GAME_Y + GAME_HEIGHT - 1) snake.head_y = GAME_Y + 1;
    
    // Update body[0] after wrapping
    snake.body[0].x = snake.head_x;
    snake.body[0].y = snake.head_y;
    
    // Check for self collision (game over)
    if (check_self_collision()) {
        return false; // Game over
    }
    
    // Handle eating food
    if (eating) {
        grow_snake();
        score += 10;
        place_food();
    }
    
    return true; // Game continues
}

// Handle keyboard input for direction
void handle_input(char c) {
    switch (c) {
        case 'w':
        case 'W':
        case 0x11: // KEY_UP
            if (snake.facing != DIR_DOWN) snake.facing = DIR_UP;
            break;
        case 's':
        case 'S':
        case 0x12: // KEY_DOWN
            if (snake.facing != DIR_UP) snake.facing = DIR_DOWN;
            break;
        case 'a':
        case 'A':
        case 0x13: // KEY_LEFT
            if (snake.facing != DIR_RIGHT) snake.facing = DIR_LEFT;
            break;
        case 'd':
        case 'D':
        case 0x14: // KEY_RIGHT
            if (snake.facing != DIR_LEFT) snake.facing = DIR_RIGHT;
            break;
    }
}

// Draw game bounds (box)
void draw_bounds() {
    // Draw box outline
    os_api->vgraphics_draw_box(GAME_X, GAME_Y, GAME_WIDTH, GAME_HEIGHT, 
                               VGA_COLOR(COLOR_LGREEN, COLOR_BLACK));
    
    // Draw title
    os_api->vgraphics_put_string(GAME_X + (GAME_WIDTH - 10) / 2, GAME_Y, 
                                 " SNAKE ", VGA_COLOR(COLOR_WHITE, COLOR_BLUE));
    
    // Draw instructions
    os_api->vgraphics_put_string(2, 24, "WASD/Arrows: Move | Q: Quit", 
                                 VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));
}

// Clear game area inside bounds
void clear_game_area() {
    for (int y = GAME_Y + 1; y < GAME_Y + GAME_HEIGHT - 1; y++) {
        for (int x = GAME_X + 1; x < GAME_X + GAME_WIDTH - 1; x++) {
            os_api->vgraphics_put_char(x, y, ' ', VGA_COLOR(COLOR_BLACK, COLOR_BLACK));
        }
    }
}

// Draw score and high score
void draw_score() {
    char score_str[20];
    os_api->int_to_str(score, score_str);
    os_api->vgraphics_put_string(2, 23, "Score: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->vgraphics_put_string(9, 23, score_str, VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
    
    // Show high score (use cached value)
    os_api->int_to_str(high_score, score_str);
    os_api->vgraphics_put_string(60, 23, "Hi: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->vgraphics_put_string(64, 23, score_str, VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));
}

// Draw game over screen
void draw_game_over() {
    // Draw box in center
    int box_x = 25;
    int box_y = 8;
    int box_w = 30;
    int box_h = 10;
    
    os_api->vgraphics_draw_box(box_x, box_y, box_w, box_h, VGA_COLOR(COLOR_RED, COLOR_BLACK));
    
    // Title
    os_api->vgraphics_put_string(box_x + 8, box_y + 1, "GAME OVER!", VGA_COLOR(COLOR_LRED, COLOR_BLACK));
    
    // Final score
    char score_str[20];
    os_api->int_to_str(score, score_str);
    os_api->vgraphics_put_string(box_x + 3, box_y + 3, "Final Score: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->vgraphics_put_string(box_x + 16, box_y + 3, score_str, VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
    
    // High score (use cached value loaded at game start)
    os_api->int_to_str(high_score, score_str);
    os_api->vgraphics_put_string(box_x + 3, box_y + 5, "High Score: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->vgraphics_put_string(box_x + 15, box_y + 5, score_str, VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));
    
    // New high score message
    if (score > high_score && score > 0) {
        os_api->vgraphics_put_string(box_x + 6, box_y + 6, "NEW HIGH SCORE!", VGA_COLOR(COLOR_LGREEN, COLOR_BLACK));
    }
    
    // Instructions
    os_api->vgraphics_put_string(box_x + 2, box_y + 8, "R: Restart  Q: Quit", VGA_COLOR(COLOR_LGRAY, COLOR_BLACK));
}

// Main game loop
void game_loop() {
    bool playing = true;
    bool game_over = false;
    
    // Load high score at game start
    high_score = load_high_score();
    
    while (playing) {
        // Initialize graphics
        os_api->vgraphics_init();
        
        // Clear screen
        os_api->vgraphics_clear(COLOR_BLACK);
        
        // Draw game bounds
        draw_bounds();
        
        // Initialize snake
        init_snake();
        
        // Game speed - starts slower (higher number), gets faster (lower number) as snake grows
        // Base speed starts at 35 (slower), progressively gets faster
        int base_speed = 35;
        
        game_over = false;
        
        // Main game loop
        while (!game_over) {
            // Clear game area
            clear_game_area();
            
            // Draw food
            draw_food();
            
            // Draw snake
            draw_snake();
            
            // Draw score
            draw_score();
            
            // Handle input (non-blocking)
            char c = os_api->keyboard_getchar();
            if (c == 'q' || c == 'Q' || c == 0x1B) {
                playing = false;
                break;
            }
            handle_input(c);
            
            // Update snake position - returns false if game over
            if (!update_snake()) {
                game_over = true;
                save_high_score();  // Save high score when game over
            }
            
            // Calculate speed based on snake length - gets faster as you progress
            // Start at base_speed (35), reduce by 1 for every 3 segments eaten
            int current_speed = base_speed - ((snake.length - 3) / 3);
            
            // Minimum speed cap (don't get too fast)
            if (current_speed < 8) current_speed = 8;
            
            os_api->vgraphics_repaint();
            for (volatile int d = 0; d < current_speed * 100000; d++);
        }
        
        // Game over - show screen and wait for input
        if (game_over) {
            draw_game_over();
            
            // Wait for R (restart) or Q (quit)
            while (1) {
                char c = os_api->keyboard_getchar();
                if (c == 'q' || c == 'Q' || c == 0x1B) {
                    playing = false;
                    break;
                }
                if (c == 'r' || c == 'R') {
                    break;  // Restart game
                }
            }
        }
    }
}

// Entry point
void main(void) {
    init_api();
    
    // Clear shell screen first
    os_api->screen_clear_shell();

    
    // Run game
    game_loop();
    
    // Clear shell screen on exit
    os_api->screen_clear_shell();
}
