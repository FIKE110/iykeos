#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Adjustable game speed - higher = slower
int game_speed = 1000;

#define OS_API_ADDR 0x5F0F0

// Screen dimensions
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

// Game bounds
#define GAME_X 2
#define GAME_Y 2
#define GAME_WIDTH 60
#define GAME_HEIGHT 22

// Preview box (for next piece)
#define PREVIEW_X 65
#define PREVIEW_Y 4
#define PREVIEW_WIDTH 12
#define PREVIEW_HEIGHT 8

// Board dimensions (10 columns x 20 rows)
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define BOARD_OFFSET_X 12
#define BOARD_OFFSET_Y 3

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

// Piece types
#define PIECE_I 0
#define PIECE_O 1
#define PIECE_T 2
#define PIECE_S 3
#define PIECE_Z 4
#define PIECE_J 5
#define PIECE_L 6

// Piece colors
#define COLOR_I COLOR_CYAN
#define COLOR_O COLOR_YELLOW
#define COLOR_T COLOR_MAGENTA
#define COLOR_S COLOR_GREEN
#define COLOR_Z COLOR_RED
#define COLOR_J COLOR_BLUE
#define COLOR_L COLOR_BROWN

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

// Tetromino definitions - 7 pieces, 4 rotations each, 4x4 grid
// 1 = filled, 0 = empty
const uint8_t TETROMINOES[7][4][4][4] = {
    // I-piece (Cyan)
    {
        {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
        {{0,0,1,0}, {0,0,1,0}, {0,0,1,0}, {0,0,1,0}},
        {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}}
    },
    // O-piece (Yellow)
    {
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}
    },
    // T-piece (Purple/Magenta)
    {
        {{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,1,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}}
    },
    // S-piece (Green)
    {
        {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,0,0}},
        {{0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0}},
        {{1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}}
    },
    // Z-piece (Red)
    {
        {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,0,1,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0}},
        {{0,1,0,0}, {1,1,0,0}, {1,0,0,0}, {0,0,0,0}}
    },
    // J-piece (Blue)
    {
        {{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,1,0}, {0,0,1,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {1,1,0,0}, {0,0,0,0}}
    },
    // L-piece (Brown/Orange)
    {
        {{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,1,0}, {1,0,0,0}, {0,0,0,0}},
        {{1,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}}
    }
};

// Piece colors
const uint8_t PIECE_COLORS[7] = {
    COLOR_I, COLOR_O, COLOR_T, COLOR_S, COLOR_Z, COLOR_J, COLOR_L
};

// Game state structure
typedef struct {
    int board[BOARD_HEIGHT][BOARD_WIDTH];
    int current_piece;
    int current_x, current_y;
    int current_rotation;
    int next_piece;
    int score;
    int lines_cleared;
    bool game_over;
    uint32_t fall_delay;
    uint32_t last_fall_time;
} TetrisGame;

TetrisGame game;
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

// Load high score for Tetris (game 1)
int load_high_score() {
    int scores[6];
    load_all_high_scores(scores, 6);
    return scores[1];
}

// Save high score for Tetris (game 1)
void save_high_score() {
    int scores[6];
    
    load_all_high_scores(scores, 6);
    
    if (game.score > scores[1]) {
        scores[1] = game.score;
        high_score = game.score;
        save_all_high_scores(scores, 6);
    }
}

// Initialize Tetris game
void init_tetris() {
    // Clear board
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            game.board[y][x] = 0;
        }
    }
    
    game.score = 0;
    game.lines_cleared = 0;
    game.game_over = false;
    game.fall_delay = 1000;  // Start with 1 second delay
    game.last_fall_time = 0;
    
    // Seed RNG and get first pieces
    os_api->rng_seed();
    game.next_piece = os_api->get_random(7);
}

// Check if a position is valid for a piece
bool can_move(int piece, int rotation, int x, int y) {
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            if (TETROMINOES[piece][rotation][py][px]) {
                int new_x = x + px;
                int new_y = y + py;
                
                // Check bounds
                if (new_x < 0 || new_x >= BOARD_WIDTH || new_y >= BOARD_HEIGHT) {
                    return false;
                }
                
                // Check collision with locked pieces (only if within board)
                if (new_y >= 0 && game.board[new_y][new_x] != 0) {
                    return false;
                }
            }
        }
    }
    return true;
}

