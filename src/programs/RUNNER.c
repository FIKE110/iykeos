#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

#define OS_API_ADDR 0x5F0F0

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

/* Window dimensions */
#define APP_X       1
#define APP_Y       2
#define APP_WIDTH   78
#define APP_HEIGHT  20

/* Toolbar button positions */
#define BTN_REFRESH_X   0
#define BTN_REFRESH_W   9
#define BTN_RUN_X       11
#define BTN_RUN_W       7
#define BTN_SCROLL_UP_X(APP_W)  ((APP_W) - 6)
#define BTN_SCROLL_UP_W         3
#define BTN_SCROLL_DOWN_X(APP_W) ((APP_W) - 3)
#define BTN_SCROLL_DOWN_W       3

/* File list display */
#define HEADER_ROW      1
#define FILE_START_ROW  2
#define MAX_VISIBLE_FILES(APP_H) ((APP_H) - 3)

/* Input limits */
#define MAX_FILENAME    32

/* Colors - Modern Green Theme */
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

#define ATTR(fg, bg)     ((bg) << 4 | (fg))

/* Desktop */
#define DESKTOP_COLOR    COLOR_BLACK

/* Window chrome */
#define WINDOW_BG        COLOR_WHITE
#define WINDOW_BORDER    COLOR_GREEN
#define TITLE_BAR_BG     COLOR_GREEN
#define TITLE_BAR_FG     COLOR_WHITE

/* Toolbar */
#define TOOLBAR_BG       COLOR_LGRAY
#define TOOLBAR_FG       COLOR_BLACK
#define TOOLBAR_HOVER_BG COLOR_GREEN
#define TOOLBAR_HOVER_FG COLOR_WHITE

/* File list */
#define FILE_LIST_BG         COLOR_WHITE
#define FILE_LIST_FG         COLOR_BLACK
#define FILE_SELECTED_BG     COLOR_GREEN
#define FILE_SELECTED_FG     COLOR_WHITE
#define FILE_HOVERED_BG      COLOR_LGREEN
#define FILE_HOVERED_FG      COLOR_BLACK
#define FILE_HEADER_BG       COLOR_DGRAY
#define FILE_HEADER_FG       COLOR_WHITE

/* Status bar */
#define STATUS_BG        COLOR_DGRAY
#define STATUS_FG        COLOR_WHITE

/* Scrollbar */
#define SCROLL_BG        COLOR_LGRAY
#define SCROLL_FG        COLOR_DGRAY

/* Attribute macros */
#define ATTR_NORMAL      ATTR(FILE_LIST_FG, FILE_LIST_BG)
#define ATTR_SELECTED    ATTR(FILE_SELECTED_FG, FILE_SELECTED_BG)
#define ATTR_HOVERED     ATTR(FILE_HOVERED_FG, FILE_HOVERED_BG)
#define ATTR_TOOLBAR     ATTR(TOOLBAR_FG, TOOLBAR_BG)
#define ATTR_TOOLBAR_HOV ATTR(TOOLBAR_HOVER_FG, TOOLBAR_HOVER_BG)
#define ATTR_HEADER      ATTR(FILE_HEADER_FG, FILE_HEADER_BG)
#define ATTR_TITLE       ATTR(TITLE_BAR_FG, TITLE_BAR_BG)
#define ATTR_STATUS      ATTR(STATUS_FG, STATUS_BG)

/* File type colors */
#define ATTR_BIN         ATTR(COLOR_GREEN, FILE_LIST_BG)
#define ATTR_BIN_SEL     ATTR(COLOR_WHITE, FILE_SELECTED_BG)
#define ATTR_BAS         ATTR(COLOR_CYAN, FILE_LIST_BG)
#define ATTR_BAS_SEL     ATTR(COLOR_WHITE, FILE_SELECTED_BG)
#define ATTR_IKE         ATTR(COLOR_LBLUE, FILE_LIST_BG)
#define ATTR_IKE_SEL     ATTR(COLOR_WHITE, FILE_SELECTED_BG)
#define ATTR_BGS         ATTR(COLOR_MAGENTA, FILE_LIST_BG)
#define ATTR_BGS_SEL     ATTR(COLOR_WHITE, FILE_SELECTED_BG)

/* Keys */
#define KEY_UP          0x48
#define KEY_DOWN        0x50
#define KEY_LEFT        0x4B
#define KEY_RIGHT       0x4D
#define KEY_ENTER       0x1C
#define KEY_ESC         0x01

