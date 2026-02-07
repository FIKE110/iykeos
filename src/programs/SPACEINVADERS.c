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

#define KEY_LEFT    0x4B
#define KEY_RIGHT   0x4D
#define KEY_ESC     0x01

#define GAME_ID 5

#define ALIEN_ROWS 5
#define ALIEN_COLS 10
#define ALIEN_WIDTH 4
#define PLAYER_Y 20
#define BUNKER_Y 17

#define MAX_BULLETS 5
#define MAX_BOMBS 3

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
} os_api_t;

os_api_t* os_api;

typedef struct {
    int x, y;
    bool active;
    uint8_t color;
} Alien;

typedef struct {
    int x, y;
    bool active;
} Bullet;

typedef struct {
    int x, y;
    bool active;
} Bomb;

Alien aliens[ALIEN_ROWS][ALIEN_COLS];
Bullet bullets[MAX_BULLETS];
Bomb bombs[MAX_BOMBS];
int player_x;
int score = 0;
int high_score = 0;
int alien_dir = 1;
int alien_speed = 30;
int alien_timer = 0;
bool game_over = false;
bool victory = false;

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
                               VGA_COLOR(COLOR_GREEN, COLOR_BLACK));
    
    os_api->vgraphics_put_string(GAME_X + (GAME_WIDTH - 18) / 2, GAME_Y, 
                                 " SPACE INVADERS ", VGA_COLOR(COLOR_WHITE, COLOR_BLUE));
    
    os_api->vgraphics_put_string(2, 24, "<-/-> or A/D: Move | SPACE: Shoot | Q: Quit", 
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
}

void draw_player() {
    os_api->vgraphics_put_char(player_x, PLAYER_Y, 'A', VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));
    os_api->vgraphics_put_char(player_x - 1, PLAYER_Y + 1, '<', VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));
    os_api->vgraphics_put_char(player_x, PLAYER_Y + 1, '=', VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));
    os_api->vgraphics_put_char(player_x + 1, PLAYER_Y + 1, '>', VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));
}

void init_aliens() {
    int start_x = GAME_X + 5;
    int start_y = GAME_Y + 2;
    
    uint8_t alien_colors[ALIEN_ROWS] = {
        VGA_COLOR(COLOR_RED, COLOR_BLACK),
        VGA_COLOR(COLOR_LRED, COLOR_BLACK),
        VGA_COLOR(COLOR_YELLOW, COLOR_BLACK),
        VGA_COLOR(COLOR_GREEN, COLOR_BLACK),
        VGA_COLOR(COLOR_LBLUE, COLOR_BLACK)
    };
    
    for (int row = 0; row < ALIEN_ROWS; row++) {
        for (int col = 0; col < ALIEN_COLS; col++) {
            aliens[row][col].x = start_x + col * ALIEN_WIDTH;
            aliens[row][col].y = start_y + row * 2;
            aliens[row][col].active = true;
            aliens[row][col].color = alien_colors[row];
        }
    }
}

void draw_aliens() {
    for (int row = 0; row < ALIEN_ROWS; row++) {
        for (int col = 0; col < ALIEN_COLS; col++) {
            if (aliens[row][col].active) {
                char alien_char = (alien_timer < 15) ? 'M' : 'W';
                os_api->vgraphics_put_char(aliens[row][col].x, aliens[row][col].y, 
                                          alien_char, aliens[row][col].color);
                os_api->vgraphics_put_char(aliens[row][col].x + 1, aliens[row][col].y, 
                                          alien_char, aliens[row][col].color);
            }
        }
    }
}

void draw_bullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            os_api->vgraphics_put_char(bullets[i].x, bullets[i].y, '|', 
                                      VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
        }
    }
}

void draw_bombs() {
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (bombs[i].active) {
            os_api->vgraphics_put_char(bombs[i].x, bombs[i].y, 'v', 
                                      VGA_COLOR(COLOR_RED, COLOR_BLACK));
        }
    }
}