// Spawn a new piece
void spawn_piece() {
    game.current_piece = game.next_piece;
    game.next_piece = os_api->get_random(7);
    game.current_rotation = 0;
    game.current_x = BOARD_WIDTH / 2 - 2;
    game.current_y = 0;
    
    // Check if spawn position is valid
    if (!can_move(game.current_piece, game.current_rotation, game.current_x, game.current_y)) {
        game.game_over = true;
    }
}

// Rotate piece (with wall kick)
void rotate_piece() {
    int new_rotation = (game.current_rotation + 1) % 4;
    
    // Try to rotate
    if (can_move(game.current_piece, new_rotation, game.current_x, game.current_y)) {
        game.current_rotation = new_rotation;
    } else {
        // Try wall kicks
        if (can_move(game.current_piece, new_rotation, game.current_x - 1, game.current_y)) {
            game.current_x--;
            game.current_rotation = new_rotation;
        } else if (can_move(game.current_piece, new_rotation, game.current_x + 1, game.current_y)) {
            game.current_x++;
            game.current_rotation = new_rotation;
        }
    }
}

// Lock piece to board
void lock_piece() {
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            if (TETROMINOES[game.current_piece][game.current_rotation][py][px]) {
                int board_x = game.current_x + px;
                int board_y = game.current_y + py;
                
                if (board_y >= 0 && board_y < BOARD_HEIGHT && 
                    board_x >= 0 && board_x < BOARD_WIDTH) {
                    game.board[board_y][board_x] = game.current_piece + 1;
                }
            }
        }
    }
}

// Clear complete lines and return number cleared
int clear_lines() {
    int lines_cleared = 0;
    
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        bool complete = true;
        
        // Check if line is complete
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (game.board[y][x] == 0) {
                complete = false;
                break;
            }
        }
        
        if (complete) {
            lines_cleared++;
            
            // Shift all lines above down
            for (int shift_y = y; shift_y > 0; shift_y--) {
                for (int x = 0; x < BOARD_WIDTH; x++) {
                    game.board[shift_y][x] = game.board[shift_y - 1][x];
                }
            }
            
            // Clear top line
            for (int x = 0; x < BOARD_WIDTH; x++) {
                game.board[0][x] = 0;
            }
            
            // Check this line again (since we shifted down)
            y++;
        }
    }
    
    return lines_cleared;
}

// Update score based on lines cleared
void update_score(int lines) {
    // Scoring: 100/300/500/800 for 1/2/3/4 lines
    int points = 0;
    switch (lines) {
        case 1: points = 100; break;
        case 2: points = 300; break;
        case 3: points = 500; break;
        case 4: points = 800; break;
        default: points = 0; break;
    }
    
    game.score += points;
    game.lines_cleared += lines;
    
    // Speed progression: get faster every 10 lines
    int speed_level = game.lines_cleared / 10;
    if (speed_level > 10) speed_level = 10;
    
    game.fall_delay = 1000 - (speed_level * 80);
    if (game.fall_delay < 100) game.fall_delay = 1000;
}

// Draw a single block
void draw_block(int x, int y, uint8_t color) {
    os_api->vgraphics_put_char(x, y, ' ', VGA_COLOR(COLOR_BLACK, color));
}

// Draw the board with locked pieces
void draw_board() {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            int screen_x = BOARD_OFFSET_X + x * 2;
            int screen_y = BOARD_OFFSET_Y + y;
            
            if (game.board[y][x] == 0) {
                // Empty space
                draw_block(screen_x, screen_y, COLOR_BLACK);
                draw_block(screen_x + 1, screen_y, COLOR_BLACK);
            } else {
                // Filled block
                uint8_t color = PIECE_COLORS[game.board[y][x] - 1];
                draw_block(screen_x, screen_y, color);
                draw_block(screen_x + 1, screen_y, color);
            }
        }
    }
}

