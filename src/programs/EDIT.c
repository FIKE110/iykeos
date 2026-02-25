#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define OS_API_ADDR 0x5F0F0

/* Screen dimensions */
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

/* Editor uses full screen */
#define EDITOR_X 0
#define EDITOR_Y 1
#define EDITOR_WIDTH 80
#define EDITOR_HEIGHT 23

/* Text buffer */
#define MAX_LINES 1000
#define MAX_LINE_LENGTH 76
#define MAX_BUFFER_SIZE 8192

/* Color scheme - Modern Cyan/Teal Theme */
#define COLOR_BG          0x0   /* Black */
#define COLOR_TEXT        0xF   /* White */
#define COLOR_GRAY        0x8   /* Dark Gray */
#define COLOR_LGRAY       0x7   /* Light Gray */
#define COLOR_HEADER_BG   0x3   /* Cyan - top bar */
#define COLOR_HEADER_FG   0x0   /* Black - top bar text */
#define COLOR_STATUS_BG   0x8   /* Dark Gray */
#define COLOR_STATUS_FG   0xF   /* White */
#define COLOR_CMD_FG      0xE   /* Yellow */
#define COLOR_CURSOR      0xF   /* White */
#define COLOR_LINE_NUM    0x8   /* Dark Gray */
#define COLOR_MODIFIED    0xC   /* Light Red */
#define COLOR_BOTTOM_BG   0xB   /* Light Cyan - bottom bar */
#define COLOR_BOTTOM_FG   0x0   /* Black - bottom bar text */

#define VGA_COLOR(fg, bg) ((bg << 4) | (fg))

/* Keys */
#define KEY_UP          0x11
#define KEY_DOWN        0x12
#define KEY_LEFT        0x13
#define KEY_RIGHT       0x14

#define KEY_CTRL        0x15
#define KEY_ALT         0x16
#define KEY_SHIFT       0x17
#define KEY_CAPSLOCK    0x18
#define KEY_FN          0x19
#define KEY_ESC         0x1B
#define KEY_ENTER       '\n'
#define KEY_BACKSPACE   0x08
#define KEY_TAB         '\t'
#define KEY_SPACE       ' '
#define KEY_DELETE      0x7F

#define KEY_F1          0x80
#define KEY_F2          0x81
#define KEY_F3          0x82
#define KEY_F4          0x83
#define KEY_F5          0x84
#define KEY_F6          0x85
#define KEY_F7          0x86
#define KEY_F8          0x87
#define KEY_F9          0x88
#define KEY_F10         0x89
#define KEY_F11         0x8A
#define KEY_F12         0x8B

#define KEY_HOME        0x8C
#define KEY_END         0x8D
#define KEY_PAGEUP      0x8E
#define KEY_PAGEDOWN    0x8F
#define KEY_INSERT      0x90

/* FAT16 directory entry */
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

/* OS API structure */

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

/* Editor state */
char text_buffer[MAX_BUFFER_SIZE];
int text_length = 0;
int cursor_pos = 0;
int scroll_line = 0;
char filename[13] = "UNTITLED.TXT";
int modified = 0;
int quit = 0;
int command_mode = 0;  /* 0 = insert mode, 1 = command mode */

/* Line tracking */
int line_starts[MAX_LINES];
int line_count = 1;

/* Message display */
char message[40] = "";
int message_timer = 0;

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

void init_api(void) {
    os_api = (os_api_t*)OS_API_ADDR;
}

void clear_buffer(void) {
    os_api->memset(text_buffer, 0, MAX_BUFFER_SIZE);
    text_length = 0;
    cursor_pos = 0;
    scroll_line = 0;
    modified = 0;
    line_count = 1;
    line_starts[0] = 0;
    message[0] = '\0';
}

void show_message(const char* msg) {
    os_api->strcpy(message, msg);
    message_timer = 100;  /* Display for ~100 frames */
}

