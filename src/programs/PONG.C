#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Adjustable game speed - higher = slower
int game_speed = 500;

#define OS_API_ADDR 0x5F0F0

// Screen dimensions
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

// Game bounds
#define GAME_X 2
#define GAME_Y 2
#define GAME_WIDTH 60
#define GAME_HEIGHT 22

// Paddle dimensions
#define PADDLE_WIDTH 2
#define PADDLE_HEIGHT 4

// Ball constants
#define BALL_CHAR 'O'
#define BALL_COLOR COLOR_WHITE
#define WIN_SCORE 10

// Paddle positions
#define LEFT_PADDLE_X 4
#define RIGHT_PADDLE_X (GAME_X + GAME_WIDTH - 6)

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

// Scancodes
#define KEY_LEFT    0x4B
#define KEY_RIGHT   0x4D
#define KEY_UP      0x48
#define KEY_DOWN    0x50
#define KEY_SPACE   0x39
#define KEY_Q       0x10
#define KEY_ESC     0x01

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

// Ball structure
typedef struct {
    int x, y;           // FPosition in screen coordinates
    int dx, dy;         // Velocity (-1, 0, or 1 for each axis)
    bool active;        // Is ball in play?
} Ball;

// Game state structure
typedef struct {
    int left_paddle_y;    // Player paddle Y (0 to GAME_HEIGHT - PADDLE_HEIGHT - 2)
    int right_paddle_y;   // AI paddle Y
    int left_score;       // Player score
    int right_score;      // AI score
    int target_y;         // AI target Y position
    bool game_over;
} PongGame;

PongGame game;
Ball ball;
int high_score = 0;

// Initialize API
void init_api() {
    os_api = (os_api_t*)OS_API_ADDR;
}

// Parse number from string until delimiter
int parse_number(const char* str, int* pos, char delimiter) {
    int num = 0;
    while (str[*pos] >= '0' && str[*pos] <= '9') {
        num = num * 10 + (str[*pos] - '0');
        (*pos)++;
    }
    if (str[*pos] == delimiter) {
        (*pos)++;
    }
    return num;
}

// Load high scores from GAMES.TXT
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

// Save high scores to GAMES.TXT
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

// Load Pong high score (position 2)
int load_high_score() {
    int scores[6];
    load_all_high_scores(scores, 6);
    return scores[2];
}

// Save Pong high score
void save_high_score() {
    int scores[6];
    
    load_all_high_scores(scores, 6);
    
    // Calculate total score (left + right scores)
    int total_score = game.left_score + game.right_score;
    
    if (total_score > scores[2]) {
        scores[2] = total_score;
        high_score = total_score;
        save_all_high_scores(scores, 6);
    }
}

// Initialize ball
void init_ball() {
    ball.x = GAME_X + GAME_WIDTH / 2;
    ball.y = GAME_Y + GAME_HEIGHT / 2;
    ball.dx = (os_api->get_random(2) == 0) ? -1 : 1;  // Random left or right
    ball.dy = (os_api->get_random(3) - 1);  // -1, 0, or 1
    ball.active = true;
}

// Reset ball after score
void reset_ball() {
    ball.x = GAME_X + GAME_WIDTH / 2;
    ball.y = GAME_Y + GAME_HEIGHT / 2;
    ball.dx = (game.left_score > game.right_score) ? -1 : 1;  // Serve to loser
    ball.dy = (os_api->get_random(3) - 1);
    ball.active = true;
}

// Initialize game
void init_pong() {
    // Center paddles vertically
    game.left_paddle_y = GAME_Y + (GAME_HEIGHT / 2) - (PADDLE_HEIGHT / 2);
    game.right_paddle_y = game.left_paddle_y;
    game.left_score = 0;
    game.right_score = 0;
    game.target_y = game.right_paddle_y;
    game.game_over = false;
    init_ball();
}

// Draw game frame (green border)
void draw_bounds() {
    os_api->vgraphics_draw_box(GAME_X, GAME_Y, GAME_WIDTH, GAME_HEIGHT, 
                               VGA_COLOR(COLOR_GREEN, COLOR_BLACK));
    os_api->vgraphics_put_string(GAME_X + (GAME_WIDTH - 8) / 2, GAME_Y, 
                                 " PONG ", VGA_COLOR(COLOR_WHITE, COLOR_BLUE));
}

// Draw left paddle (player)
void draw_left_paddle() {
    for (int i = 0; i < PADDLE_HEIGHT; i++) {
        os_api->vgraphics_put_char(LEFT_PADDLE_X, game.left_paddle_y + i, '[', 
                                   VGA_COLOR(COLOR_WHITE, COLOR_WHITE));
        os_api->vgraphics_put_char(LEFT_PADDLE_X + 1, game.left_paddle_y + i, ']', 
                                   VGA_COLOR(COLOR_WHITE, COLOR_WHITE));
    }
}

