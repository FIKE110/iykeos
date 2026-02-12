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
    void (*graphics_put_pixel)(int x, int y, uint8_t color)

} os_api_t;



os_api_t* os_api;

void init_api(void){
    os_api=(os_api_t*) OS_API_ADDR;
}


uint16_t gui_buffer[80 * 25];

void buffer_put_char(int x, int y, char c, uint8_t attr) {
    if (x >= 0 && x < 80 && y >= 0 && y < 25) {
        if(c=='\0'){
            return;
        }
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
    uint16_t val = (uint16_t)' ' | ((uint16_t)color << 12);
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
    buffer_put_string(x + 1, y, art, (rect_col << 4) | text_col);

    uint8_t label_col = hover ? COLOR_YELLOW : COLOR_WHITE;
    int max_width = 9;
    int line = 0;
    int i = 0;
    while (label[i] != '\0') {
        char line_buf[10];
        int j;
        for (j = 0; j < max_width && label[i + j] != '\0'; j++) {
            line_buf[j] = label[i + j];
        }
        line_buf[j] = '\0';
        buffer_put_string(x, y + 2 + line, line_buf, (COLOR_BLUE << 4) | label_col);
        i += j;
        line++;
    }
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


int get_files(char* dir, char* buffer) {
    os_api->fat16_chdir(dir);

    int count = os_api->get_file_list(buffer);

    char* p = buffer;
    int skipped = 0;

    while (*p != '\0' && skipped < 2) {
        if (*p == '|') {
            skipped++;
        }
        p++;
    }
    if (skipped == 2) {
        int i = 0;
        while (*p != '\0') {
            buffer[i++] = *p++;
        }
        buffer[i] = '\0';
        count -= 2;
    }

    os_api->fat16_chdir("..");
    return count > 0 ? count : 0;
}



void draw_gui(int mx, int my, int menu_open, int programs_menu_open, int welcome_open, int app_window_open) {
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

    char buffer[1024];
    int count = get_files("DESKTOP", buffer);

    uint8_t win_attr = (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK;

    char filename[13];
    int file_idx = 0;
    int name_pos = 0;

    int start_x = 2;
    int start_y = 3;
    int col_width = 10;
    int row_height = 5;
    int max_rows = 4;

    for (int i = 0; buffer[i] != '\0'; i++) {

        if (buffer[i] == '|') {
            filename[name_pos] = '\0';

            int col = file_idx / max_rows;
            int row = file_idx % max_rows;

            int x = start_x + (col * col_width);
            int y = start_y + (row * row_height);

            char icon[4];
            icon[0] = '[';
            icon[1] = filename[0];
            icon[2] = ']';
            icon[3] = '\0';

            draw_icon(x, y, icon, filename, COLOR_DARK_GRAY, mx, my);

            file_idx++;
            name_pos = 0;

            if (file_idx >= count)
                break;

        } else {
            if (name_pos < 12) {
                filename[name_pos++] = buffer[i];
            }
        }
    }

    if (name_pos > 0 && file_idx < count) {
        filename[name_pos] = '\0';

        int col = file_idx / max_rows;
        int row = file_idx % max_rows;

        int x = start_x + (col * col_width);
        int y = start_y + (row * row_height);

        char icon[4];
        icon[0] = '[';
        icon[1] = filename[0];
        icon[2] = ']';
        icon[3] = '\0';

        draw_icon(x, y, icon, filename, COLOR_DARK_GRAY, mx, my);
    }

    if (welcome_open) {
        buffer_draw_rect_fill(21, 6, 40, 10, COLOR_BLACK);
        buffer_draw_window(20, 5, 40, 10, "Welcome", COLOR_LIGHT_GRAY);
        uint8_t win_attr = (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK;
        buffer_put_string(22, 7, "Welcome to IYKEOS v0.2", win_attr);
        buffer_put_string(22, 9, "A simple, powerful OS.", win_attr);
        buffer_put_string(22, 11, "Explore the menu above.", win_attr);
        
        int hover = is_hover(mx, my, 56, 5, 3, 1);
        attr = hover ? ((COLOR_RED << 4) | COLOR_WHITE) : ((COLOR_LIGHT_GRAY << 4) | COLOR_RED);
        buffer_put_string(56, 5, "[X]", attr);
    }

    buffer_draw_rect_fill(0, 24, 80, 1, COLOR_LIGHT_GRAY);
    
    int hover = is_hover(mx, my, 0, 24, 11, 1);
    attr = hover ? ((COLOR_LIGHT_GREEN << 4) | COLOR_WHITE) : ((COLOR_GREEN << 4) | COLOR_WHITE);
    buffer_put_string(0, 24, " [ START ] ", attr);
    
    if (welcome_open) {
        buffer_put_string(12, 24, "|  Welcome  |", (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK);
    }

    if (menu_open) {
        buffer_draw_window(0, 10, 15, 11, "Menu", COLOR_LIGHT_GRAY);
        uint8_t win_attr = (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK;

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
        int pm_y = 10;
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
    os_api->graphics_loading_screen();  // Show loading screen after graphics init
    os_api->mouse_init();
    os_api->keyboard_init();  // Initialize keyboard when GUI starts
    
    int mx = 40, my = 12;
    int menu_open = 0;
    int programs_menu_open = 0;
    int welcome_open = 1;
    int app_window_open = 0;
    char app_window_title[32] = "";
    
    int files_scroll = 0;
    int input_mode = 0;
    char input_buffer[32] = "";
    int input_len = 0;
    
    char calc_buffer[16] = "0";
    int calc_op = 0;
    int calc_acc = 0;
    int calc_new_num = 1;
    
    static char notes_buffer[1024];
    notes_buffer[0] = 0;
    int notes_len = 0;
    
    static char edit_buffer[4096];
    static char edit_filename[12];
    edit_buffer[0] = 0;
    edit_filename[0] = 0;
    int edit_len = 0;
    
    while (1) {
        buffer_clear(COLOR_BLUE);
        draw_gui(mx, my, menu_open, programs_menu_open, welcome_open, app_window_open);

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
        uint16_t mouse_char = 30;
        if (mx >= 0 && mx < 80 && my >= 0 && my < 25) {
             gui_buffer[my * 80 + mx] = mouse_char | (COLOR_RED << 12) | (COLOR_WHITE << 8);
        }

        buffer_present();

        int left_click = 0;
        int right_click = 0;
        for(int i=0; i<10; i++) os_api->mouse_handler(&mx, &my, &left_click, &right_click);
        
        char c =os_api->keyboard_getchar();
        
        if (left_click) {
            c = '\n';
           os_api->busy_delay(100); 
        }

        if (c == 0 && !left_click) {
            continue;
        }
    
        if (c == KEY_UP && my > 0) my--;
        if (c == KEY_DOWN && my < 24) my++;
        if (c == KEY_LEFT && mx > 0) mx--;
        if (c == KEY_RIGHT && mx < 79) mx++;
        
        if (c == '\n' || c == ' ') {
            if (my == 24 && mx < 11) {
                menu_open = !menu_open;
                programs_menu_open = 0; // Close programs submenu when toggling menu
            }
            if (welcome_open && is_hover(mx, my, 56, 5, 3, 1)) {
                welcome_open = 0;
            }
            
            if (!app_window_open) {
               
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
                // break;
            }
            
            // Programs Submenu Clicks
            if (programs_menu_open) {
                int pm_x = 15;
                int pm_y = 10;
                
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
    os_api->mouse_init();
     window();
    os_api->load_vga(0);
}