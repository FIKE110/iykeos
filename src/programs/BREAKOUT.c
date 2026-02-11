#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define OS_API_ADDR 0x5F0F0

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

#define GAME_X 2
#define GAME_Y 2
#define GAME_WIDTH 76
#define GAME_HEIGHT 22

#define PADDLE_WIDTH 8
#define PADDLE_Y 20

#define BALL_CHAR 'O'

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

#define KEY_ESC     0x1B

#define BRICK_ROWS 5
#define BRICK_COLS 10
#define BRICK_WIDTH 6
#define BRICK_HEIGHT 1

#define GAME_ID 3

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
    void (*vgraphics_draw_window)(int x, int y, int w, int h, const char* title, uint8_t color);
    void (*vgraphics_draw_rect_fill)(int x, int y, int w, int h, uint8_t color);
    int (*run_with_status)(char* filename,uint32_t address);
    uint32_t (*fat16_file_size)(char* filename);
    int (*str_to_int)(const char *s);

} os_api_t;


os_api_t* os_api;

typedef struct {
    int x, y;
    int dx, dy;
    bool active;
} Ball;

typedef struct {
    int x;
    int width;
} Paddle;

typedef struct {
    int x, y;
    bool active;
    uint8_t color;
} Brick;

Ball ball = {0};
Paddle paddle = {0};
Brick bricks[BRICK_ROWS][BRICK_COLS];
int score = 0;
int high_score = 0;
int lives = 3;
bool game_over_flag = false;
bool victory_flag = false;
int ball_speed = 25;
int bricks_broken = 0;

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

void clear_game_area() {
    for (int y = GAME_Y + 1; y < GAME_Y + GAME_HEIGHT - 1; y++) {
        for (int x = GAME_X + 1; x < GAME_X + GAME_WIDTH - 1; x++) {
            os_api->vgraphics_put_char(x, y, ' ', VGA_COLOR(COLOR_BLACK, COLOR_BLACK));
        }
    }
}

void draw_bounds() {
    os_api->vgraphics_draw_box(GAME_X, GAME_Y, GAME_WIDTH, GAME_HEIGHT, 
                               VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));
    
    os_api->vgraphics_put_string(GAME_X + (GAME_WIDTH - 10) / 2, GAME_Y, 
                                 " BREAKOUT ", VGA_COLOR(COLOR_WHITE, COLOR_BLUE));
    
    os_api->vgraphics_put_string(2, 24, "<-/-> or A/D: Move | Q: Quit", 
                                 VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));
}

void draw_score() {
    char score_str[20];
    os_api->int_to_str(score, score_str);
    os_api->vgraphics_put_string(2, 23, "Score: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->vgraphics_put_string(9, 23, score_str, VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
    
    os_api->int_to_str(high_score, score_str);
    os_api->vgraphics_put_string(60, 23, "Hi: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->vgraphics_put_string(64, 23, score_str, VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));
    
    os_api->int_to_str(lives, score_str);
    os_api->vgraphics_put_string(35, 23, "Lives: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->vgraphics_put_string(42, 23, score_str, VGA_COLOR(COLOR_RED, COLOR_BLACK));
}

void draw_paddle() {
    uint8_t paddle_color = VGA_COLOR(COLOR_LCYAN, COLOR_BLACK);
    
    for (int i = 0; i < paddle.width; i++) {
        os_api->vgraphics_put_char(paddle.x + i, PADDLE_Y, '=', paddle_color);
    }
}

void draw_ball() {
    if (ball.active) {
        os_api->vgraphics_put_char(ball.x, ball.y, BALL_CHAR, VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    }
}

void init_bricks() {
    int brick_start_x = GAME_X + (GAME_WIDTH - (BRICK_COLS * BRICK_WIDTH)) / 2;
    int brick_start_y = GAME_Y + 2;
    
    uint8_t brick_colors[BRICK_ROWS] = {
        VGA_COLOR(COLOR_RED, COLOR_BLACK),
        VGA_COLOR(COLOR_LRED, COLOR_BLACK),
        VGA_COLOR(COLOR_YELLOW, COLOR_BLACK),
        VGA_COLOR(COLOR_GREEN, COLOR_BLACK),
        VGA_COLOR(COLOR_LBLUE, COLOR_BLACK)
    };
    
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            bricks[row][col].x = brick_start_x + col * BRICK_WIDTH;
            bricks[row][col].y = brick_start_y + row * (BRICK_HEIGHT + 1);
            bricks[row][col].active = true;
            bricks[row][col].color = brick_colors[row];
        }
    }
}

void draw_bricks() {
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            if (bricks[row][col].active) {
                for (int i = 0; i < BRICK_WIDTH; i++) {
                    os_api->vgraphics_put_char(
                        bricks[row][col].x + i, 
                        bricks[row][col].y, 
                        '#', 
                        bricks[row][col].color
                    );
                }
            }
        }
    }
}

