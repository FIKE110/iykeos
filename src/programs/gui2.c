#include <stddef.h>
#include <stdint.h>
#define OS_API_ADDR 0x5F0F0


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

typedef struct {
    char name[8];         // Filename
    char ext[3];          // Extension
    uint8_t attr;         // File attributes
    uint8_t reserved[10]; // Reserved
    uint16_t time;        // Last write time
    uint16_t date;        // Last write date
    uint16_t first_cluster; // First cluster of file
    uint32_t size;        // File size in bytes
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
    void (*graphics_putpixel)(int x, int y, uint8_t color);

} os_api_t;


os_api_t* os_api;

void init_api(void){
    os_api=(os_api_t*) OS_API_ADDR;
}


uint16_t gui_buffer[80 * 25];

void buffer_put_char(int x, int y, char c, uint8_t attr) {
    if (x >= 0 && x < 80 && y >= 0 && y < 25) {
        gui_buffer[y * 80 + x] = (uint16_t)c | ((uint16_t)attr << 8);
    }
}

void buffer_put_string(int x, int y, const char* str, uint8_t attr) {
    int i = 0;
    while (str[i]) {
        buffer_put_char(x + i, y, str[i], attr);
        i++;
    }
}

void buffer_draw_rect_fill(int x, int y, int w, int h, uint8_t color) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            buffer_put_char(x + i, y + j, ' ', (color << 4) | 0x0F);
        }
    }
}

void buffer_draw_window(int x, int y, int w, int h, const char* title, uint8_t bg_color) {
    buffer_draw_rect_fill(x, y, w, h, bg_color);
    buffer_draw_rect_fill(x, y, w, 1, COLOR_BLUE);
    uint8_t title_attr = (COLOR_BLUE << 4) | COLOR_WHITE;
    
    int title_len = 0;
    while(title[title_len]) title_len++;
    int tx = x + (w - title_len) / 2;
    if (tx < x) tx = x;
    buffer_put_string(tx, y, title, title_attr);
}

void buffer_clear(uint8_t color) {
    uint16_t val = (uint16_t)' ' | ((uint16_t)color << 12); // Background color
    for (int i = 0; i < 80 * 25; i++) {
        gui_buffer[i] = val;
    }
}

void buffer_present() {
    uint16_t* vga = (uint16_t*)0xB8000;
    for (int i = 0; i < 80 * 25; i++) {
        vga[i] = gui_buffer[i];
    }
}

int is_hover(int mx, int my, int x, int y, int w, int h) {
    return (mx >= x && mx < x + w && my >= y && my < y + h);
}

void draw_icon(int x, int y, const char* art, const char* label, uint8_t bg_col, int mx, int my) {
    int hover = is_hover(mx, my, x, y, 6, 2);
    uint8_t rect_col = bg_col;
    if (hover) {
        if (bg_col == COLOR_DARK_GRAY) rect_col = COLOR_LIGHT_GRAY;
        else if (bg_col == COLOR_LIGHT_GRAY) rect_col = COLOR_WHITE;
        else if (bg_col < 8) rect_col = bg_col + 8;
    }

    buffer_draw_rect_fill(x, y, 6, 2, rect_col);
    
    uint8_t text_col = COLOR_WHITE;
    if (rect_col == COLOR_YELLOW || rect_col == COLOR_WHITE || rect_col == COLOR_LIGHT_GREEN || rect_col == COLOR_LIGHT_CYAN || rect_col == COLOR_LIGHT_GRAY) {
        text_col = COLOR_BLACK;
    }
    
    buffer_put_string(x+1, y, art, (rect_col << 4) | text_col);
    
    uint8_t label_col = hover ? COLOR_YELLOW : COLOR_WHITE;
    buffer_put_string(x, y+2, label, (COLOR_BLUE << 4) | label_col);
}

#define APP_NONE 0
#define APP_FILES 1
#define APP_SHELL 2
#define APP_WEB   3
#define APP_EDIT  4
#define APP_CONFIG 5
#define APP_MUSIC 6
#define APP_VIDEO 7
#define APP_CALC  8
#define APP_NOTES 9
#define APP_HELP  10