void init_game() {
    player_x = GAME_X + GAME_WIDTH / 2;
    score = 0;
    alien_dir = 1;
    alien_speed = 30;
    alien_timer = 0;
    game_over = false;
    victory = false;
    
    high_score = load_high_score();
    
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = false;
    }
    for (int i = 0; i < MAX_BOMBS; i++) {
        bombs[i].active = false;
    }
    
    init_aliens();
    
    os_api->vgraphics_init();
    os_api->vgraphics_clear(COLOR_BLACK);
    draw_bounds();
}

void move_player(int direction) {
    player_x += direction;
    
    if (player_x < GAME_X + 2) {
        player_x = GAME_X + 2;
    }
    if (player_x > GAME_X + GAME_WIDTH - 3) {
        player_x = GAME_X + GAME_WIDTH - 3;
    }
}

void fire_bullet() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = player_x;
            bullets[i].y = PLAYER_Y - 1;
            bullets[i].active = true;
            os_api->beep(1000, 20);
            break;
        }
    }
}

void alien_drop_bomb() {
    int active_cols[ALIEN_COLS];
    int active_count = 0;
    
    for (int col = 0; col < ALIEN_COLS; col++) {
        for (int row = ALIEN_ROWS - 1; row >= 0; row--) {
            if (aliens[row][col].active) {
                active_cols[active_count++] = col;
                break;
            }
        }
    }
    
    if (active_count > 0) {
        int bomb_col = active_cols[os_api->get_random(active_count)];
        
        for (int row = ALIEN_ROWS - 1; row >= 0; row--) {
            if (aliens[row][bomb_col].active) {
                for (int i = 0; i < MAX_BOMBS; i++) {
                    if (!bombs[i].active) {
                        bombs[i].x = aliens[row][bomb_col].x;
                        bombs[i].y = aliens[row][bomb_col].y + 1;
                        bombs[i].active = true;
                        break;
                    }
                }
                break;
            }
        }
    }
}