/* Calculate line starts based on current text */
void recalculate_lines(void) {
    line_count = 0;
    int pos = 0;
    
    while (pos < text_length && line_count < MAX_LINES) {
        line_starts[line_count] = pos;
        line_count++;
        
        /* Find end of this line */
        while (pos < text_length && text_buffer[pos] != '\n') {
            pos++;
        }
        
        /* Skip the newline */
        if (pos < text_length && text_buffer[pos] == '\n') {
            pos++;
        }
    }
    
    if (line_count == 0) {
        line_count = 1;
        line_starts[0] = 0;
    }
}

/* Get cursor line and column */
void get_cursor_line_col(int* line, int* col) {
    *line = 0;
    *col = 0;
    
    for (int i = 0; i < line_count; i++) {
        int line_end;
        if (i < line_count - 1) {
            line_end = line_starts[i + 1] - 1;
            if (text_buffer[line_end] == '\n') {
                line_end--;
            }
        } else {
            line_end = text_length - 1;
        }
        
        if (cursor_pos <= line_end || i == line_count - 1) {
            *line = i;
            *col = cursor_pos - line_starts[i];
            return;
        }
    }
}

/* Set cursor position from line and column */
void set_cursor_from_line_col(int line, int col) {
    if (line < 0) line = 0;
    if (line >= line_count) line = line_count - 1;
    
    cursor_pos = line_starts[line] + col;
    
    /* Find end of this line */
    int line_end;
    if (line < line_count - 1) {
        line_end = line_starts[line + 1] - 1;
        if (text_buffer[line_end] == '\n') {
            line_end--;
        }
    } else {
        line_end = text_length - 1;
    }
    
    if (cursor_pos > line_end + 1) {
        cursor_pos = line_end + 1;
    }
    if (cursor_pos < line_starts[line]) {
        cursor_pos = line_starts[line];
    }
}

/* ============================================================================
 * FILE OPERATIONS
 * ============================================================================ */

void load_file(const char* fname) {
    uint8_t temp_buffer[MAX_BUFFER_SIZE];
    int size = os_api->fat16_file_load(fname, temp_buffer);
    
    if (size > 0) {
        clear_buffer();
        if (size >= MAX_BUFFER_SIZE) {
            size = MAX_BUFFER_SIZE - 1;
        }
        os_api->memcpy(text_buffer, temp_buffer, size);
        text_length = size;
        text_buffer[text_length] = '\0';
        
        /* Copy filename */
        int i = 0;
        while (fname[i] && i < 12) {
            filename[i] = fname[i];
            i++;
        }
        filename[i] = '\0';
        
        recalculate_lines();
        show_message("File loaded");
    } else {
        show_message("Failed to load file");
    }
}

void save_file(void) {
    if (filename[0] == '\0') {
        os_api->strcpy(filename, "UNTITLED.TXT");
    }
    
    os_api->fat16_file_save(filename, (uint8_t*)text_buffer, text_length);
    modified = 0;
    show_message("File saved");
}

/* ============================================================================
 * TEXT EDITING
 * ============================================================================ */

void insert_char(char c) {
    if (text_length >= MAX_BUFFER_SIZE - 1) return;
    
    /* Shift everything after cursor */
    for (int i = text_length; i > cursor_pos; i--) {
        text_buffer[i] = text_buffer[i - 1];
    }
    
    text_buffer[cursor_pos] = c;
    cursor_pos++;
    text_length++;
    text_buffer[text_length] = '\0';
    
    modified = 1;
    recalculate_lines();
}

void delete_char(void) {
    if (cursor_pos >= text_length || text_length == 0) return;
    
    /* Shift everything after cursor left */
    for (int i = cursor_pos; i < text_length; i++) {
        text_buffer[i] = text_buffer[i + 1];
    }
    
    text_length--;
    text_buffer[text_length] = '\0';
    
    modified = 1;
    recalculate_lines();
}

void backspace(void) {
    if (cursor_pos == 0) return;
    
    cursor_pos--;
    delete_char();
}

void insert_newline(void) {
    insert_char('\n');
}

/* ============================================================================
 * CURSOR MOVEMENT
 * ============================================================================ */