/* Mouse */
#define MOUSE_CHAR      30
#define MOUSE_COLOR     COLOR_RED

/* Close button */
#define CLOSE_BTN_X(APP_X, APP_W)  ((APP_X) + (APP_W) - 4)
#define CLOSE_BTN_Y(APP_Y)         ((APP_Y) - 1)

/* Supported extensions */
#define EXT_BIN         0
#define EXT_BAS         1
#define EXT_IKE         2
#define EXT_BGS         3
#define EXT_UNKNOWN     4

/* ============================================================================
 * TYPES
 * ============================================================================ */

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
    void (*vgraphics_init)(void);
    void (*vgraphics_clear)(uint8_t color);
    void (*vgraphics_repaint)(void);
    void (*vgraphics_put_char)(int x, int y, char c, uint8_t color);
    void (*vgraphics_put_string)(int x, int y, const char* str, uint8_t color);
    void (*vgraphics_draw_box)(int x, int y, int w, int h, uint8_t color);
    void (*vgraphics_draw_window)(int x, int y, int w, int h, const char* title, uint8_t color);
    void (*vgraphics_draw_rect_fill)(int x, int y, int w, int h, uint8_t color);
    int (*run_with_status)(char* filename);
    uint32_t (*fat16_file_size)(const char* filename);
} os_api_t;

/* ============================================================================
 * GLOBAL STATE
 * ============================================================================ */

os_api_t* os_api;

/* File manager state */
static char file_list_str[2048];
static char filtered_list[1024];
static int file_count = 0;
static int scroll_offset = 0;
static int selected_file = -1;

/* Mouse position */
static int mouse_x = 40, mouse_y = 12;

/* Keyboard navigation focus state */
static int keyboard_mode = 0; /* 0=mouse, 1=keyboard nav */
static int focus_index = 0;   /* 0=close btn, 1=refresh, 2=run, 3+=file list */
#define FOCUS_CLOSE_BTN   0
#define FOCUS_REFRESH     1
#define FOCUS_RUN         2
#define FOCUS_FILE_START  3

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

static int is_hover(int mx, int my, int x, int y, int w, int h) {
    return (mx >= x && mx < x + w && my >= y && my < y + h);
}

static int str_ends_with(const char* str, const char* suffix) {
    int str_len = os_api->strlen(str);
    int suffix_len = os_api->strlen(suffix);
    if (str_len < suffix_len) return 0;
    return os_api->memcmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
}

static int get_file_extension_type(const char* filename) {
    if (str_ends_with(filename, ".bin")) return EXT_BIN;
    if (str_ends_with(filename, ".BIN")) return EXT_BIN;
    if (str_ends_with(filename, ".bas")) return EXT_BAS;
    if (str_ends_with(filename, ".BAS")) return EXT_BAS;
    if (str_ends_with(filename, ".ike")) return EXT_IKE;
    if (str_ends_with(filename, ".IKE")) return EXT_IKE;
    if (str_ends_with(filename, ".bgs")) return EXT_BGS;
    if (str_ends_with(filename, ".BGS")) return EXT_BGS;
    return EXT_UNKNOWN;
}

static const char* get_extension_name(int ext_type) {
    switch (ext_type) {
        case EXT_BIN: return "BIN";
        case EXT_BAS: return "BAS";
        case EXT_IKE: return "IKE";
        case EXT_BGS: return "BGS";
        default: return "???";
    }
}

static void filter_file_list(void) {
    int src_idx = 0;
    int dst_idx = 0;
    filtered_list[0] = '\0';
    file_count = 0;
    
    while (file_list_str[src_idx]) {
        /* Extract filename */
        char filename[MAX_FILENAME];
        int buf_idx = 0;
        while (file_list_str[src_idx] && file_list_str[src_idx] != '|' && buf_idx < MAX_FILENAME - 1) {
            filename[buf_idx++] = file_list_str[src_idx++];
        }
        filename[buf_idx] = '\0';
        
        /* Check if it's a supported file type */
        if (get_file_extension_type(filename) != EXT_UNKNOWN) {
            /* Add to filtered list */
            if (dst_idx > 0) {
                filtered_list[dst_idx++] = '|';
            }
            int i = 0;
            while (filename[i]) {
                filtered_list[dst_idx++] = filename[i++];
            }
            file_count++;
        }
        
        /* Skip separator */
        if (file_list_str[src_idx] == '|') src_idx++;
    }
    
    filtered_list[dst_idx] = '\0';
}