void init_game() {
    paddle.x = GAME_X + GAME_WIDTH / 2 - PADDLE_WIDTH / 2;
    paddle.width = PADDLE_WIDTH;
    
    ball.x = paddle.x + PADDLE_WIDTH / 2;
    ball.y = PADDLE_Y - 1;
    ball.dx = 0;
    ball.dy = 0;
    ball.active = false;
    
    score = 0;
    lives = 3;
    game_over_flag = false;
    victory_flag = false;
    ball_speed = 25;
    bricks_broken = 0;
    
    high_score = load_high_score();
    
    init_bricks();
    
    os_api->vgraphics_init();
    os_api->vgraphics_clear(COLOR_BLACK);
    draw_bounds();
    draw_bricks();
    draw_paddle();
    draw_ball();
    draw_score();
}

void launch_ball() {
    if (!ball.active) {
        ball.active = true;
        ball.dx = 1;
        ball.dy = -1;
        os_api->beep(660, 20);
    }
}

void reset_ball() {
    ball.active = false;
    ball.x = paddle.x + PADDLE_WIDTH / 2;
    ball.y = PADDLE_Y - 1;
    ball.dx = 0;
    ball.dy = 0;
}

void move_paddle(int direction) {
    paddle.x += direction * 2;
    
    if (paddle.x < GAME_X + 1) {
        paddle.x = GAME_X + 1;
    }
    if (paddle.x + paddle.width > GAME_X + GAME_WIDTH - 1) {
        paddle.x = GAME_X + GAME_WIDTH - 1 - paddle.width;
    }
    
    if (!ball.active) {
        ball.x = paddle.x + PADDLE_WIDTH / 2;
    }
}

bool check_brick_collision() {
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            if (bricks[row][col].active) {
                int bx = bricks[row][col].x;
                int by = bricks[row][col].y;
                
                if (ball.x >= bx && ball.x < bx + BRICK_WIDTH &&
                    ball.y >= by && ball.y < by + BRICK_HEIGHT) {
                    
                    bricks[row][col].active = false;
                    ball.dy = -ball.dy;
                    
                    bricks_broken++;
                    if (ball_speed > 8 && bricks_broken % 5 == 0) {
                        ball_speed--;
                    }
                    
                    int random_bounce = os_api->get_random(100);
                    if (random_bounce < 20) {
                        ball.dx = -ball.dx;
                    } else if (random_bounce < 35) {
                        ball.dx = (ball.dx > 0) ? 2 : -2;
                    } else if (random_bounce < 50) {
                        ball.dx = (ball.dx > 0) ? 1 : -1;
                    }
                    
                    score += 10 * (BRICK_ROWS - row);
                    
                    os_api->beep(880 + (row * 100), 15);
                    
                    return true;
                }
            }
        }
    }
    return false;
}

bool check_victory() {
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            if (bricks[row][col].active) {
                return false;
            }
        }
    }
    return true;
}

void update_ball() {
    if (!ball.active) return;
    
    ball.x += ball.dx;
    ball.y += ball.dy;
    
    if (ball.x <= GAME_X + 1 || ball.x >= GAME_X + GAME_WIDTH - 2) {
        ball.dx = -ball.dx;
        int random_angle = os_api->get_random(100);
        if (random_angle < 30) {
            ball.dx = (ball.dx > 0) ? 2 : -2;
        } else if (random_angle < 60) {
            ball.dx = (ball.dx > 0) ? 1 : -1;
        }
        ball.x += ball.dx;
        os_api->beep(440, 5);
    }
    
    if (ball.y <= GAME_Y + 1) {
        ball.dy = -ball.dy;
        int random_bounce = os_api->get_random(100);
        if (random_bounce < 25) {
            ball.dx = -ball.dx;
        }
        ball.y += ball.dy;
        os_api->beep(440, 5);
    }
    
    if (ball.y == PADDLE_Y && ball.x >= paddle.x && ball.x < paddle.x + paddle.width) {
        ball.dy = -ball.dy;
        ball.y += ball.dy;
        
        int hit_pos = ball.x - paddle.x;
        if (hit_pos < paddle.width / 3) {
            ball.dx = -1;
        } else if (hit_pos > 2 * paddle.width / 3) {
            ball.dx = 1;
        } else {
            ball.dx = (ball.dx > 0) ? 1 : -1;
        }
        
        os_api->beep(660, 10);
        score += 1;
    }
    
    check_brick_collision();
    
    if (ball.y >= GAME_Y + GAME_HEIGHT - 1) {
        lives--;
        
        if (lives <= 0) {
            game_over_flag = true;
        } else {
            reset_ball();
            os_api->beep(220, 300);
        }
    }
}