void move_cursor_left(void) {
    if (cursor_pos > 0) {
        cursor_pos--;
    }
}

void move_cursor_right(void) {
    if (cursor_pos < text_length) {
        cursor_pos++;
    }
}

void move_cursor_up(void) {
    int line, col;
    get_cursor_line_col(&line, &col);
    
    if (line > 0) {
        set_cursor_from_line_col(line - 1, col);
    }
}

void move_cursor_down(void) {
    int line, col;
    get_cursor_line_col(&line, &col);
    
    if (line < line_count - 1) {
        set_cursor_from_line_col(line + 1, col);
    }
}

void move_cursor_home(void) {
    int line, col;
    get_cursor_line_col(&line, &col);
    cursor_pos = line_starts[line];
}

void move_cursor_end(void) {
    int line, col;
    get_cursor_line_col(&line, &col);
    
    if (line < line_count - 1) {
        cursor_pos = line_starts[line + 1] - 1;
        if (text_buffer[cursor_pos] == '\n') {
            cursor_pos--;
        }
    } else {
        cursor_pos = text_length - 1;
    }
    
    if (cursor_pos < 0) cursor_pos = 0;
}

void ensure_cursor_visible(void) {
    int line, col;
    get_cursor_line_col(&line, &col);
    
    /* Scroll up if cursor is above visible area */
    while (line < scroll_line && scroll_line > 0) {
        scroll_line--;
    }
    
    /* Scroll down if cursor is below visible area */
    while (line >= scroll_line + EDITOR_HEIGHT - 2 && scroll_line < line_count - 1) {
        scroll_line++;
    }
}

/* ============================================================================
 * DRAWING - GUI STYLE
 * ============================================================================ */

void draw_header(void) {
    /* Top header bar - Blue background */
    os_api->vgraphics_draw_rect_fill(0, 0, SCREEN_WIDTH, 1, COLOR_GRAY);
    
    /* Title on left */
    char title[32];
    os_api->strcpy(title, " EDIT: ");
    os_api->strcat(title, filename);
    if (modified) {
        os_api->strcat(title, " *");
    }
    
    os_api->vgraphics_put_string(1, 0, title, VGA_COLOR(COLOR_HEADER_FG, COLOR_HEADER_BG));
    
    /* Mode indicator on right */
    const char* mode_str = command_mode ? "[COMMAND]" : "[INSERT] ";
    int mode_color = command_mode ? 0x4 : 0x1;  /* Red for command, Blue for insert */
    os_api->vgraphics_put_string(SCREEN_WIDTH - 10, 0, mode_str, VGA_COLOR(mode_color, COLOR_HEADER_BG));
}