void draw_files_app(int x, int y, int w, int h, int mx, int my, int scroll_offset) {
    // Background
    buffer_draw_rect_fill(x, y, w, h, COLOR_WHITE);
    
    // Toolbar
    buffer_draw_rect_fill(x, y, w, 1, COLOR_LIGHT_GRAY);
    
    // New File Button
    int hover_nf = (mx >= x && mx < x + 10 && my == y);
    uint8_t attr_nf = hover_nf ? ((COLOR_BLUE << 4) | COLOR_WHITE) : ((COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    buffer_put_string(x, y, " New File ", attr_nf);
    
    // New Dir Button
    int hover_nd = (mx >= x + 11 && mx < x + 21 && my == y);
    uint8_t attr_nd = hover_nd ? ((COLOR_BLUE << 4) | COLOR_WHITE) : ((COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    buffer_put_string(x + 11, y, " New Dir ", attr_nd);
    
    // Scroll Up
    int hover_up = (mx >= x + w - 6 && mx < x + w - 3 && my == y);
    uint8_t attr_up = hover_up ? ((COLOR_BLUE << 4) | COLOR_WHITE) : ((COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    buffer_put_string(x + w - 6, y, "[^]", attr_up);

    // Scroll Down
    int hover_dn = (mx >= x + w - 3 && mx < x + w && my == y);
    uint8_t attr_dn = hover_dn ? ((COLOR_BLUE << 4) | COLOR_WHITE) : ((COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    buffer_put_string(x + w - 3, y, "[v]", attr_dn);

    // Header
    buffer_draw_rect_fill(x, y + 1, w, 1, COLOR_LIGHT_GRAY);
    buffer_put_string(x + 1, y + 1, "Name     Ext   Size", (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    
    // Content
    fat16_dir_entry entries[512]; 
    int count = os_api->fat16_list_root(entries, 512);
    
    // Debug count
    char count_str[16];
    os_api->int_to_str(count, count_str);
    buffer_put_string(x + 60, y + 1, "Count: ", (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    buffer_put_string(x + 67, y + 1, count_str, (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    
    int row = 0;
    for (int i = 0; i < count; i++) {
        if (entries[i].name[0] == 0) continue;
        if ((uint8_t)entries[i].name[0] == 0xE5) continue; // Deleted
        
        // Skip for scroll
        if (i < scroll_offset) continue;
        
        if (row >= h - 3) break; // Clip (h - toolbar - header - padding)
        
        int item_y = y + 2 + row;
        
        // Hover effect
        int is_hovered = (mx >= x && mx < x + w && my == item_y);
        uint8_t bg = is_hovered ? COLOR_BLUE : COLOR_WHITE;
        uint8_t fg = is_hovered ? COLOR_WHITE : COLOR_BLACK;
        uint8_t attr = (bg << 4) | fg;
        
        // Clear line background
        for(int k=0; k<w; k++) {
            buffer_put_char(x+k, item_y, ' ', attr);
        }
        
        // Name
        char name_buf[9];
        os_api->memcpy(name_buf, entries[i].name, 8);
        name_buf[8] = 0;
        // Trim spaces
        for(int k=7; k>=0; k--) { if(name_buf[k]==' ') name_buf[k]=0; else break; }
        buffer_put_string(x + 1, item_y, name_buf, attr);
        
        // Ext
        char ext_buf[4];
        os_api->memcpy(ext_buf, entries[i].ext, 3);
        ext_buf[3] = 0;
        buffer_put_string(x + 10, item_y, ext_buf, attr);
        
        // Size or DIR
        if (entries[i].attr & 0x10) {
            buffer_put_string(x + 16, item_y, "<DIR>", attr);
        } else {
            char size_buf[10];
            os_api->int_to_str(entries[i].size, size_buf);
            buffer_put_string(x + 16, item_y, size_buf, attr);
        }
        
        row++;
    }
}

int handle_files_click(int x, int y, int w, int h, int mx, int my, int scroll_offset) {
    // Toolbar Clicks
    if (my == y) {
        if (mx >= x && mx < x + 10) return 2; // New File
        if (mx >= x + 11 && mx < x + 21) return 3; // New Dir
        if (mx >= x + w - 6 && mx < x + w - 3) return 4; // Scroll Up
        if (mx >= x + w - 3 && mx < x + w) return 5; // Scroll Down
        return 0;
    }

    fat16_dir_entry entries[512];
    int count = os_api->fat16_list_root(entries, 512);
    
    int row = 0;
    for (int i = 0; i < count; i++) {
        if (entries[i].name[0] == 0) continue;
        if ((uint8_t)entries[i].name[0] == 0xE5) continue;
        
        if (i < scroll_offset) continue;
        if (row >= h - 3) break;
        
        int item_y = y + 2 + row;
        
        if (mx >= x && mx < x + w && my == item_y) {
            // Clicked this item
            char name_buf[12];
            os_api->memcpy(name_buf, entries[i].name, 8);
            int k = 7;
            while(k>=0 && name_buf[k]==' ') name_buf[k--]=0;
            name_buf[k+1] = 0;
            
            if (entries[i].attr & 0x10) {
                // Directory
                os_api->fat16_chdir(name_buf);
                return 1; // Changed dir
            }
            return 0;
        }
        row++;
    }
    return 0;
}

void draw_calc_app(int x, int y, int w, int h, int mx, int my, char* calc_buffer) {
    // Background
    buffer_draw_rect_fill(x, y, w, h, COLOR_LIGHT_GRAY);
    
    // Display
    buffer_draw_rect_fill(x + 2, y + 1, w - 4, 3, COLOR_WHITE);
    buffer_put_string(x + 4, y + 2, calc_buffer, (COLOR_WHITE << 4) | COLOR_BLACK);
    
    // Buttons
    const char* buttons[] = {
        "7", "8", "9", "/",
        "4", "5", "6", "*",
        "1", "2", "3", "-",
        "C", "0", "=", "+"
    };
    
    int btn_w = (w - 4) / 4;
    int btn_h = 2;
    
    for (int i = 0; i < 16; i++) {
        int bx = x + 2 + (i % 4) * btn_w;
        int by = y + 5 + (i / 4) * (btn_h + 1);
        
        int hover = (mx >= bx && mx < bx + btn_w - 1 && my >= by && my < by + btn_h);
        uint8_t bg = hover ? COLOR_BLUE : COLOR_WHITE;
        uint8_t fg = hover ? COLOR_WHITE : COLOR_BLACK;
        uint8_t attr = (bg << 4) | fg;
        
        buffer_draw_rect_fill(bx, by, btn_w - 1, btn_h, bg);
        buffer_put_string(bx + 1, by, buttons[i], attr);
    }
}

void handle_calc_click(int x, int y, int w, int h, int mx, int my, char* calc_buffer, int* op, int* acc, int* new_num) {
    int btn_w = (w - 4) / 4;
    int btn_h = 2;
    
    const char* buttons[] = {
        "7", "8", "9", "/",
        "4", "5", "6", "*",
        "1", "2", "3", "-",
        "C", "0", "=", "+"
    };
    
    for (int i = 0; i < 16; i++) {
        int bx = x + 2 + (i % 4) * btn_w;
        int by = y + 5 + (i / 4) * (btn_h + 1);
        
        if (mx >= bx && mx < bx + btn_w - 1 && my >= by && my < by + btn_h) {
            char c = buttons[i][0];
            
            if (c >= '0' && c <= '9') {
                if (*new_num) {
                    os_api->strcpy(calc_buffer, "");
                    *new_num = 0;
                }
                int len = os_api->strlen(calc_buffer);
                if (len < 10) {
                    calc_buffer[len] = c;
                    calc_buffer[len+1] = 0;
                }
            } else if (c == 'C') {
                os_api->strcpy(calc_buffer, "0");
                *acc = 0;
                *op = 0;
                *new_num = 1;
            } else if (c == '=') {
                int val = 0;
                // Simple atoi
                char* p = calc_buffer;
                while(*p) { val = val*10 + (*p - '0'); p++; }
                
                if (*op == 1) *acc += val;
                else if (*op == 2) *acc -= val;
                else if (*op == 3) *acc *= val;
                else if (*op == 4) { if(val!=0) *acc /= val; }
                else *acc = val;
                
                *op = 0;
                os_api->int_to_str(*acc, calc_buffer);
                *new_num = 1;
            } else {
                // Op
                int val = 0;
                char* p = calc_buffer;
                while(*p) { val = val*10 + (*p - '0'); p++; }
                *acc = val;
                
                if (c == '+') *op = 1;
                else if (c == '-') *op = 2;
                else if (c == '*') *op = 3;
                else if (c == '/') *op = 4;
                
                *new_num = 1;
            }
            return;
        }
    }
}

void draw_web_app(int x, int y, int w, int h, int mx, int my) {
    buffer_draw_rect_fill(x, y, w, h, COLOR_WHITE);
    buffer_draw_rect_fill(x, y, w, 1, COLOR_LIGHT_GRAY); // Address bar
    buffer_put_string(x + 1, y, "https://iykeos.net", (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    
    buffer_put_string(x + 2, y + 4, "Welcome to the Web!", (COLOR_WHITE << 4) | COLOR_BLACK);
    buffer_put_string(x + 2, y + 6, "No network adapter found.", (COLOR_WHITE << 4) | COLOR_RED);
}

void draw_edit_app(int x, int y, int w, int h, int mx, int my, char* edit_buffer, char* filename) {
    // Background
    buffer_draw_rect_fill(x, y, w, h, COLOR_WHITE);
    
    // Toolbar
    buffer_draw_rect_fill(x, y, w, 1, COLOR_LIGHT_GRAY);
    
    // New Button
    int hover_new = (mx >= x && mx < x + 5 && my == y);
    uint8_t attr_new = hover_new ? ((COLOR_BLUE << 4) | COLOR_WHITE) : ((COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    buffer_put_string(x, y, " New ", attr_new);
    
    // Open Button
    int hover_open = (mx >= x + 6 && mx < x + 12 && my == y);
    uint8_t attr_open = hover_open ? ((COLOR_BLUE << 4) | COLOR_WHITE) : ((COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    buffer_put_string(x + 6, y, " Open ", attr_open);
    
    // Save Button
    int hover_save = (mx >= x + 13 && mx < x + 19 && my == y);
    uint8_t attr_save = hover_save ? ((COLOR_BLUE << 4) | COLOR_WHITE) : ((COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    buffer_put_string(x + 13, y, " Save ", attr_save);
    
    // Filename Display
    char title[32] = "File: ";
    if (filename[0] == 0) os_api->strcat(title, "UNTITLED");
    else os_api->strcat(title, filename);
    buffer_put_string(x + 22, y, title, (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    
    // Text Area
    int cursor_x = 0;
    int cursor_y = 0;
    int max_w = w - 2;
    int max_h = h - 2;
    
    int i = 0;
    while(edit_buffer[i]) {
        char c = edit_buffer[i];
        if (c == '\n') {
            cursor_x = 0;
            cursor_y++;
        } else {
            if (cursor_y < max_h) {
                buffer_put_char(x + 1 + cursor_x, y + 2 + cursor_y, c, (COLOR_WHITE << 4) | COLOR_BLACK);
            }
            cursor_x++;
            if (cursor_x >= max_w) {
                cursor_x = 0;
                cursor_y++;
            }
        }
        i++;
    }
    
    // Cursor
    if (cursor_y < max_h) {
        buffer_put_char(x + 1 + cursor_x, y + 2 + cursor_y, '_', (COLOR_WHITE << 4) | COLOR_BLACK);
    }
}

int handle_edit_click(int x, int y, int w, int h, int mx, int my) {
    // Toolbar
    if (my == y) {
        if (mx >= x && mx < x + 5) return 1; // New
        if (mx >= x + 6 && mx < x + 12) return 2; // Open
        if (mx >= x + 13 && mx < x + 19) return 3; // Save
    }
    return 0;
}

void draw_config_app(int x, int y, int w, int h, int mx, int my) {
    buffer_draw_rect_fill(x, y, w, h, COLOR_LIGHT_GRAY);
    
    buffer_put_string(x + 2, y + 2, "System Configuration", (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    buffer_put_string(x + 2, y + 4, "Resolution: 80x25 Text", (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    buffer_put_string(x + 2, y + 6, "Memory: 128 MB (Simulated)", (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    buffer_put_string(x + 2, y + 8, "OS Version: 0.2", (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
}

void draw_music_app(int x, int y, int w, int h, int mx, int my) {
    buffer_draw_rect_fill(x, y, w, h, COLOR_BLACK);
    
    buffer_put_string(x + 2, y + 2, "Music Player", (COLOR_BLACK << 4) | COLOR_WHITE);
    
    // Play Button
    int hover = (mx >= x + 2 && mx < x + 8 && my == y + 5);
    uint8_t attr = hover ? ((COLOR_GREEN << 4) | COLOR_WHITE) : ((COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    buffer_put_string(x + 2, y + 5, " PLAY ", attr);
    
    buffer_put_string(x + 10, y + 5, "No tracks found.", (COLOR_BLACK << 4) | COLOR_DARK_GRAY);
}

void draw_video_app(int x, int y, int w, int h, int mx, int my) {
    buffer_draw_rect_fill(x, y, w, h, COLOR_BLACK);
    buffer_put_string(x + 2, y + 2, "Video Player", (COLOR_BLACK << 4) | COLOR_WHITE);
    
    // Screen
    buffer_draw_rect_fill(x + 5, y + 5, w - 10, h - 8, COLOR_DARK_GRAY);
    buffer_put_string(x + 15, y + 8, "No Signal", (COLOR_DARK_GRAY << 4) | COLOR_WHITE);
}

void draw_help_app(int x, int y, int w, int h, int mx, int my) {
    buffer_draw_rect_fill(x, y, w, h, COLOR_WHITE);
    buffer_put_string(x + 2, y + 2, "Help & Support", (COLOR_WHITE << 4) | COLOR_BLUE);
    
    buffer_put_string(x + 2, y + 4, "- Click icons to open apps", (COLOR_WHITE << 4) | COLOR_BLACK);
    buffer_put_string(x + 2, y + 6, "- Use [X] to close windows", (COLOR_WHITE << 4) | COLOR_BLACK);
    buffer_put_string(x + 2, y + 8, "- Files app supports scrolling", (COLOR_WHITE << 4) | COLOR_BLACK);
    buffer_put_string(x + 2, y + 10, "- Notes app saves to disk", (COLOR_WHITE << 4) | COLOR_BLACK);
}

void draw_notes_app(int x, int y, int w, int h, int mx, int my, char* notes_buffer) {
    // Background
    buffer_draw_rect_fill(x, y, w, h, COLOR_WHITE);
    
    // Toolbar
    buffer_draw_rect_fill(x, y, w, 1, COLOR_LIGHT_GRAY);
    
    // Save Button
    int hover_save = (mx >= x && mx < x + 6 && my == y);
    uint8_t attr_save = hover_save ? ((COLOR_BLUE << 4) | COLOR_WHITE) : ((COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    buffer_put_string(x, y, " Save ", attr_save);
    
    // Clear Button
    int hover_clear = (mx >= x + 7 && mx < x + 14 && my == y);
    uint8_t attr_clear = hover_clear ? ((COLOR_BLUE << 4) | COLOR_WHITE) : ((COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    buffer_put_string(x + 7, y, " Clear ", attr_clear);
    
    // Text Area
    int cursor_x = 0;
    int cursor_y = 0;
    int max_w = w - 2;
    int max_h = h - 2;
    
    int i = 0;
    while(notes_buffer[i]) {
        char c = notes_buffer[i];
        if (c == '\n') {
            cursor_x = 0;
            cursor_y++;
        } else {
            if (cursor_y < max_h) {
                buffer_put_char(x + 1 + cursor_x, y + 2 + cursor_y, c, (COLOR_WHITE << 4) | COLOR_BLACK);
            }
            cursor_x++;
            if (cursor_x >= max_w) {
                cursor_x = 0;
                cursor_y++;
            }
        }
        i++;
    }
    
    // Cursor
    if (cursor_y < max_h) {
        buffer_put_char(x + 1 + cursor_x, y + 2 + cursor_y, '_', (COLOR_WHITE << 4) | COLOR_BLACK);
    }
}

void handle_notes_click(int x, int y, int w, int h, int mx, int my, char* notes_buffer) {
    // Toolbar
    if (my == y) {
        if (mx >= x && mx < x + 6) { // Save
            os_api->fat16_create_file("NOTES.TXT", 0);
            os_api->fat16_file_save("NOTES.TXT", (uint8_t*)notes_buffer, os_api->strlen(notes_buffer));
        }
        if (mx >= x + 7 && mx < x + 14) { // Clear
            notes_buffer[0] = 0;
        }
    }
}

void draw_gui(int mx, int my, int menu_open, int programs_menu_open, int welcome_open, int app_window_open, char* app_window_title, int scroll_offset, int input_mode, char* input_buffer, char* calc_buffer, char* notes_buffer, char* edit_buffer, char* edit_filename) {
    buffer_draw_rect_fill(0, 0, 80, 1, COLOR_DARK_GRAY);
    uint8_t top_base = (COLOR_DARK_GRAY << 4) | COLOR_WHITE;
    buffer_put_string(2, 0, "IYKEOS", top_base);
    buffer_put_string(12, 0, "|", top_base);
    
    uint8_t attr;
    attr = is_hover(mx, my, 15, 0, 4, 1) ? ((COLOR_LIGHT_GRAY << 4) | COLOR_BLACK) : top_base;
    buffer_put_string(15, 0, "File", attr);
    attr = is_hover(mx, my, 22, 0, 4, 1) ? ((COLOR_LIGHT_GRAY << 4) | COLOR_BLACK) : top_base;
    buffer_put_string(22, 0, "View", attr);
    attr = is_hover(mx, my, 29, 0, 7, 1) ? ((COLOR_LIGHT_GRAY << 4) | COLOR_BLACK) : top_base;
    buffer_put_string(29, 0, "Options", attr);
    attr = is_hover(mx, my, 39, 0, 4, 1) ? ((COLOR_LIGHT_GRAY << 4) | COLOR_BLACK) : top_base;
    buffer_put_string(39, 0, "Help", attr);

    // 2. Desktop Icons
    // Column 1
    draw_icon(2, 3, "[T]", "Shell", COLOR_DARK_GRAY, mx, my);
    draw_icon(2, 8, "[F]", "Files", COLOR_YELLOW, mx, my);
    draw_icon(2, 13, "[B]", "Web", COLOR_LIGHT_BLUE, mx, my);
    draw_icon(2, 18, "[E]", "Edit", COLOR_RED, mx, my);
    
    // Column 2
    draw_icon(12, 3, "[S]", "Config", COLOR_LIGHT_GRAY, mx, my);
    draw_icon(12, 8, "[M]", "Music", COLOR_MAGENTA, mx, my);
    draw_icon(12, 13, "[V]", "Video", COLOR_GREEN, mx, my);
    draw_icon(12, 18, "[C]", "Calc", COLOR_CYAN, mx, my);
    
    // Column 3
    draw_icon(22, 3, "[N]", "Notes", COLOR_WHITE, mx, my);
    draw_icon(22, 8, "[H]", "Help", COLOR_LIGHT_GREEN, mx, my);


    // 3. Window (Welcome)
    if (welcome_open) {
        buffer_draw_rect_fill(21, 6, 40, 10, COLOR_BLACK); // Shadow
        buffer_draw_window(20, 5, 40, 10, "Welcome", COLOR_LIGHT_GRAY);
        uint8_t win_attr = (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK;
        buffer_put_string(22, 7, "Welcome to IYKEOS v0.2", win_attr);
        buffer_put_string(22, 9, "A simple, powerful OS.", win_attr);
        buffer_put_string(22, 11, "Explore the menu above.", win_attr);
        
        // Close Button
        int hover = is_hover(mx, my, 56, 5, 3, 1);
        attr = hover ? ((COLOR_RED << 4) | COLOR_WHITE) : ((COLOR_LIGHT_GRAY << 4) | COLOR_RED);
        buffer_put_string(56, 5, "[X]", attr);
    }
    
    // 3.5 App Window (Full Screen)
    if (app_window_open) {
        // Full screen area: x=0, y=1, w=80, h=23 (leaving top bar and taskbar)
        int wx = 0;
        int wy = 1;
        int ww = 80;
        int wh = 23;
        
        buffer_draw_window(wx, wy, ww, wh, app_window_title, COLOR_LIGHT_GRAY);
        
        // Close Button (Right side for full screen feel)
        int hover = is_hover(mx, my, wx + ww - 4, wy, 3, 1);
        attr = hover ? ((COLOR_RED << 4) | COLOR_WHITE) : ((COLOR_LIGHT_GRAY << 4) | COLOR_RED);
        buffer_put_string(wx + ww - 4, wy, "[X]", attr);
        
        // Content
        if (app_window_open == APP_FILES) {
            // Content area inside window
            draw_files_app(wx + 1, wy + 2, ww - 2, wh - 3, mx, my, scroll_offset);
        } else if (app_window_open == APP_CALC) {
            draw_calc_app(wx + 20, wy + 2, 40, 18, mx, my, calc_buffer);
        } else if (app_window_open == APP_NOTES) {
            draw_notes_app(wx + 1, wy + 2, ww - 2, wh - 3, mx, my, notes_buffer);
        } else if (app_window_open == APP_WEB) {
            draw_web_app(wx + 1, wy + 2, ww - 2, wh - 3, mx, my);
        } else if (app_window_open == APP_EDIT) {
            draw_edit_app(wx + 1, wy + 2, ww - 2, wh - 3, mx, my, edit_buffer, edit_filename);
        } else if (app_window_open == APP_CONFIG) {
            draw_config_app(wx + 1, wy + 2, ww - 2, wh - 3, mx, my);
        } else if (app_window_open == APP_MUSIC) {
            draw_music_app(wx + 1, wy + 2, ww - 2, wh - 3, mx, my);
        } else if (app_window_open == APP_VIDEO) {
            draw_video_app(wx + 1, wy + 2, ww - 2, wh - 3, mx, my);
        } else if (app_window_open == APP_HELP) {
            draw_help_app(wx + 1, wy + 2, ww - 2, wh - 3, mx, my);
        } else {
            uint8_t content_attr = (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK;
            buffer_put_string(wx + 2, wy + 2, "Running: ", content_attr);
            buffer_put_string(wx + 11, wy + 2, app_window_title, content_attr);
        }
    }
    
    // Input Modal
    if (input_mode) {
        int mw = 30;
        int mh = 8;
        int mx_pos = (80 - mw) / 2;
        int my_pos = (25 - mh) / 2;
        
        // Shadow
        buffer_draw_rect_fill(mx_pos + 1, my_pos + 1, mw, mh, COLOR_BLACK);
        // Window
        buffer_draw_rect_fill(mx_pos, my_pos, mw, mh, COLOR_LIGHT_GRAY);
        // Title Bar
        buffer_draw_rect_fill(mx_pos, my_pos, mw, 1, COLOR_BLUE);
        
        char* title = "Input";
        if (input_mode == 1) title = "New File Name:";
        if (input_mode == 2) title = "New Folder Name:";
        if (input_mode == 3) title = "Open File:";
        if (input_mode == 4) title = "Save File As:";
        
        buffer_put_string(mx_pos + 1, my_pos, title, (COLOR_BLUE << 4) | COLOR_WHITE);
        
        // Input Box
        buffer_draw_rect_fill(mx_pos + 2, my_pos + 3, mw - 4, 1, COLOR_WHITE);
        buffer_put_string(mx_pos + 2, my_pos + 3, input_buffer, (COLOR_WHITE << 4) | COLOR_BLACK);
        
        buffer_put_string(mx_pos + 2, my_pos + 6, "[Enter] OK  [Esc] Cancel", (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    }

    // 4. Taskbar
    buffer_draw_rect_fill(0, 24, 80, 1, COLOR_LIGHT_GRAY);
    
    // Start Button
    int hover = is_hover(mx, my, 0, 24, 11, 1);
    attr = hover ? ((COLOR_LIGHT_GREEN << 4) | COLOR_WHITE) : ((COLOR_GREEN << 4) | COLOR_WHITE);
    buffer_put_string(0, 24, " [ START ] ", attr);
    
    if (welcome_open) {
        buffer_put_string(12, 24, "|  Welcome  |", (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    }

    // 5. Menu (if open)
    if (menu_open) {
        buffer_draw_window(0, 10, 15, 11, "Menu", COLOR_LIGHT_GRAY);
        uint8_t win_attr = (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK;

        // Items at y=12, 14, 16, 18, 20 (moved up so Start button is visible)
        // Programs
        attr = (is_hover(mx, my, 1, 12, 13, 1)) ? ((COLOR_BLUE << 4) | COLOR_WHITE) : win_attr;
        buffer_put_string(1, 12, "Programs", attr);

        attr = (is_hover(mx, my, 1, 14, 13, 1)) ? ((COLOR_BLUE << 4) | COLOR_WHITE) : win_attr;
        buffer_put_string(1, 14, "Settings", attr);

        attr = (is_hover(mx, my, 1, 16, 13, 1)) ? ((COLOR_BLUE << 4) | COLOR_WHITE) : win_attr;
        buffer_put_string(1, 16, "Run...", attr);

        attr = (is_hover(mx, my, 1, 18, 13, 1)) ? ((COLOR_BLUE << 4) | COLOR_WHITE) : win_attr;
        buffer_put_string(1, 18, "Shell", attr);

        attr = (is_hover(mx, my, 1, 20, 13, 1)) ? ((COLOR_BLUE << 4) | COLOR_WHITE) : win_attr;
        buffer_put_string(1, 20, "Shutdown", attr);
    }
    
    // Programs Submenu (if open)
    if (programs_menu_open) {
        int pm_x = 15;
        int pm_y = 14;
        int pm_w = 20;
        int pm_h = 13;
        buffer_draw_window(pm_x, pm_y, pm_w, pm_h, "Programs", COLOR_LIGHT_GRAY);
        uint8_t pm_attr = (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK;
        
        // List all programs at y=16, 17, 18, 19, 20, 21, 22, 23, 24, 25
        attr = (is_hover(mx, my, pm_x + 1, pm_y + 2, pm_w - 2, 1)) ? ((COLOR_BLUE << 4) | COLOR_WHITE) : pm_attr;
        buffer_put_string(pm_x + 1, pm_y + 2, "Shell", attr);
        
        attr = (is_hover(mx, my, pm_x + 1, pm_y + 3, pm_w - 2, 1)) ? ((COLOR_BLUE << 4) | COLOR_WHITE) : pm_attr;
        buffer_put_string(pm_x + 1, pm_y + 3, "Files", attr);
        
        attr = (is_hover(mx, my, pm_x + 1, pm_y + 4, pm_w - 2, 1)) ? ((COLOR_BLUE << 4) | COLOR_WHITE) : pm_attr;
        buffer_put_string(pm_x + 1, pm_y + 4, "Web", attr);
        
        attr = (is_hover(mx, my, pm_x + 1, pm_y + 5, pm_w - 2, 1)) ? ((COLOR_BLUE << 4) | COLOR_WHITE) : pm_attr;
        buffer_put_string(pm_x + 1, pm_y + 5, "Edit", attr);
        
        attr = (is_hover(mx, my, pm_x + 1, pm_y + 6, pm_w - 2, 1)) ? ((COLOR_BLUE << 4) | COLOR_WHITE) : pm_attr;
        buffer_put_string(pm_x + 1, pm_y + 6, "Config", attr);
        
        attr = (is_hover(mx, my, pm_x + 1, pm_y + 7, pm_w - 2, 1)) ? ((COLOR_BLUE << 4) | COLOR_WHITE) : pm_attr;
        buffer_put_string(pm_x + 1, pm_y + 7, "Music", attr);
        
        attr = (is_hover(mx, my, pm_x + 1, pm_y + 8, pm_w - 2, 1)) ? ((COLOR_BLUE << 4) | COLOR_WHITE) : pm_attr;
        buffer_put_string(pm_x + 1, pm_y + 8, "Video", attr);
        
        attr = (is_hover(mx, my, pm_x + 1, pm_y + 9, pm_w - 2, 1)) ? ((COLOR_BLUE << 4) | COLOR_WHITE) : pm_attr;
        buffer_put_string(pm_x + 1, pm_y + 9, "Calc", attr);
        
        attr = (is_hover(mx, my, pm_x + 1, pm_y + 10, pm_w - 2, 1)) ? ((COLOR_BLUE << 4) | COLOR_WHITE) : pm_attr;
        buffer_put_string(pm_x + 1, pm_y + 10, "Notes", attr);
        
        attr = (is_hover(mx, my, pm_x + 1, pm_y + 11, pm_w - 2, 1)) ? ((COLOR_BLUE << 4) | COLOR_WHITE) : pm_attr;
        buffer_put_string(pm_x + 1, pm_y + 11, "Help", attr);
    }
}

void window() {
    os_api->disable_cursor();
    os_api->graphics_init();
    os_api->graphics_loading_screen();
    os_api->mouse_init();
    os_api->keyboard_init(); 
    
 
    int mx = 40, my = 12;
    int menu_open = 0;
    int programs_menu_open = 0;
    int welcome_open = 1;
    int app_window_open = 0;
    char app_window_title[32] = "";
    
    int files_scroll = 0;
    int input_mode = 0; // 0=none, 1=file, 2=dir
    char input_buffer[32] = "";
    int input_len = 0;
    
    // Calculator State
    char calc_buffer[16] = "0";
    int calc_op = 0;
    int calc_acc = 0;
    int calc_new_num = 1;
    
    // Notes State
    static char notes_buffer[1024]; // 1KB buffer for notes
    notes_buffer[0] = 0;
    int notes_len = 0;
    
    // Edit State
    static char edit_buffer[4096];
    static char edit_filename[12];
    edit_buffer[0] = 0;
    edit_filename[0] = 0;
    int edit_len = 0;
    
    while (1) {
        // 1. Clear Buffer
        buffer_clear(COLOR_BLUE);
        
        // 2. Draw GUI to Buffer
        draw_gui(mx, my, menu_open, programs_menu_open, welcome_open, app_window_open, app_window_title, files_scroll, input_mode, input_buffer, calc_buffer, notes_buffer, edit_buffer, edit_filename);
        
        // 3. Update Clock to Buffer
        uint8_t h, m, s;
        os_api->get_rtc_time(&h, &m, &s);
        char time_str[10];
        time_str[0] = (h / 10) + '0';
        time_str[1] = (h % 10) + '0';
        time_str[2] = ':';
        time_str[3] = (m / 10) + '0';
        time_str[4] = (m % 10) + '0';
        time_str[5] = 0;
        buffer_put_string(70, 24, time_str, (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);

        // 4. Draw Mouse to Buffer
        uint16_t mouse_char = 30; // Up Arrow
        if (mx >= 0 && mx < 80 && my >= 0 && my < 25) {
             gui_buffer[my * 80 + mx] = mouse_char | (COLOR_RED << 12) | (COLOR_WHITE << 8);
        }

        // // 5. Present (Flip) Buffer to Screen
        buffer_present();

        // // 6. Input Handling
        
        // // Mouse Input
        int left_click = 0;
        int right_click = 0;
        // Poll mouse multiple times per frame to catch movement
        for(int i=0; i<10; i++) os_api->mouse_handler(&mx, &my, &left_click, &right_click);
        
        char c =os_api->keyboard_getchar();
        
        // // Combine mouse click with keyboard 'click'
        if (left_click) {
            c = '\n'; // Treat left click as Enter/Select
            // Debounce simple
           os_api->busy_delay(100); 
        }

        if (c == 0 && !left_click) {
            continue;
        }
        
        // // Input Mode Handling
        if (input_mode) {
            if (c == '\n') {
                // Create
                if (input_len > 0) {
                    if (input_mode == 1) { // File
                         os_api->fat16_create_file(input_buffer, 0);
                    } else if (input_mode == 2) { // Dir
                        os_api->fat16_mkdir(input_buffer);
                    } else if (input_mode == 3) { // Edit Open
                        if (os_api->fat16_file_exists(input_buffer)) {
                            os_api->strcpy(edit_filename, input_buffer);
                            // Load file
                            os_api->memset(edit_buffer, 0, 4096);
                            os_api->fat16_file_load(edit_filename, (uint8_t*)edit_buffer);
                            edit_len = os_api->strlen(edit_buffer);
                        }
                    } else if (input_mode == 4) { // Edit Save As
                        os_api->strcpy(edit_filename, input_buffer);
                        os_api->fat16_create_file(edit_filename, 0); // Ensure exists
                        os_api->fat16_file_save(edit_filename, (uint8_t*)edit_buffer, edit_len);
                    }
                }
                input_mode = 0;
                input_len = 0;
                input_buffer[0] = 0;
            } else if (c == 27) { // ESC (approx)
                input_mode = 0;
            } else if (c == '\b') {
                if (input_len > 0) {
                    input_len--;
                    input_buffer[input_len] = 0;
                }
            } else if (c >= 32 && c <= 126 && input_len < 12) {
                input_buffer[input_len++] = c;
                input_buffer[input_len] = 0;
            }
            continue; // Skip other input
        }
        
        if (input_mode) {
             // ... (existing input mode logic)
        } else if (app_window_open == APP_NOTES) {
             // Notes Input
             if (c == '\b') {
                 if (notes_len > 0) {
                     notes_len--;
                     notes_buffer[notes_len] = 0;
                 }
             } else if (c == '\n') {
                 if (notes_len < 1023) {
                     notes_buffer[notes_len++] = '\n';
                     notes_buffer[notes_len] = 0;
                 }
             } else if (c >= 32 && c <= 126) {
                 if (notes_len < 1023) {
                     notes_buffer[notes_len++] = c;
                     notes_buffer[notes_len] = 0;
                 }
             }
        } else if (app_window_open == APP_EDIT) {
             // Edit Input
             if (c == '\b') {
                 if (edit_len > 0) {
                     edit_len--;
                     edit_buffer[edit_len] = 0;
                 }
             } else if (c == '\n') {
                 if (edit_len < 4095) {
                     edit_buffer[edit_len++] = '\n';
                     edit_buffer[edit_len] = 0;
                 }
             } else if (c >= 32 && c <= 126) {
                 if (edit_len < 4095) {
                     edit_buffer[edit_len++] = c;
                     edit_buffer[edit_len] = 0;
                 }
             }
        }
        
        if (c == 'q') break;
        
        // Move Mouse
        if (c == KEY_UP && my > 0) my--;
        if (c == KEY_DOWN && my < 24) my++;
        if (c == KEY_LEFT && mx > 0) mx--;
        if (c == KEY_RIGHT && mx < 79) mx++;
        
        // Handle Click
        if (c == '\n' || c == ' ') {
            if (my == 24 && mx < 11) {
                menu_open = !menu_open;
                programs_menu_open = 0; // Close programs submenu when toggling menu
            }
            // Close Window Click
            if (welcome_open && is_hover(mx, my, 56, 5, 3, 1)) {
                welcome_open = 0;
            }
            
            // App Window Close Click (Top Right)
            if (app_window_open && is_hover(mx, my, 76, 1, 3, 1)) {
                app_window_open = APP_NONE;
                files_scroll = 0; // Reset scroll
            }
            
            // App Content Click
            if (app_window_open == APP_FILES) {
                // Match the coordinates used in draw_gui
                int action = handle_files_click(1, 3, 78, 20, mx, my, files_scroll);
                if (action == 1) {
                    files_scroll = 0; // Reset scroll on dir change
                } else if (action == 2) { // New File
                    input_mode = 1;
                    input_len = 0;
                    input_buffer[0] = 0;
                } else if (action == 3) { // New Dir
                    input_mode = 2;
                    input_len = 0;
                    input_buffer[0] = 0;
                } else if (action == 4) { // Scroll Up
                    if (files_scroll > 0) files_scroll--;
                } else if (action == 5) { // Scroll Down
                    files_scroll++;
                }
            } else if (app_window_open == APP_CALC) {
                handle_calc_click(20, 3, 40, 18, mx, my, calc_buffer, &calc_op, &calc_acc, &calc_new_num);
            } else if (app_window_open == APP_NOTES) {
                handle_notes_click(1, 3, 78, 20, mx, my, notes_buffer);
                // Update len if cleared
                if (notes_buffer[0] == 0) notes_len = 0;
            } else if (app_window_open == APP_EDIT) {
                int action = handle_edit_click(1, 3, 78, 20, mx, my);
                if (action == 1) { // New
                    edit_buffer[0] = 0;
                    edit_len = 0;
                    edit_filename[0] = 0;
                } else if (action == 2) { // Open
                    input_mode = 3;
                    input_len = 0;
                    input_buffer[0] = 0;
                } else if (action == 3) { // Save
                    if (edit_filename[0] == 0) {
                        input_mode = 4; // Save As
                        input_len = 0;
                        input_buffer[0] = 0;
                    } else {
                        os_api->fat16_create_file(edit_filename, 0); // Ensure exists
                        os_api->fat16_file_save(edit_filename, (uint8_t*)edit_buffer, edit_len);
                    }
                }
            }

            // Icon Clicks - ONLY if no app window is open
            if (!app_window_open) {
                // Column 1
                if (is_hover(mx, my, 2, 3, 6, 2)) { 
                    
                   os_api->start_shell();
                }
                if (is_hover(mx, my, 2, 8, 6, 2)) { app_window_open = APP_FILES; os_api->strcpy(app_window_title, "Files"); }
                if (is_hover(mx, my, 2, 13, 6, 2)) { app_window_open = APP_WEB; os_api->strcpy(app_window_title, "Web"); }
                if (is_hover(mx, my, 2, 18, 6, 2)) { app_window_open = APP_EDIT; os_api->strcpy(app_window_title, "Edit"); }
                
                // Column 2
                if (is_hover(mx, my, 12, 3, 6, 2)) { app_window_open = APP_CONFIG; os_api->strcpy(app_window_title, "Config"); }
                if (is_hover(mx, my, 12, 8, 6, 2)) { app_window_open = APP_MUSIC; os_api->strcpy(app_window_title, "Music"); }
                if (is_hover(mx, my, 12, 13, 6, 2)) { app_window_open = APP_VIDEO;os_api-> strcpy(app_window_title, "Video"); }
                if (is_hover(mx, my, 12, 18, 6, 2)) { app_window_open = APP_CALC; os_api->strcpy(app_window_title, "Calc"); }
                
                // Column 3
                if (is_hover(mx, my, 22, 3, 6, 2)) { app_window_open = APP_NOTES;os_api->strcpy(app_window_title, "Notes"); }
                if (is_hover(mx, my, 22, 8, 6, 2)) { app_window_open = APP_HELP; os_api->strcpy(app_window_title, "Help"); }
            }
            
            // Programs Menu Click - toggle submenu
            if (menu_open && is_hover(mx, my, 1, 12, 13, 1)) {
                programs_menu_open = !programs_menu_open;
            }
            
            // Shell Click
            if (menu_open && is_hover(mx, my, 1, 18, 13, 1)) {
                os_api->start_shell();
            }

            // Shutdown Click
            if (menu_open && is_hover(mx, my, 1, 20, 13, 1)) {
                break;
            }
            
            // Programs Submenu Clicks
            if (programs_menu_open) {
                int pm_x = 15;
                int pm_y = 14;
                
                // Shell
                if (is_hover(mx, my, pm_x + 1, pm_y + 2, 18, 1)) {
                    os_api->start_shell();
                    programs_menu_open = 0;
                    menu_open = 0;
                }
                // Files
                else if (is_hover(mx, my, pm_x + 1, pm_y + 3, 18, 1)) {
                    app_window_open = APP_FILES;
                    os_api->strcpy(app_window_title, "Files");
                    programs_menu_open = 0;
                    menu_open = 0;
                }
                // Web
                else if (is_hover(mx, my, pm_x + 1, pm_y + 4, 18, 1)) {
                    app_window_open = APP_WEB;
                    os_api->strcpy(app_window_title, "Web");
                    programs_menu_open = 0;
                    menu_open = 0;
                }
                // Edit
                else if (is_hover(mx, my, pm_x + 1, pm_y + 5, 18, 1)) {
                    app_window_open = APP_EDIT;
                    os_api->strcpy(app_window_title, "Edit");
                    programs_menu_open = 0;
                    menu_open = 0;
                }
                // Config
                else if (is_hover(mx, my, pm_x + 1, pm_y + 6, 18, 1)) {
                    app_window_open = APP_CONFIG;
                    os_api->strcpy(app_window_title, "Config");
                    programs_menu_open = 0;
                    menu_open = 0;
                }
                // Music
                else if (is_hover(mx, my, pm_x + 1, pm_y + 7, 18, 1)) {
                    app_window_open = APP_MUSIC;
                    os_api->strcpy(app_window_title, "Music");
                    programs_menu_open = 0;
                    menu_open = 0;
                }
                // Video
                else if (is_hover(mx, my, pm_x + 1, pm_y + 8, 18, 1)) {
                    app_window_open = APP_VIDEO;
                    os_api->strcpy(app_window_title, "Video");
                    programs_menu_open = 0;
                    menu_open = 0;
                }
                // Calc
                else if (is_hover(mx, my, pm_x + 1, pm_y + 9, 18, 1)) {
                    app_window_open = APP_CALC;
                    os_api->strcpy(app_window_title, "Calc");
                    programs_menu_open = 0;
                    menu_open = 0;
                }
                // Notes
                else if (is_hover(mx, my, pm_x + 1, pm_y + 10, 18, 1)) {
                    app_window_open = APP_NOTES;
                    os_api->strcpy(app_window_title, "Notes");
                    programs_menu_open = 0;
                    menu_open = 0;
                }
                // Help
                else if (is_hover(mx, my, pm_x + 1, pm_y + 11, 18, 1)) {
                    app_window_open = APP_HELP;
                    os_api->strcpy(app_window_title, "Help");
                    programs_menu_open = 0;
                    menu_open = 0;
                }
            }
        }
    }
}


void start(){
    init_api();
    os_api->save_vga(0);
    os_api->screen_clear_shell();
}


void main(void){
    start();
     window();
    os_api->load_vga(0);
}