void draw_game_over() {
    int box_x = 25;
    int box_y = 8;
    int box_w = 30;
    int box_h = 10;
    
    os_api->vgraphics_draw_box(box_x, box_y, box_w, box_h, VGA_COLOR(COLOR_RED, COLOR_BLACK));
    
    if (victory_flag) {
        os_api->vgraphics_put_string(box_x + 8, box_y + 1, "YOU WIN!", VGA_COLOR(COLOR_LGREEN, COLOR_BLACK));
    } else {
        os_api->vgraphics_put_string(box_x + 8, box_y + 1, "GAME OVER!", VGA_COLOR(COLOR_LRED, COLOR_BLACK));
    }
    
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

void show_title_screen() {
    os_api->vgraphics_init();
    os_api->vgraphics_clear(COLOR_BLACK);
    
    const char* title[] = {
        " ____       _      _             _   ",
        "|  _ \\     | |    | |           | |  ",
        "| |_) | ___| | ___| |_ ___  _ __| |_ ",
        "|  _ < / _ \\ |/ _ \\ __/ _ \\| '__| __|",
        "| |_) |  __/ |  __/ || (_) | |  | |_ ",
        "|____/ \\___|_|\\___|\\__\\___/|_|   \\__|"
    };
    
    uint8_t title_color = VGA_COLOR(COLOR_LCYAN, COLOR_BLACK);
    int title_y = 5;
    
    for (int i = 0; i < 6; i++) {
        int len = 0;
        while (title[i][len]) len++;
        int x = (SCREEN_WIDTH - len) / 2;
        os_api->vgraphics_put_string(x, title_y + i, title[i], title_color);
    }
    
    const char* instructions[] = {
        "",
        "Break all the bricks to win!",
        "Ball gets faster as you play!",
        "",
        "CONTROLS:",
        "<- / ->  - Move paddle",
        "A / D    - Move paddle (alt)",
        "SPACE    - Launch ball",
        "Q        - Quit game",
        "",
        "Press SPACE to start..."
    };

    
    
    uint8_t text_color = VGA_COLOR(COLOR_WHITE, COLOR_BLACK);
    int inst_y = 13;
    
    for (int i = 0; i < 9; i++) {
        int len = 0;
        while (instructions[i][len]) len++;
        int x = (SCREEN_WIDTH - len) / 2;
        os_api->vgraphics_put_string(x, inst_y + i, instructions[i], text_color);
    }


    os_api->vgraphics_repaint();

    
    
    
    while (1) {
        char key = os_api->keyboard_read();
        if (key != 0) {
            if (key == ' ') {
                return;
            } else if (key == 'q' || key == 'Q' || key == 0x1B) {
                os_api->vgraphics_clear(COLOR_BLACK);
                return;
            }
        }
    }

    
}

void show_countdown() {
    int center_x = GAME_X + GAME_WIDTH / 2;
    int center_y = GAME_Y + GAME_HEIGHT / 2;
    
    const char* count[] = {"3", "2", "1", "GO!"};
    uint8_t colors[] = {
        VGA_COLOR(COLOR_RED, COLOR_BLACK),
        VGA_COLOR(COLOR_YELLOW, COLOR_BLACK),
        VGA_COLOR(COLOR_GREEN, COLOR_BLACK),
        VGA_COLOR(COLOR_LCYAN, COLOR_BLACK)
    };
    
    for (int i = 0; i < 4; i++) {
        clear_game_area();
        draw_bricks();
        draw_paddle();
        draw_ball();
        draw_score();
        os_api->vgraphics_repaint();
        
        os_api->vgraphics_put_string(center_x - 1, center_y, count[i], colors[i]);
        os_api->vgraphics_repaint();
        
        os_api->busy_delay(5000);
    }
}

void game_loop() {
    bool playing = true;
    
    show_title_screen();
    
    
    while (playing) {
        init_game();
        
        show_countdown();
        launch_ball();
        
        int frame_count = 0;
        
        while (!game_over_flag && !victory_flag) {
            clear_game_area();
            draw_bricks();
            draw_paddle();
            draw_ball();
            draw_score();
            
            char key = os_api->keyboard_getchar();
            if (key != 0) {
                if (key == 'q' || key == 'Q' || key == 0x1B) {
                    playing = false;
                    break;
                } else if (key == 'a' || key == 'A' || key == 0x13) {
                    move_paddle(-1);
                } else if (key == 'd' || key == 'D' || key == 0x14) {
                    move_paddle(1);
                } else if (key == ' ') {
                    launch_ball();
                }
            }
            
            frame_count++;
            if (frame_count >= ball_speed) {
                frame_count = 0;
                
                update_ball();
                
                if (check_victory()) {
                    victory_flag = true;
                    game_over_flag = true;
                    break;
                }
            }
            
            os_api->vgraphics_repaint();
            os_api->busy_delay(500);
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
    os_api = (os_api_t*)OS_API_ADDR;
    
    os_api->disable_cursor();
    game_loop();
    os_api->enable_cursor();
    
    os_api->vgraphics_clear(COLOR_BLACK);
    
    return 0;
}