void update_bullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].y--;
            
            if (bullets[i].y < GAME_Y + 1) {
                bullets[i].active = false;
            } else {
                for (int row = 0; row < ALIEN_ROWS; row++) {
                    for (int col = 0; col < ALIEN_COLS; col++) {
                        if (aliens[row][col].active) {
                            if (bullets[i].x >= aliens[row][col].x && 
                                bullets[i].x <= aliens[row][col].x + 1 &&
                                bullets[i].y == aliens[row][col].y) {
                                
                                aliens[row][col].active = false;
                                bullets[i].active = false;
                                score += 10 * (ALIEN_ROWS - row);
                                os_api->beep(880, 30);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void update_bombs() {
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (bombs[i].active) {
            bombs[i].y++;
            
            if (bombs[i].y >= PLAYER_Y + 2) {
                bombs[i].active = false;
            } else if (bombs[i].y >= PLAYER_Y && 
                       bombs[i].x >= player_x - 1 && 
                       bombs[i].x <= player_x + 1) {
                game_over = true;
                bombs[i].active = false;
            }
        }
    }
}

void move_aliens() {
    alien_timer++;
    
    if (alien_timer >= alien_speed) {
        alien_timer = 0;
        
        int leftmost = GAME_X + GAME_WIDTH;
        int rightmost = GAME_X;
        int lowest = GAME_Y;
        
        for (int row = 0; row < ALIEN_ROWS; row++) {
            for (int col = 0; col < ALIEN_COLS; col++) {
                if (aliens[row][col].active) {
                    if (aliens[row][col].x < leftmost) leftmost = aliens[row][col].x;
                    if (aliens[row][col].x > rightmost) rightmost = aliens[row][col].x;
                    if (aliens[row][col].y > lowest) lowest = aliens[row][col].y;
                }
            }
        }
        
        bool should_drop = false;
        
        if (alien_dir == 1 && rightmost >= GAME_X + GAME_WIDTH - 3) {
            alien_dir = -1;
            should_drop = true;
        } else if (alien_dir == -1 && leftmost <= GAME_X + 2) {
            alien_dir = 1;
            should_drop = true;
        }
        
        if (should_drop) {
            for (int row = 0; row < ALIEN_ROWS; row++) {
                for (int col = 0; col < ALIEN_COLS; col++) {
                    if (aliens[row][col].active) {
                        aliens[row][col].y++;
                    }
                }
            }
            
            if (alien_speed > 10) alien_speed--;
            
            if (lowest >= BUNKER_Y - 1) {
                game_over = true;
            }
        } else {
            for (int row = 0; row < ALIEN_ROWS; row++) {
                for (int col = 0; col < ALIEN_COLS; col++) {
                    if (aliens[row][col].active) {
                        aliens[row][col].x += alien_dir;
                    }
                }
            }
        }
        
        if (os_api->get_random(100) < 20) {
            alien_drop_bomb();
        }
    }
}

bool check_victory() {
    for (int row = 0; row < ALIEN_ROWS; row++) {
        for (int col = 0; col < ALIEN_COLS; col++) {
            if (aliens[row][col].active) {
                return false;
            }
        }
    }
    return true;
}

void draw_game_over() {
    int box_x = 25;
    int box_y = 8;
    int box_w = 30;
    int box_h = 10;
    
    os_api->vgraphics_draw_box(box_x, box_y, box_w, box_h, VGA_COLOR(COLOR_RED, COLOR_BLACK));
    
    if (victory) {
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
        "  ____       _            _____                     _           ",
        " / ___|  ___| |__   __ _ |_   _|_ _ _ __ __ _ _ __ | |__   __ _ ",
        " \\___ \\ / __| '_ \\ / _` |  | |/ _` | '__/ _` | '_ \\| '_ \\ / _` |",
        "  ___) | (__| | | | (_| |  | | (_| | | | (_| | |_) | | | | (_| |",
        " |____/ \\___|_| |_|\\__, |  |_|\\__,_|_|  \\__,_| .__/|_| |_|\\__, |",
        "                   |___/                     |_|           |___/ "
    };
    
    uint8_t title_color = VGA_COLOR(COLOR_LGREEN, COLOR_BLACK);
    int title_y = 4;
    
    for (int i = 0; i < 6; i++) {
        int len = 0;
        while (title[i][len]) len++;
        int x = (SCREEN_WIDTH - len) / 2;
        os_api->vgraphics_put_string(x, title_y + i, title[i], title_color);
    }
    
    const char* instructions[] = {
        "",
        "Destroy all aliens before they reach you!",
        "",
        "CONTROLS:",
        "<- / ->  - Move ship",
        "A / D    - Move ship (alt)",
        "SPACE    - Shoot",
        "Q        - Quit game",
        "",
        "Press SPACE to start..."
    };
    
    uint8_t text_color = VGA_COLOR(COLOR_WHITE, COLOR_BLACK);
    int inst_y = 12;
    
    for (int i = 0; i < 10; i++) {
        int len = 0;
        while (instructions[i][len]) len++;
        int x = (SCREEN_WIDTH - len) / 2;
        os_api->vgraphics_put_string(x, inst_y + i, instructions[i], text_color);
    }
    
    while (1) {
        char key = os_api->keyboard_getchar();
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

void game_loop() {
    bool playing = true;
    
    show_title_screen();
    
    while (playing) {
        init_game();
        
        int frame_count = 0;
        
        while (!game_over && !victory) {
            clear_game_area();
            
            draw_player();
            draw_aliens();
            draw_bullets();
            draw_bombs();
            draw_score();
            
            char key = os_api->keyboard_getchar();
            if (key != 0) {
                if (key == 'q' || key == 'Q' || key == 0x1B) {
                    playing = false;
                    break;
                } else if (key == 'a' || key == 'A' || key == 0x4B) {
                    move_player(-1);
                } else if (key == 'd' || key == 'D' || key == 0x4D) {
                    move_player(1);
                } else if (key == ' ') {
                    fire_bullet();
                }
            }
            
            frame_count++;
            if (frame_count >= 3) {
                frame_count = 0;
                update_bullets();
                update_bombs();
                move_aliens();
                
                if (check_victory()) {
                    victory = true;
                    break;
                }
            }
            
            os_api->vgraphics_repaint();
            os_api->busy_delay(800);
        }
        
        if (!playing) break;
        
        save_high_score();
        draw_game_over();
        
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