static void refresh_file_list(void) {
    os_api->get_file_list(file_list_str);
    filter_file_list();
    scroll_offset = 0;
    selected_file = -1;
    if (keyboard_mode && focus_index >= FOCUS_FILE_START) {
        focus_index = FOCUS_FILE_START;
    }
}

/* ============================================================================
 * DRAWING HELPERS
 * ============================================================================ */

static void draw_file_row(int x, int y, int w, const char* filename, 
                           uint32_t size, int ext_type, int is_selected, int is_hovered) {
    uint8_t attr;
    
    /* Determine attribute based on state and file type */
    if (is_selected) {
        switch (ext_type) {
            case EXT_BIN: attr = ATTR_BIN_SEL; break;
            case EXT_BAS: attr = ATTR_BAS_SEL; break;
            case EXT_IKE: attr = ATTR_IKE_SEL; break;
            case EXT_BGS: attr = ATTR_BGS_SEL; break;
            default: attr = ATTR_SELECTED; break;
        }
    } else if (is_hovered) {
        attr = ATTR_HOVERED;
    } else {
        switch (ext_type) {
            case EXT_BIN: attr = ATTR_BIN; break;
            case EXT_BAS: attr = ATTR_BAS; break;
            case EXT_IKE: attr = ATTR_IKE; break;
            case EXT_BGS: attr = ATTR_BGS; break;
            default: attr = ATTR_NORMAL; break;
        }
    }
    
    /* Clear row background */
    for (int i = 0; i < w; i++) {
        os_api->vgraphics_put_char(x + i, y, ' ', attr);
    }
    
    /* Filename */
    os_api->vgraphics_put_string(x + 1, y, filename, attr);
    
    /* Extension type */
    os_api->vgraphics_put_string(x + 30, y, get_extension_name(ext_type), attr);
    
    /* Size */
    if (size > 0) {
        char size_buf[12];
        os_api->int_to_str(size, size_buf);
        int len = os_api->strlen(size_buf);
        os_api->vgraphics_put_string(x + 36, y, size_buf, attr);
        os_api->vgraphics_put_string(x + 37 + len, y, "bytes", attr);
    }
}

/* ============================================================================
 * UI COMPONENTS
 * ============================================================================ */

static void draw_toolbar(int x, int y, int w, int mx, int my) {
    /* Background */
    os_api->vgraphics_draw_rect_fill(x, y, w, 1, TOOLBAR_BG);
    
    /* Refresh button - keyboard focus support */
    int hover_refresh = is_hover(mx, my, x + BTN_REFRESH_X, y, BTN_REFRESH_W, 1);
    int focus_refresh = (keyboard_mode && focus_index == FOCUS_REFRESH);
    uint8_t refresh_attr = (hover_refresh || focus_refresh) ? ATTR_TOOLBAR_HOV : ATTR_TOOLBAR;
    os_api->vgraphics_put_string(x + BTN_REFRESH_X, y, "[Refresh]", refresh_attr);
    
    /* Run button - keyboard focus support */
    int hover_run = is_hover(mx, my, x + BTN_RUN_X, y, BTN_RUN_W, 1);
    int focus_run = (keyboard_mode && focus_index == FOCUS_RUN);
    uint8_t run_attr = (hover_run || focus_run) ? ATTR_TOOLBAR_HOV : ATTR_TOOLBAR;
    os_api->vgraphics_put_string(x + BTN_RUN_X, y, "[Run]", run_attr);
    
    /* Separator */
    os_api->vgraphics_put_char(x + BTN_RUN_X + BTN_RUN_W + 1, y, '|', ATTR(COLOR_DGRAY, TOOLBAR_BG));
    
    /* Supported types display */
    os_api->vgraphics_put_string(x + BTN_RUN_X + BTN_RUN_W + 3, y, 
                                  "Supported: .bin .bas .ike .bgs", ATTR_TOOLBAR);
    
    /* Scroll buttons - right aligned */
    int scroll_up_x = BTN_SCROLL_UP_X(w);
    int hover_up = is_hover(mx, my, x + scroll_up_x, y, BTN_SCROLL_UP_W, 1);
    uint8_t up_attr = hover_up ? ATTR(COLOR_WHITE, SCROLL_FG) : ATTR(SCROLL_FG, SCROLL_BG);
    os_api->vgraphics_put_string(x + scroll_up_x, y, " /\ ", up_attr);
    
    int scroll_down_x = BTN_SCROLL_DOWN_X(w);
    int hover_dn = is_hover(mx, my, x + scroll_down_x, y, BTN_SCROLL_DOWN_W, 1);
    uint8_t dn_attr = hover_dn ? ATTR(COLOR_WHITE, SCROLL_FG) : ATTR(SCROLL_FG, SCROLL_BG);
    os_api->vgraphics_put_string(x + scroll_down_x, y, " \/ ", dn_attr);
}