void draw_text_area(void) {
    /* Main text area - White background */
    os_api->vgraphics_draw_rect_fill(0, 1, SCREEN_WIDTH, EDITOR_HEIGHT - 1, VGA_COLOR(COLOR_TEXT, COLOR_BG));
    
    /* Draw left border line */
    for (int y = 1; y < EDITOR_HEIGHT; y++) {
        os_api->vgraphics_put_char(0, y, 0xB3, VGA_COLOR(COLOR_GRAY, COLOR_BG));
    }
    
    /* Draw right border line */
    for (int y = 1; y < EDITOR_HEIGHT; y++) {
        os_api->vgraphics_put_char(SCREEN_WIDTH - 1, y, 0xB3, VGA_COLOR(COLOR_GRAY, COLOR_BG));
    }
    
    int max_lines_to_show = EDITOR_HEIGHT - 2;
    
    for (int i = 0; i < max_lines_to_show; i++) {
        int line_idx = scroll_line + i;
        int screen_y = 1 + i;
        
        if (line_idx >= line_count) {
            /* Empty line - just draw background */
            for (int x = 1; x < SCREEN_WIDTH - 1; x++) {
                os_api->vgraphics_put_char(x, screen_y, ' ', VGA_COLOR(COLOR_TEXT, COLOR_BG));
            }
            continue;
        }
        
        /* Line number area (columns 1-4) - Gray background */
        char line_num[5];
        os_api->int_to_str(line_idx + 1, line_num);
        int num_len = os_api->strlen(line_num);
        
        /* Pad line number with spaces */
        for (int x = 1; x < 5; x++) {
            char c = ' ';
            if (x >= 4 - num_len && x < 4) {
                c = line_num[x - (4 - num_len)];
            }
            os_api->vgraphics_put_char(x, screen_y, c, VGA_COLOR(COLOR_LINE_NUM, COLOR_BG));
        }
        
        /* Separator */
        os_api->vgraphics_put_char(5, screen_y, 0xB3, VGA_COLOR(COLOR_GRAY, COLOR_BG));
        
        /* Line content */
        int line_start = line_starts[line_idx];
        int line_end;
        
        if (line_idx < line_count - 1) {
            line_end = line_starts[line_idx + 1] - 1;
        } else {
            line_end = text_length;
        }
        
        int col = 0;
        for (int pos = line_start; pos < line_end && col < SCREEN_WIDTH - 8; pos++) {
            char c = text_buffer[pos];
            if (c == '\n') break;
            
            os_api->vgraphics_put_char(6 + col, screen_y, c, VGA_COLOR(COLOR_TEXT, COLOR_BG));
            col++;
        }
        
        /* Clear rest of line */
        for (; col < SCREEN_WIDTH - 8; col++) {
            os_api->vgraphics_put_char(6 + col, screen_y, ' ', VGA_COLOR(COLOR_TEXT, COLOR_BG));
        }
    }
}

void draw_cursor(void) {
    int line, col;
    get_cursor_line_col(&line, &col);
    
    int screen_y = 1 + (line - scroll_line);
    int screen_x = 6 + col;
    
    /* Make sure cursor is within editor bounds */
    if (screen_y >= 1 && screen_y < EDITOR_HEIGHT - 1 &&
        screen_x >= 6 && screen_x < SCREEN_WIDTH - 1) {
        
        /* Draw cursor as underscore _ */
        os_api->vgraphics_put_char(screen_x, screen_y, '_', 
                                    VGA_COLOR(COLOR_CURSOR, COLOR_BG));
    }
}

void draw_status_bar(void) {
    int status_y = EDITOR_HEIGHT;
    
    /* Status bar background - Dark Gray */
    os_api->vgraphics_draw_rect_fill(0, status_y, SCREEN_WIDTH, 1, VGA_COLOR(COLOR_STATUS_FG, COLOR_STATUS_BG));
    
    /* Line and column info */
    int line, col;
    get_cursor_line_col(&line, &col);
    
    char status[20];
    os_api->strcpy(status, "Ln ");
    char num_str[8];
    os_api->int_to_str(line + 1, num_str);
    os_api->strcat(status, num_str);
    os_api->strcat(status, ", Col ");
    os_api->int_to_str(col + 1, num_str);
    os_api->strcat(status, num_str);
    
    os_api->vgraphics_put_string(1, status_y, status, VGA_COLOR(COLOR_STATUS_FG, COLOR_STATUS_BG));
    
    /* Help text - Yellow for command mode hint */
    if (command_mode) {
        os_api->vgraphics_put_string(20, status_y, 
                                      "CMD: S=Save Q=Quit N=New Ctrl=Insert",
                                      VGA_COLOR(COLOR_CMD_FG, COLOR_STATUS_BG));
    } else {
        os_api->vgraphics_put_string(20, status_y, 
                                      "Press Ctrl for Command Mode",
                                      VGA_COLOR(COLOR_LGRAY, COLOR_STATUS_BG));
    }
    
    /* Message display */
    if (message_timer > 0) {
        message_timer--;
        int msg_x = SCREEN_WIDTH - os_api->strlen(message) - 2;
        if (msg_x < 50) msg_x = 50;
        os_api->vgraphics_put_string(msg_x, status_y, message, 
                                      VGA_COLOR(COLOR_MODIFIED, COLOR_STATUS_BG));
    }
}