// Draw right paddle (AI)
void draw_right_paddle() {
    for (int i = 0; i < PADDLE_HEIGHT; i++) {
        os_api->vgraphics_put_char(RIGHT_PADDLE_X, game.right_paddle_y + i, '[', 
                                   VGA_COLOR(COLOR_WHITE, COLOR_WHITE));
        os_api->vgraphics_put_char(RIGHT_PADDLE_X + 1, game.right_paddle_y + i, ']', 
                                   VGA_COLOR(COLOR_WHITE, COLOR_WHITE));
    }
}

// Draw score
void draw_score() {
    char score_str[20];
    
    // Score format: "0 - 0"
    os_api->int_to_str(game.left_score, score_str);
    os_api->vgraphics_put_string(GAME_X + 5, GAME_Y + GAME_HEIGHT - 2, 
                                 "Score: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->vgraphics_put_string(GAME_X + 12, GAME_Y + GAME_HEIGHT - 2, 
                                 score_str, VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
    os_api->vgraphics_put_string(GAME_X + 15, GAME_Y + GAME_HEIGHT - 2, 
                                 " - ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->int_to_str(game.right_score, score_str);
    os_api->vgraphics_put_string(GAME_X + 18, GAME_Y + GAME_HEIGHT - 2, 
                                 score_str, VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
    
    // High score
    int high = load_high_score();
    os_api->int_to_str(high, score_str);
    os_api->vgraphics_put_string(GAME_X + 35, GAME_Y + GAME_HEIGHT - 2, 
                                 "Hi: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->vgraphics_put_string(GAME_X + 39, GAME_Y + GAME_HEIGHT - 2, 
                                 score_str, VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));
}

// Draw controls
void draw_controls() {
    os_api->vgraphics_put_string(2, 24, 
                                 "W/S: You (Left) | AI: Right | Q: Quit", 
                                 VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));
}

// Draw ball
void draw_ball() {
    if (ball.active) {
        os_api->vgraphics_put_char(ball.x, ball.y, BALL_CHAR, 
                                   VGA_COLOR(BALL_COLOR, COLOR_BLACK));
    }
}

// Check win condition
void check_win() {
    if (game.left_score >= WIN_SCORE || game.right_score >= WIN_SCORE) {
        game.game_over = true;
        save_high_score();
    }
}

// Update ball with collision detection
void update_ball() {
    if (!ball.active) return;
    
    // Move ball
    ball.x += ball.dx;
    ball.y += ball.dy;
    
    // Wall collisions (top/bottom)
    if (ball.y <= GAME_Y + 1) {
        ball.y = GAME_Y + 2;
        ball.dy = -ball.dy;
    } else if (ball.y >= GAME_Y + GAME_HEIGHT - 2) {
        ball.y = GAME_Y + GAME_HEIGHT - 3;
        ball.dy = -ball.dy;
    }
    
    // Left paddle collision (player)
    if (ball.x <= LEFT_PADDLE_X + 2 && 
        ball.y >= game.left_paddle_y && 
        ball.y < game.left_paddle_y + PADDLE_HEIGHT) {
        
        ball.x = LEFT_PADDLE_X + 3;
        ball.dx = 1;  // Bounce right
        
        // Calculate angle based on hit position
        int hit_pos = ball.y - game.left_paddle_y;
        ball.dy = (hit_pos - PADDLE_HEIGHT/2) / 2;
        if (ball.dy < -2) ball.dy = -2;
        if (ball.dy > 2) ball.dy = 2;
    }
    
    // Right paddle collision (AI)
    if (ball.x >= RIGHT_PADDLE_X - 1 && 
        ball.y >= game.right_paddle_y && 
        ball.y < game.right_paddle_y + PADDLE_HEIGHT) {
        
        ball.x = RIGHT_PADDLE_X - 2;
        ball.dx = -1;  // Bounce left
        
        // Calculate angle based on hit position
        int hit_pos = ball.y - game.right_paddle_y;
        ball.dy = (hit_pos - PADDLE_HEIGHT/2) / 2;
        if (ball.dy < -2) ball.dy = -2;
        if (ball.dy > 2) ball.dy = 2;
    }
    
    // Scoring
    if (ball.x < GAME_X + 1) {
        // AI scores
        game.right_score++;
        check_win();
        if (!game.game_over) {
            reset_ball();
        }
    } else if (ball.x > GAME_X + GAME_WIDTH - 2) {
        // Player scores
        game.left_score++;
        check_win();
        if (!game.game_over) {
            reset_ball();
        }
    }
}

// Update AI paddle (tracks ball with imperfection)
void update_ai() {
    if (!ball.active) return;
    
    // AI tracks ball but with delay and imperfection
    static int ai_delay = 0;
    static int target_y = 0;
    
    ai_delay++;
    if (ai_delay > 5) {  // Update target every 5 frames
        ai_delay = 0;
        
        // Calculate target with some error (AI not perfect)
        int error = (os_api->get_random(5) - 2);  // -2 to +2 error
        target_y = ball.y + error - PADDLE_HEIGHT/2;
        
        // Clamp target to bounds
        if (target_y < GAME_Y + 1) target_y = GAME_Y + 1;
        if (target_y > GAME_Y + GAME_HEIGHT - PADDLE_HEIGHT - 2) {
            target_y = GAME_Y + GAME_HEIGHT - PADDLE_HEIGHT - 2;
        }
    }
    
    // Move toward target (slower than ball)
    if (game.right_paddle_y < target_y) {
        game.right_paddle_y++;
    } else if (game.right_paddle_y > target_y) {
        game.right_paddle_y--;
    }
}

// Draw game over screen
void draw_game_over() {
    int box_x = GAME_X + 18;
    int box_y = GAME_Y + 8;
    int box_w = 24;
    int box_h = 10;
    
    os_api->vgraphics_draw_box(box_x, box_y, box_w, box_h, 
                               VGA_COLOR(COLOR_RED, COLOR_BLACK));
    
    os_api->vgraphics_put_string(box_x + 6, box_y + 1, 
                                 "GAME OVER", VGA_COLOR(COLOR_LRED, COLOR_BLACK));
    
    // Show winner
    const char* winner = (game.left_score > game.right_score) ? 
                          "YOU WIN!" : "AI WINS!";
    os_api->vgraphics_put_string(box_x + 6, box_y + 3, 
                                 winner, VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
    
    // Scores
    char score_str[20];
    os_api->vgraphics_put_string(box_x + 2, box_y + 5, 
                                 "You: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->int_to_str(game.left_score, score_str);
    os_api->vgraphics_put_string(box_x + 7, box_y + 5, 
                                 score_str, VGA_COLOR(COLOR_LGREEN, COLOR_BLACK));
    
    os_api->vgraphics_put_string(box_x + 12, box_y + 5, 
                                 "AI: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->int_to_str(game.right_score, score_str);
    os_api->vgraphics_put_string(box_x + 16, box_y + 5, 
                                 score_str, VGA_COLOR(COLOR_LRED, COLOR_BLACK));
    
    os_api->vgraphics_put_string(box_x + 3, box_y + 8, 
                                 "R: Restart  Q: Quit", VGA_COLOR(COLOR_LGRAY, COLOR_BLACK));
}

// Clear only the inner play area (not borders)
void clear_play_area() {
    for (int y = GAME_Y + 1; y < GAME_Y + GAME_HEIGHT - 1; y++) {
        for (int x = GAME_X + 1; x < GAME_X + GAME_WIDTH - 1; x++) {
            os_api->vgraphics_put_char(x, y, ' ', VGA_COLOR(COLOR_BLACK, COLOR_BLACK));
        }
    }
}

// Handle keyboard input
void handle_input(char c) {
    switch (c) {
        case 'w':
        case 'W':
        case 0x11: // KEY_UP
            if (game.left_paddle_y > GAME_Y + 1) {
                game.left_paddle_y -= 3;
            }
            break;
        case 's':
        case 'S':
        case 0x12: // KEY_DOWN
            if (game.left_paddle_y < GAME_Y + GAME_HEIGHT - PADDLE_HEIGHT - 2) {
                game.left_paddle_y += 3;
            }
            break;
    }
}

// Main game loop
void game_loop() {
    bool playing = true;
    
    high_score = load_high_score();
    init_pong();
    init_ball();
    
    // Initialize virtual graphics
    os_api->vgraphics_init();
    os_api->vgraphics_clear(COLOR_BLACK);
    
    // Draw static elements ONCE
    draw_bounds();
    draw_controls();
    os_api->vgraphics_repaint();  // Show static elements
    
    while (playing) {
        // Clear the entire screen
        os_api->vgraphics_clear(COLOR_BLACK);
        
        // Handle input
        char c = os_api->keyboard_getchar();
        
        if (game.game_over) {
            if (c == 'q' || c == 'Q' || c == 0x1B) {
                playing = false;
                break;
            } else if (c == 'r' || c == 'R') {
                // Restart game
                game.left_score = 0;
                game.right_score = 0;
                game.left_paddle_y = GAME_Y + (GAME_HEIGHT / 2) - (PADDLE_HEIGHT / 2);
                game.right_paddle_y = game.left_paddle_y;
                game.game_over = false;
                init_ball();
                // Clear virtual buffer and redraw static elements
                os_api->vgraphics_clear(COLOR_BLACK);
                draw_bounds();
                draw_controls();
                os_api->vgraphics_repaint();
            }
        } else {
            if (c == 'q' || c == 'Q' || c == 0x1B) {
                playing = false;
                break;
            }
            handle_input(c);
        }
        
        if (!game.game_over) {
            // Update AI and ball
            update_ai();
            update_ball();
        }
        
        // Draw all elements (borders now cleared each frame)
        draw_bounds();
        draw_controls();
        draw_left_paddle();
        draw_right_paddle();
        draw_ball();
        draw_score();
        
        if (game.game_over) {
            draw_game_over();
        }
        
        // ONE atomic update to screen (no flicker!)
        os_api->vgraphics_repaint();
        
        // Delay
        for (volatile int d = 0; d < game_speed * 10000; d++);
    }
}

// Entry point
void main(void) {
    init_api();
    os_api->screen_clear_shell();
    game_loop();
}