static void draw_header(int x, int y, int w) {
    /* Header background */
    os_api->vgraphics_draw_rect_fill(x, y, w, 1, FILE_HEADER_BG);
    
    /* Column headers */
    os_api->vgraphics_put_string(x + 1, y, " Name                       ", ATTR_HEADER);
    os_api->vgraphics_put_string(x + 30, y, "Type", ATTR_HEADER);
    os_api->vgraphics_put_string(x + 36, y, "Size", ATTR_HEADER);
    
    /* File count - right aligned */
    char count_str[16];
    os_api->int_to_str(file_count, count_str);
    char count_display[24];
    os_api->strcpy(count_display, "Files: ");
    os_api->strcat(count_display, count_str);
    int count_x = x + w - os_api->strlen(count_display) - 2;
    os_api->vgraphics_put_string(count_x, y, count_display, ATTR_HEADER);
}

static void parse_and_draw_files(int x, int y, int w, int h, int mx, int my) {
    int row = 0;
    int current_idx = 0;
    int file_idx = 0;
    int max_visible = MAX_VISIBLE_FILES(h);
    
    /* Skip scrolled files */
    int skip_count = 0;
    while (skip_count < scroll_offset && filtered_list[current_idx]) {
        if (filtered_list[current_idx] == '|') skip_count++;
        current_idx++;
    }
    file_idx = skip_count;
    
    /* Display files */
    while (filtered_list[current_idx] && row < max_visible && file_idx < file_count) {
        int item_y = y + FILE_START_ROW + row;
        
        /* Extract filename */
        char filename_buf[MAX_FILENAME];
        int buf_idx = 0;
        while (filtered_list[current_idx] && filtered_list[current_idx] != '|' && buf_idx < MAX_FILENAME - 1) {
            filename_buf[buf_idx++] = filtered_list[current_idx++];
        }
        filename_buf[buf_idx] = '\0';
        
        /* Determine state - keyboard focus overrides mouse hover */
        int is_selected = (file_idx == selected_file);
        int is_kb_focused = (keyboard_mode && focus_index == FOCUS_FILE_START + file_idx);
        int is_hovered = is_kb_focused || is_hover(mx, my, x, item_y, w, 1);
        int ext_type = get_file_extension_type(filename_buf);
        
        /* Get size */
        uint32_t size = os_api->fat16_file_size(filename_buf);
        
        draw_file_row(x, item_y, w, filename_buf, size, ext_type, is_selected, is_hovered);
        
        /* Skip separator */
        if (filtered_list[current_idx] == '|') current_idx++;
        
        file_idx++;
        row++;
    }
}

static void draw_runner_app(int x, int y, int w, int h, int mx, int my) {
    /* Background - fill entire window area */
    os_api->vgraphics_draw_rect_fill(x, y, w, h, WINDOW_BG);
    
    /* Components */
    draw_toolbar(x, y, w, mx, my);
    draw_header(x, y + HEADER_ROW, w);
    parse_and_draw_files(x, y, w, h, mx, my);
    
    /* Status bar at bottom */
    int status_y = y + h - 1;
    os_api->vgraphics_draw_rect_fill(x, status_y, w, 1, STATUS_BG);
    if (keyboard_mode) {
        os_api->vgraphics_put_string(x + 1, status_y, "[Up/Down] Navigate  [Enter] Run  [Q]uit", ATTR_STATUS);
    } else {
        os_api->vgraphics_put_string(x + 1, status_y, "[Up/Down] Navigate  [Q]uit  [Click] Mouse", ATTR_STATUS);
    }
}

/* ============================================================================
 * FILE OPERATIONS
 * ============================================================================ */

static void run_selected_file(void) {
    if (selected_file < 0 || selected_file >= file_count) return;
    
    /* Find the selected file in the filtered list */
    int file_idx = 0;
    int current_idx = 0;
    while (file_idx < selected_file && filtered_list[current_idx]) {
        if (filtered_list[current_idx] == '|') file_idx++;
        current_idx++;
    }
    
    /* Extract filename */
    char filename_buf[MAX_FILENAME];
    int buf_idx = 0;
    while (filtered_list[current_idx] && filtered_list[current_idx] != '|' && buf_idx < MAX_FILENAME - 1) {
        filename_buf[buf_idx++] = filtered_list[current_idx++];
    }
    filename_buf[buf_idx] = '\0';
    
    /* Run the file */
    os_api->run(filename_buf);
}

