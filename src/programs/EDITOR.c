#include  "stddef.h"
#include "stdint.h"

#define TEXT_COL_LIMIT 76
#define MAX_LINES 200
#define MAX_TEXT_SIZE 8192
#define LINE_LENGTH 76
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
#define FILENAME_SIZE 12  // 8 chars name + 3 chars extension + null terminator
#define TEXT_BUFFER_ADDR 0xBAB000


char* current_filename = (char*)(TEXT_BUFFER_ADDR - FILENAME_SIZE);

int text_length = 0;

char line_buffer[MAX_LINES][TEXT_COL_LIMIT]; 
int line_count = 0;
uint8_t quit=0;

uint8_t saved=1;
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

void next_line();
void set_char_with_cursor(char c);
void clear_memory(void* start_addr, uint32_t size);

char number_map[10]={'0','1','2','3','4','5','6','7','8','9'};


#define OS_API_ADDR 0x5F0F0
#define TOP_NAV 0
#define BOTTOM_NAV 25

#define VGA_ADDRESS 0xB8000


static uint16_t* vga_buffer = (uint16_t*) VGA_ADDRESS;

char* render_buffer=(char*) 0xB00000;
char* text_buffer=(char*)   0xBAB000;

uint8_t control_mode=0;

os_api_t* os_api;
uint8_t cursor_row=1;
uint8_t cursor_col=2;
int cursor_counter=10;


int current_line = 0;
int current_col = 0;

void clear_text_buffer() {
    for (int i = 0; i < LINE_LENGTH * MAX_LINES; i++) {
        text_buffer[i] = 0;
    }
    current_line = 0;
    current_col = 0;
}

void change_control_mode(uint8_t mode) {
    control_mode = mode;
}

void write_char_to_buffer(char c) {
    if (c == '\n') {
        current_line++;
        current_col = 0;
         text_length++;
        return;
    }

    if (current_line >= MAX_LINES)
        return;
    if (current_col >= LINE_LENGTH)
        current_col = LINE_LENGTH - 1;

    int offset = (current_line * LINE_LENGTH) + current_col;
    text_buffer[offset] = c;
    current_col++;
     text_length++;
}


int count_lines_in_buffer(void) {
    int count = 0;

    for (int i = 0; i < MAX_LINES; i++) {
        int empty = 1;
        for (int j = 0; j < TEXT_COL_LIMIT; j++) {
            if (line_buffer[i][j] != '\0') {
                empty = 0;
                break;
            }
        }
        if (!empty) {
            count++;
        } else {
            break;
        }
    }

    return count;
}

void parse_line_buffer_to_text_buffer(void) {
    int offset = 0;

    line_count = count_lines_in_buffer();
    if (line_count == 0) line_count = 1;

    for (int i = 0; i < line_count; i++) {
        int line_empty = 1;

        for (int j = 0; j < TEXT_COL_LIMIT && line_buffer[i][j] != '\0'; j++) {
            if (offset < MAX_TEXT_SIZE - 1) {
                text_buffer[offset++] = line_buffer[i][j];
                line_empty = 0;
            }
        }

        if (!line_empty && i < line_count - 1 && offset < MAX_TEXT_SIZE - 1) {
            text_buffer[offset++] = '\n';
        }
    }

    text_buffer[offset] = '\0';
    text_length = offset;
}



void write_string_to_buffer(const char* str) {
    while (*str) {
        write_char_to_buffer(*str++);
    }
}

char read_char_from_buffer(int line, int col) {
    if (line >= MAX_LINES || col >= LINE_LENGTH)
        return 0;
    int offset = (line * LINE_LENGTH) + col;
    return text_buffer[offset];
}

void read_line_from_buffer(int line, char* dest) {
    if (line >= MAX_LINES)
        return;
    int offset = line * LINE_LENGTH;
    for (int i = 0; i < LINE_LENGTH; i++) {
        dest[i] = text_buffer[offset + i];
    }
}



void init_api(void){
    os_api=(os_api_t*) OS_API_ADDR;
}

static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | (uint16_t) color << 8;
}

// ga_buffer[cursor_row * 80 + cursor_col] = vga_entry(render_buffer[i], 0x0F);

void render(){
    for(int i=0;i<80*25;i++){
         vga_buffer[i] = vga_entry(render_buffer[i], 0x0F);
    }
    os_api->move_cursor((uint8_t) cursor_row, (uint8_t) cursor_col);
}

void print_banner_border(char c,int number){
    for(int i=0;i<80*25;i++){
         render_buffer[i]=c;
    }
}

void print_string(char* c,int number){
    int loop_start=cursor_counter;
    int temp=0;
    for(int i=loop_start;i<loop_start+number;i++,temp++){
         render_buffer[i]=c[temp];
    }

}


void move_cursor(uint8_t x,uint8_t y){
    cursor_counter=x*80+y;
     os_api->move_cursor(cursor_row,cursor_col);
}

void print_top_nav(){
    print_banner_border(' ',40-6);
    move_cursor(0,40-6);
    print_string("TEXT EDITOR (",13);
    for(int i=0;i<12;i++){
        if(current_filename[i]==0){
            render_buffer[0*80+ (40-6+13 + i)]=')';
            break;
        }
         render_buffer[0*80+ (40-6+13 + i)]=current_filename[i];
    }
}

void print_bottom_nav(){
      move_cursor(24,0);
      if(saved==1){
          move_cursor(24,0);
          print_string("======SAVE(CTRL+S)  EXIT(CTRL+Q)  NEW(CTRL+N) LOAD(CTRL+L) MODE(CTRL) (S)======",80);
          return;
      }
          print_string("======SAVE(CTRL+S)  EXIT(CTRL+Q)  NEW(CTRL+N) LOAD(CTRL+L) MODE(CTRL) (N)======",80);
}