// Draw the current falling piece
void draw_piece() {
    uint8_t color = PIECE_COLORS[game.current_piece];
    
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            if (TETROMINOES[game.current_piece][game.current_rotation][py][px]) {
                int board_x = game.current_x + px;
                int board_y = game.current_y + py;
                
                if (board_y >= 0 && board_y < BOARD_HEIGHT && 
                    board_x >= 0 && board_x < BOARD_WIDTH) {
                    int screen_x = BOARD_OFFSET_X + board_x * 2;
                    int screen_y = BOARD_OFFSET_Y + board_y;
                    
                    draw_block(screen_x, screen_y, color);
                    draw_block(screen_x + 1, screen_y, color);
                }
            }
        }
    }
}

// Draw the next piece preview
void draw_preview() {
    // Clear preview area
    for (int y = PREVIEW_Y + 1; y < PREVIEW_Y + PREVIEW_HEIGHT - 1; y++) {
        for (int x = PREVIEW_X + 1; x < PREVIEW_X + PREVIEW_WIDTH - 1; x++) {
            os_api->vgraphics_put_char(x, y, ' ', VGA_COLOR(COLOR_BLACK, COLOR_BLACK));
        }
    }
    
    // Draw next piece centered in preview box
    uint8_t color = PIECE_COLORS[game.next_piece];
    int offset_x = PREVIEW_X + 2;
    int offset_y = PREVIEW_Y + 2;
    
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            if (TETROMINOES[game.next_piece][0][py][px]) {
                int screen_x = offset_x + px * 2;
                int screen_y = offset_y + py;
                
                draw_block(screen_x, screen_y, color);
                draw_block(screen_x + 1, screen_y, color);
            }
        }
    }
}

// Draw game bounds (box)
void draw_bounds() {
    os_api->vgraphics_draw_box(GAME_X, GAME_Y, GAME_WIDTH, GAME_HEIGHT, 
                               VGA_COLOR(COLOR_CYAN, COLOR_BLACK));
    
    os_api->vgraphics_put_string(GAME_X + (GAME_WIDTH - 10) / 2, GAME_Y, 
                                 " TETRIS ", VGA_COLOR(COLOR_WHITE, COLOR_BLUE));
}

// Draw preview box (NEXT piece)
void draw_preview_box() {
    os_api->vgraphics_draw_box(PREVIEW_X, PREVIEW_Y, PREVIEW_WIDTH, PREVIEW_HEIGHT, 
                               VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));
    
    os_api->vgraphics_put_string(PREVIEW_X + 3, PREVIEW_Y, 
                                 " NEXT ", VGA_COLOR(COLOR_WHITE, COLOR_DGRAY));
}