/* ============================================================================
 * INPUT HANDLING
 * ============================================================================ */

typedef enum {
    ACTION_NONE = 0,
    ACTION_REFRESH,
    ACTION_RUN,
    ACTION_SCROLL_UP,
    ACTION_SCROLL_DOWN,
    ACTION_SELECT_FILE
} action_t;

static action_t handle_toolbar_click(int x, int y, int w, int mx, int my) {
    if (my != y) return ACTION_NONE;
    
    if (is_hover(mx, my, x + BTN_REFRESH_X, y, BTN_REFRESH_W, 1)) {
        return ACTION_REFRESH;
    }
    if (is_hover(mx, my, x + BTN_RUN_X, y, BTN_RUN_W, 1)) {
        return ACTION_RUN;
    }
    if (is_hover(mx, my, x + BTN_SCROLL_UP_X(w), y, BTN_SCROLL_UP_W, 1)) {
        return ACTION_SCROLL_UP;
    }
    if (is_hover(mx, my, x + BTN_SCROLL_DOWN_X(w), y, BTN_SCROLL_DOWN_W, 1)) {
        return ACTION_SCROLL_DOWN;
    }
    
    return ACTION_NONE;
}

static action_t handle_file_list_click(int x, int y, int w, int h, int mx, int my) {
    int row = 0;
    int current_idx = 0;
    int file_idx = 0;
    int max_visible = MAX_VISIBLE_FILES(h);
    
    /* Skip scrolled files */
    int skip_count = 0;
    while (skip_count < scroll_offset && filtered_list[current_idx]) {
        if (filtered_list[current_idx] == '|') skip_count++;
        current_idx++;
    }
    file_idx = skip_count;
    
    /* Check each file */
    while (filtered_list[current_idx] && row < max_visible && file_idx < file_count) {
        int item_y = y + FILE_START_ROW + row;
        
        if (is_hover(mx, my, x, item_y, w, 1)) {
            selected_file = file_idx;
            return ACTION_SELECT_FILE;
        }
        
        /* Skip to next file */
        while (filtered_list[current_idx] && filtered_list[current_idx] != '|') current_idx++;
        if (filtered_list[current_idx] == '|') current_idx++;
        
        file_idx++;
        row++;
    }
    
    return ACTION_NONE;
}

static action_t handle_runner_click(int x, int y, int w, int h, int mx, int my) {
    action_t action = handle_toolbar_click(x, y, w, mx, my);
    if (action != ACTION_NONE) return action;
    
    return handle_file_list_click(x, y, w, h, mx, my);
}

static void process_action(action_t action) {
    switch (action) {
        case ACTION_REFRESH:
            refresh_file_list();
            break;
        case ACTION_RUN:
            run_selected_file();
            break;
        case ACTION_SCROLL_UP:
            if (scroll_offset > 0) scroll_offset--;
            break;
        case ACTION_SCROLL_DOWN:
            if (scroll_offset < file_count - 1) scroll_offset++;
            break;
        case ACTION_SELECT_FILE:
            /* File was selected, ready to run */
            break;
        default:
            break;
    }
}

static int get_max_focus(void) {
    return FOCUS_FILE_START + file_count - 1;
}

static void handle_keyboard_nav(char c) {
    int max_focus = get_max_focus();
    
    switch (c) {
        case KEY_DOWN:
            keyboard_mode = 1;
            focus_index++;
            if (focus_index > max_focus) {
                focus_index = FOCUS_CLOSE_BTN;
            }
            /* Auto-scroll if needed when in file list */
            if (focus_index >= FOCUS_FILE_START) {
                int file_idx = focus_index - FOCUS_FILE_START;
                selected_file = file_idx;
                if (file_idx >= scroll_offset + MAX_VISIBLE_FILES(APP_HEIGHT)) {
                    scroll_offset++;
                }
            }
            break;
            
        case KEY_UP:
            keyboard_mode = 1;
            focus_index--;
            if (focus_index < FOCUS_CLOSE_BTN) {
                focus_index = max_focus;
            }
            /* Auto-scroll if needed when in file list */
            if (focus_index >= FOCUS_FILE_START) {
                int file_idx = focus_index - FOCUS_FILE_START;
                selected_file = file_idx;
                if (file_idx < scroll_offset) {
                    scroll_offset = file_idx;
                }
            }
            break;
    }
}