void update_control_mode_indicator(){
    move_cursor(0,0);
    if(control_mode==1){
        print_string("CONTROL MODE",13);
    }
    else{
        print_string("INSERT MODE",12);
    }
}


void parse_text_buffer_to_line_buffer() {
    line_count = 0;
    int col = 0;
    int line_idx = 0;

    for (int i = 0; i < text_length || text_buffer[i] != '\0'; i++) {
        char c = text_buffer[i];

        if (c == '\n' || col >= 76-1) {
            line_buffer[line_count][col] = '\0'; // terminate line
            line_count++;
            col = 0;
            line_idx = 0;

            if (line_count >= MAX_LINES) break;
            continue;
        }

        line_buffer[line_count][col++] = c;
    }

    // Terminate the last line if needed
    if (col > 0 && line_count < MAX_LINES) {
        line_buffer[line_count][col] = '\0';
        line_count++;
    }
}


void render_text_panel(){
    for(int i=1;i<24;i++){
        for(int j=0;j<80;j++){
            if(j==0){
                render_buffer[i*80+j]='|';
                continue;
            }
            else if(j==1){
                render_buffer[i*80+j]='|';
                continue;   
            }
            else if(j==78){
                render_buffer[i*80+j]='|';
                continue;   
            }
            else if(j==79){
                render_buffer[i*80+j]='|';
                continue;   
            }
            else{
                render_buffer[i*80+j]=line_buffer[i-1][j-2];
            }
        }
    }
}


void exit_editor(){
    os_api->load_vga(0);
}


void set_filename(const char* name, const char* ext) {
    for (int i = 0; i < FILENAME_SIZE; i++) current_filename[i] = 0;

    for (int i = 0; i < 8 && name[i]; i++) current_filename[i] = name[i];

    for (int i = 0; i < 3 && ext[i]; i++) current_filename[8+i] = ext[i];
}



char get_input(){
    return os_api->keyboard_read();
}


#define TAB_WIDTH 4

void handle_input(char c) {
    if (c == 0) return;

    if(c==KEY_CTRL){
        change_control_mode(control_mode == 1 ? 0 : 1);
        return;
    }


    if(control_mode==1){
        if(c=='Q' || c=='q'){
            quit=1;
            return;
        }
        if(c=='S' || c=='s'){
            parse_line_buffer_to_text_buffer();
            os_api->fat16_file_save(current_filename, (uint8_t*)text_buffer, text_length);
            saved=1;
            return;
        }

        return;
    }
    
    if(c==KEY_UP){
        if(cursor_row>1){
            cursor_row--;
        }
        return;
    }
    if(c==KEY_DOWN){
        if(cursor_row<24-1){
            cursor_row++;
        }
        return;
    }
    if(c==KEY_LEFT){
        if(cursor_col>2){
            cursor_col--;
        }
        return;
    }
    if(c==KEY_RIGHT){
        if(cursor_col<76){
            cursor_col++;
        }
        return;
    }
   
    if (c == '\n') {
        next_line();
        return;
    }

    if (c == '\b') {

        if (cursor_col > 2) {
            cursor_col--;
            set_char_with_cursor(' ');
        }

        else if (cursor_row > 1) {
            cursor_row--;
            int len = 0;
            while (len < 76-1 && line_buffer[cursor_row-1][len] != '\0')
                len++;
            cursor_col = len + 2;
        }
        return;
    }

    if (c == '\t') {
        for (int i = 0; i < TAB_WIDTH; i++) {
            if (cursor_col > 76) { // wrap
                cursor_row++;
                cursor_col = 2;
                if (cursor_row >= 24) cursor_row = 23;
                break;
            }
            set_char_with_cursor(' ');
            cursor_col++;
        }
        return;
    }

    if (c >= 32 && c <= 126) {
        set_char_with_cursor(c);
        cursor_col++;

        // Wrap to next line when reaching edge
        if (cursor_col > 78) {
            cursor_row++;
            cursor_col = 2;
            if (cursor_row >= 24) cursor_row = 23;
        }
        return;
    }

}

void set_char_with_cursor(char c){
    line_buffer[cursor_row-1][cursor_col-2]=c;
}

void set_char_at_line_buffer(int x,int y,char c){
    line_buffer[x][y] = c;
}


void next_line(){
    cursor_row++;
    cursor_col=2;
    if (cursor_row >= 24) cursor_row = 23;
}


void clear_memory(void* start_addr, uint32_t size) {
    uint8_t* ptr = (uint8_t*)start_addr;
    for (uint32_t i = 0; i < size; i++) ptr[i] = 0;
}
 


void clean_up(){
    clear_memory(render_buffer, 4096);
    clear_text_buffer();
    for(int i=0;i<MAX_LINES;i++){
        for(int j=0;j<TEXT_COL_LIMIT;j++){
            line_buffer[i][j]=0;
        }
    }
    text_length=0;
    cursor_row=1;
    cursor_col=2;
    control_mode=0;
    quit=0;
}


void main(){
    init_api();
    os_api->save_vga(0);
    os_api->screen_clear_shell();
    os_api->move_cursor(cursor_row,cursor_col);
    print_top_nav();
    print_bottom_nav();
    update_control_mode_indicator();
    parse_text_buffer_to_line_buffer();
    render_text_panel();
    render();
    while(quit==0){
        char c=get_input();
        handle_input(c);
        print_bottom_nav();
        if(c){
            if(control_mode==0){
                saved=0;
                render_text_panel();
            }
            update_control_mode_indicator();
            render();
        }
    }
    clean_up();
    exit_editor();
}