// Draw score and high score
void draw_score() {
    char score_str[20];
    
    os_api->vgraphics_put_string(2, 23, "Score: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->int_to_str(game.score, score_str);
    os_api->vgraphics_put_string(9, 23, score_str, VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
    
    os_api->vgraphics_put_string(32, 23, "Hi: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->int_to_str(high_score, score_str);
    os_api->vgraphics_put_string(36, 23, score_str, VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));
    
    os_api->vgraphics_put_string(50, 23, "Lines: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->int_to_str(game.lines_cleared, score_str);
    os_api->vgraphics_put_string(57, 23, score_str, VGA_COLOR(COLOR_LGREEN, COLOR_BLACK));
}

// Draw controls
void draw_controls() {
    os_api->vgraphics_put_string(2, 24, "Arrows: Move | Up: Rotate | Down: Soft | Space: Hard | Q: Quit", 
                                 VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));
}

// Draw game over screen
void draw_game_over() {
    // Draw box in center of board
    int box_x = BOARD_OFFSET_X + 8;
    int box_y = BOARD_OFFSET_Y + 6;

    os_api->vgraphics_draw_box(box_x, box_y, 24, 8, VGA_COLOR(COLOR_RED, COLOR_BLACK));

    // Title
    os_api->vgraphics_put_string(box_x + 6, box_y + 1, "GAME OVER",
                                 VGA_COLOR(COLOR_LRED, COLOR_BLACK));

    // Final score
    char score_str[20];
    os_api->int_to_str(game.score, score_str);
    os_api->vgraphics_put_string(box_x + 2, box_y + 3, "Score: ",
                                 VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->vgraphics_put_string(box_x + 9, box_y + 3, score_str,
                                 VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));

    // High score
    os_api->int_to_str(high_score, score_str);
    os_api->vgraphics_put_string(box_x + 2, box_y + 5, "High: ",
                                 VGA_COLOR(COLOR_WHITE, COLOR_BLACK));
    os_api->vgraphics_put_string(box_x + 8, box_y + 5, score_str,
                                 VGA_COLOR(COLOR_LCYAN, COLOR_BLACK));

    // Instructions
    os_api->vgraphics_put_string(box_x + 2, box_y + 6, "R: Restart  Q: Quit",
                                 VGA_COLOR(COLOR_LGRAY, COLOR_BLACK));
}

// Hard drop - move piece to bottom instantly
void hard_drop() {
    while (can_move(game.current_piece, game.current_rotation, game.current_x, game.current_y + 1)) {
        game.current_y++;
        game.score += 2;  // Bonus for hard drop
    }
}

// Get current time in milliseconds (approximate)
uint32_t get_time_ms() {
    // Use a simple counter based on frame count
    static uint32_t counter = 0;
    return counter++;
}

// Main game update
void update_game() {
    // Try to move piece down
    if (can_move(game.current_piece, game.current_rotation, game.current_x, game.current_y + 1)) {
        game.current_y++;
    } else {
        // Lock piece
        lock_piece();
        
        // Clear lines
        int lines = clear_lines();
        if (lines > 0) {
            update_score(lines);
        }
        
        // Spawn next piece
        spawn_piece();
        
        if (game.game_over) {
            save_high_score();
        }
    }
}

// Main game loop
void game_loop() {
    bool playing = true;
    
    // Load high score at game start
    high_score = load_high_score();
    
    // Initialize game
    init_tetris();
    spawn_piece();
    
    // Initialize graphics
    os_api->vgraphics_init();
    os_api->vgraphics_clear(COLOR_BLACK);
    
    // Draw static elements
    draw_bounds();
    draw_preview_box();
    draw_controls();
    
    // Main loop
    while (playing) {
        // Handle input
        char c = os_api->keyboard_getchar();

        if (c == 'q' || c == 'Q' || c == 0x1B) {
            playing = false;
            break;
        }

        // Handle keyboard input
        switch (c) {
            case 0x11: // KEY_UP - rotate
                rotate_piece();
                break;
            case 0x12: // KEY_DOWN - soft drop
                if (can_move(game.current_piece, game.current_rotation, game.current_x, game.current_y + 1)) {
                    game.current_y++;
                    game.score += 1;
                }
                break;
            case 0x13: // KEY_LEFT - move left
                if (can_move(game.current_piece, game.current_rotation, game.current_x - 1, game.current_y)) {
                    game.current_x--;
                }
                break;
            case 0x14: // KEY_RIGHT - move right
                if (can_move(game.current_piece, game.current_rotation, game.current_x + 1, game.current_y)) {
                    game.current_x++;
                }
                break;
            case ' ': // Space - hard drop
                hard_drop();
                // Force immediate lock
                if (!can_move(game.current_piece, game.current_rotation, game.current_x, game.current_y + 1)) {
                    lock_piece();
                    int lines = clear_lines();
                    if (lines > 0) update_score(lines);
                    spawn_piece();
                    if (game.game_over) save_high_score();
                }
                break;
        }

        // Automatic falling based on speed
        static int fall_counter = 0;
        fall_counter++;
        if (fall_counter >= game_speed) {
            update_game();
            fall_counter = 0;
        }

        // Draw everything
        draw_board();
        draw_piece();
        draw_preview();
        draw_score();

        os_api->vgraphics_repaint();
        // Frame delay
        for (volatile int d = 0; d < 10000; d++);

        // Check for game over
        if (game.game_over) {
            // Game over - show screen and wait for input
            save_high_score();

            // Draw final state
            draw_board();
            draw_game_over();

            // Wait for R (restart) or Q (quit)
            while (1) {
                char c = os_api->keyboard_getchar();
                if (c == 'q' || c == 'Q' || c == 0x1B) {
                    playing = false;
                    break;
                }
                if (c == 'r' || c == 'R') {
                    // Restart game
                    init_tetris();
                    spawn_piece();
                    game_speed = 1000;  // Reset speed to default
                    break;
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