/* ============================================================================
 * MAIN LOOP
 * ============================================================================ */

static void app_loop(void) {
    /* Clear keyboard buffer */
    while (os_api->keyboard_getchar() != 0) {}
    refresh_file_list();

    while (1) {
        /* Clear and draw background */
        os_api->vgraphics_clear(DESKTOP_COLOR);

        /* Draw main window frame with 3D effect */
        os_api->vgraphics_draw_window(APP_X - 1, APP_Y - 1, APP_WIDTH + 2, APP_HEIGHT + 2, 
                                       " Program Runner ", WINDOW_BORDER);
        
        /* Draw close button [X] */
        int close_x = CLOSE_BTN_X(APP_X, APP_WIDTH);
        int close_y = CLOSE_BTN_Y(APP_Y);
        int close_hover = is_hover(mouse_x, mouse_y, close_x, close_y, 3, 1);
        int close_focus = (keyboard_mode && focus_index == FOCUS_CLOSE_BTN);
        uint8_t close_attr = (close_hover || close_focus) ? ATTR(COLOR_WHITE, COLOR_RED) : ATTR(COLOR_WHITE, COLOR_GREEN);
        os_api->vgraphics_put_string(close_x, close_y, "[X]", close_attr);    
      
        /* Draw main UI */
        draw_runner_app(APP_X, APP_Y, APP_WIDTH, APP_HEIGHT, mouse_x, mouse_y);
        
        /* Draw mouse cursor */
        os_api->vgraphics_put_char(mouse_x, mouse_y, MOUSE_CHAR, MOUSE_COLOR);
        os_api->vgraphics_repaint();

        /* Input handling */
        int left_click = 0;
        int right_click = 0;
        
        /* Poll mouse */
        int prev_mouse_x = mouse_x, prev_mouse_y = mouse_y;
        for (int i = 0; i < 10; i++) {
            os_api->mouse_handler(&mouse_x, &mouse_y, &left_click, &right_click);
        }
        /* If mouse moved, switch to mouse mode */
        if (mouse_x != prev_mouse_x || mouse_y != prev_mouse_y) {
            keyboard_mode = 0;
        }

        char c = os_api->keyboard_getchar();
        
        /* Convert left click to enter */
        if (left_click) {
            c = '\n';
            os_api->busy_delay(100);
        }

        if (c == 0 && !left_click) {
            continue;
        }
        
        /* Handle quit */
        if (c == 'q' || c == KEY_ESC) break;
        
        /* Handle close button click */
        if (c == '\n' || c == ' ') {
            int close_x = CLOSE_BTN_X(APP_X, APP_WIDTH);
            int close_y = CLOSE_BTN_Y(APP_Y);
            if (is_hover(mouse_x, mouse_y, close_x, close_y, 3, 1)) {
                break; /* Exit on close button click */
            }
        }
        
        /* Handle keyboard navigation */
        handle_keyboard_nav(c);
        
        /* Handle clicks and keyboard activation */
        if (c == '\n' || c == ' ') {
            if (keyboard_mode) {
                /* Keyboard activation */
                if (focus_index == FOCUS_CLOSE_BTN) {
                    break;
                } else if (focus_index == FOCUS_REFRESH) {
                    refresh_file_list();
                } else if (focus_index == FOCUS_RUN) {
                    run_selected_file();
                } else if (focus_index >= FOCUS_FILE_START) {
                    /* Run selected file */
                    run_selected_file();
                }
            } else {
                /* Mouse click */
                action_t action = handle_runner_click(APP_X, APP_Y, APP_WIDTH, APP_HEIGHT, mouse_x, mouse_y);
                process_action(action);
                
                /* If a file was selected with mouse, run it */
                if (action == ACTION_SELECT_FILE) {
                    run_selected_file();
                }
            }
        }
    }
}

/* ============================================================================
 * ENTRY POINT
 * ============================================================================ */

int main(void) {
    os_api = (os_api_t*)OS_API_ADDR;
    
    os_api->disable_cursor();
    os_api->mouse_init();
    os_api->keyboard_init();
    
    app_loop();
    
    os_api->enable_cursor();
    os_api->vgraphics_clear(COLOR_BLACK);
    
    return 0;
}