void draw_bottom_bar(void) {
    int bottom_y = SCREEN_HEIGHT - 1;
    
    /* Bottom bar - Light Cyan */
    os_api->vgraphics_draw_rect_fill(0, bottom_y, SCREEN_WIDTH, 1, VGA_COLOR(COLOR_BOTTOM_FG, COLOR_BOTTOM_BG));
    
    os_api->vgraphics_put_string(1, bottom_y, "IYKEOS Text Editor v1.0", 
                                  VGA_COLOR(0XF, 0X0));
}

void draw_editor(void) {
    draw_header();
    draw_text_area();
    draw_cursor();
    draw_status_bar();
    draw_bottom_bar();
    
    os_api->vgraphics_repaint();
}

/* ============================================================================
 * INPUT HANDLING
 * ============================================================================ */

void handle_command_input(char c) {
    switch (c) {
        case 's':
        case 'S':
            save_file();
            command_mode = 0;
            break;
        case 'q':
        case 'Q':
            quit = 1;
            break;
        case 'n':
        case 'N':
            clear_buffer();
            os_api->strcpy(filename, "UNTITLED.TXT");
            command_mode = 0;
            show_message("New file");
            break;
        case KEY_UP:
            move_cursor_up();
            break;
        case KEY_DOWN:
            move_cursor_down();
            break;
        case KEY_LEFT:
            move_cursor_left();
            break;
        case KEY_RIGHT:
            move_cursor_right();
            break;
        case KEY_HOME:
            move_cursor_home();
            break;
        case KEY_END:
            move_cursor_end();
            break;
        case 0x1B:  /* Escape */
        case KEY_CTRL:
            /* Toggle back to insert mode */
            command_mode = 0;
            show_message("Insert mode");
            break;
        default:
            /* Unknown command - stay in command mode */
            break;
    }
}

void handle_insert_input(char c) {
    switch (c) {
        case KEY_CTRL:
            /* Toggle to command mode */
            command_mode = 1;
            show_message("Command mode - Press S/Q/N/Ctrl");
            break;
        case KEY_UP:
            move_cursor_up();
            break;
        case KEY_DOWN:
            move_cursor_down();
            break;
        case KEY_LEFT:
            move_cursor_left();
            break;
        case KEY_RIGHT:
            move_cursor_right();
            break;
        case KEY_HOME:
            move_cursor_home();
            break;
        case KEY_END:
            move_cursor_end();
            break;
        case KEY_ESC:
            command_mode = 1;
            show_message("Command mode - Press S/Q/N/Ctrl");
            break;
        case KEY_ENTER:
            insert_newline();
            break;
        case KEY_BACKSPACE:
            backspace();
            break;
        case KEY_DELETE:
            delete_char();
            break;
        case KEY_TAB:
            /* Insert 4 spaces for tab */
            for (int i = 0; i < 4; i++) {
                insert_char(' ');
            }
            break;
        default:
            /* Printable characters */
            if (c >= 32 && c <= 126) {
                insert_char(c);
            }
            break;
    }
}

void handle_input(char c) {
    if (c == 0) return;
    
    if (command_mode) {
        handle_command_input(c);
    } else {
        handle_insert_input(c);
    }
    
    ensure_cursor_visible();
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

void editor_loop(void) {
    while (!quit) {
        draw_editor();
        
        char c = os_api->keyboard_getchar();
        handle_input(c);
        
        /* Small delay to prevent busy-waiting */
        os_api->busy_delay(1000);
    }
}

int main(void) {
    init_api();
    
    /* Initialize virtual graphics */
    os_api->vgraphics_init();
    os_api->keyboard_init();
    os_api->disable_cursor();
    

    /* Clear and setup */
    clear_buffer();
    
    /* Main editor loop */
    editor_loop();
    
    /* Cleanup */
    os_api->enable_cursor();
    os_api->vgraphics_clear(COLOR_BG);
    os_api->screen_clear_shell();
    
    return 0;
}
