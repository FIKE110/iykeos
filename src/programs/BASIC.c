#include <stddef.h>
#include <stdint.h>
#define SECTOR_SIZE 512

void prepare_environment(void);
void clear_memory(uint32_t start_addr, uint32_t size);
void handle_read(char* args);
void handle_write(char* args);
void handle_beep(char* args);
void handle_pset(char* args);
void handle_line(char* args);
void handle_dim(char* args);
void handle_list();
void handle_left(char* args);
void handle_set_array(char* args);
void handle_get_array(char* args);
void handle_print(char* args);
void handle_end(void);
void print_in_line(char* args);
int handle_peek(char* args);
void handle_locate(char* args);
void handle_color(char* args);
void handle_poke(char* args);
void execute_program(void);
void parse_program(char* program);
void init_program();
void graceful_exit();
void clear_all_string_vars();
void tiny_free_all();
void init_api(void);
int starts_with(const char* str, const char* prefix);
uint32_t parse_solve_expression(char* expression);
uint32_t get_random(uint32_t max);


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
} os_api_t;

typedef struct {
    int line;
    char condition[64];
} WhileLoop;

#define WHILE_STACK_SIZE 10
static WhileLoop while_stack[WHILE_STACK_SIZE];
static int while_stack_ptr = 0;

typedef struct {
    char var;
    int end;
    int line;
} ForLoop;

#define FOR_STACK_SIZE 10
static ForLoop for_stack[FOR_STACK_SIZE];
static int for_stack_ptr = 0;

#define MAX_ARRAYS 5
#define ARRAY_SIZE 100
typedef struct {
    char var;
    int data[ARRAY_SIZE];
    int size;
} Array;
static Array arrays[MAX_ARRAYS];
static int array_count = 0;


typedef struct {
    char name[16];
    uint8_t line;
} Function;

#define FUNCTIONS_NUM 24

Function functions[FUNCTIONS_NUM];
uint8_t returnaddresses[100];
uint8_t return_stack_pointr=0;

static int data_line_ptr = 0;
static char* data_char_ptr = NULL;

static char* expr_ptr;
static int parse_expr();



#define OS_API_ADDR 0x5F0F0

#define PROGRAM_MEM_START 0xBAB000
#define TEXT_BUFFER_ADDR  0xB00000
#define MAX_STRING_LEN 64
#define MAX_LINES 200
#define MAX_LINE_LENGTH 70

#define HEAP_START 0x800000
#define HEAP_SIZE  1024  

#define STRING_BLOCK_SIZE 64     // each string gets 64 bytes
#define NUM_STRING_VARS 26       // A-Z
#define STRING_HEAP_START 0x900000

char (*program_lines)[MAX_LINE_LENGTH] = (char (*)[MAX_LINE_LENGTH])PROGRAM_MEM_START;
int line_numbers[MAX_LINES];
int program_line_count = 0;

#define FIXED_SCALE 1000

int int_to_fixed(int n) {
    return n * FIXED_SCALE;
}

int fixed_to_int(int f) {
    return f / FIXED_SCALE;
}

int str_to_fixed(const char* s) {
    int sign = 1;
    if (*s == '-') { sign = -1; s++; }
    
    int whole = 0;
    while (*s >= '0' && *s <= '9') {
        whole = whole * 10 + (*s - '0');
        s++;
    }
    
    int frac = 0;
    int divisor = 1;
    if (*s == '.') {
        s++;
        while (*s >= '0' && *s <= '9' && divisor < FIXED_SCALE) {
            frac = frac * 10 + (*s - '0');
            divisor *= 10;
            s++;
        }
    }
    
    // Scale fractional part
    while (divisor < FIXED_SCALE) {
        frac *= 10;
        divisor *= 10;
    }
    
    return sign * (whole * FIXED_SCALE + frac);
}

void fixed_to_str(int f, char* buf) {
    if (f < 0) {
        *buf++ = '-';
        f = -f;
    }
    
    int whole = f / FIXED_SCALE;
    int frac = f % FIXED_SCALE;
    
    // Convert whole part
    char temp[16];
    int i = 0;
    if (whole == 0) {
        temp[i++] = '0';
    } else {
        while (whole > 0) {
            temp[i++] = (whole % 10) + '0';
            whole /= 10;
        }
    }
    
    // Reverse whole part
    for (int j = i - 1; j >= 0; j--) {
        *buf++ = temp[j];
    }
    
    // Add decimal point and fractional part
    *buf++ = '.';
    
    // Always show 3 decimal places
    *buf++ = (frac / 100) + '0';
    *buf++ = ((frac / 10) % 10) + '0';
    *buf++ = (frac % 10) + '0';
    *buf = '\0';
}

int fixed_add(int a, int b) {
    return a + b;
}

int fixed_sub(int a, int b) {
    return a - b;
}

int fixed_mul(int a, int b) {
    int a_whole = a / FIXED_SCALE;
    int a_frac = a % FIXED_SCALE;
    int b_whole = b / FIXED_SCALE;
    int b_frac = b % FIXED_SCALE;
    
    int result = a_whole * b_whole * FIXED_SCALE;
    result += a_whole * b_frac;
    result += a_frac * b_whole;
    result += (a_frac * b_frac) / FIXED_SCALE;
    
    return result;
}

int fixed_div(int a, int b) {
    if (b == 0) return 0;
    int quot = a / b;
    int rem = a % b;
    return quot * FIXED_SCALE + (rem * FIXED_SCALE) / b;
}

void clear_all_string_vars() {
    for (int i = 0; i < NUM_STRING_VARS; i++) {
        char* block = (char*)(STRING_HEAP_START + (i * STRING_BLOCK_SIZE));
        for (int j = 0; j < STRING_BLOCK_SIZE; j++) {
            block[j] = '\0';
        }
    }
}


int str_to_int(const char* s) {
    int result = 0;
    int sign = 1;
    if (*s == '-') {
        sign = -1;
        s++;
    }

    while (*s) {
        if (*s < '0' || *s > '9') break; // stop at non-digit
        result = result * 10 + (*s - '0');
        s++;
    }
    return result * sign;
}


void int_to_str(int num, char* buf) {
    int i = 0;
    int sign = 0;

    if (num < 0) {
        sign = 1;
        num = -num;
    }

    do {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    } while (num > 0);

    if (sign) buf[i++] = '-';

    buf[i] = '\0';

    for (int j = 0, k = i - 1; j < k; j++, k--) {
        char tmp = buf[j];
        buf[j] = buf[k];
        buf[k] = tmp;
    }
}

int str_to_int_len(const char* s, int len) {
    int result = 0;
    for (int i = 0; i < len; i++) {
        if (s[i] < '0' || s[i] > '9') break;
        result = result * 10 + (s[i] - '0');
    }
    return result;
}



void parse_program(char* program) {
    int i = 0;
    int line_index = 0;
    int char_index = 0;
    int skip_number = 1;

    while (program[i] != '\0' && line_index < MAX_LINES) {
        char c = program[i];

        if (c == '\n' || c == ';') {
            program_lines[line_index][char_index] = '\0';
            line_index++;
            char_index = 0;
            skip_number = 1;
        } 
        else {
            if (skip_number) {
                if (c >= '0' && c <= '9') {
                    line_numbers[line_index] = line_numbers[line_index] * 10 + (c - '0');
                }
                if (c == ' ') {
                    skip_number = 0;
                }
            } 
            else {
                if (char_index < MAX_LINE_LENGTH - 1)
                    program_lines[line_index][char_index++] = c;
            }
        }

        i++;
    }

    if (char_index > 0 && line_index < MAX_LINES) {
        program_lines[line_index][char_index] = '\0';
        line_index++;
    }

    program_line_count = line_index;
}



char* get_string_var(char var) {
    if (var < 'A' || var > 'Z') return 0; // invalid variable
    uint32_t index = var - 'A';
    return (char*)(STRING_HEAP_START + (index * STRING_BLOCK_SIZE));
}

void set_string_var(char var, const char* value) {
    if (var < 'A' || var > 'Z') return;
    uint32_t index = var - 'A';
    char* dest = (char*)(STRING_HEAP_START + (index * STRING_BLOCK_SIZE));

    size_t len = 0;
    while (*value && len < STRING_BLOCK_SIZE - 1) {
        dest[len++] = *value++;
    }
    dest[len] = '\0';
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}



int int_vars[26];

os_api_t* os_api;

static uint8_t* heap_ptr = (uint8_t*) HEAP_START;       // next free byte
static uint8_t* heap_end = (uint8_t*) (HEAP_START + HEAP_SIZE);

char* tiny_malloc(uint32_t size) {
    if (heap_ptr + size + 1 > heap_end) return 0;
    char* ptr = (char*)heap_ptr;
    heap_ptr += size;
    *heap_ptr = '\0';
    heap_ptr++;
    return ptr;
}

void tiny_free_all() {
    heap_ptr = (uint8_t*)HEAP_START;
}


void init_api(void){
    os_api=(os_api_t*) OS_API_ADDR;
}


void init_program(){
    tiny_free_all();
}


int str_eq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}

void str_left(char* dest, const char* src, int n) {
    int i = 0;
    while (i < n && src[i]) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

void str_right(char* dest, const char* src, int n) {
    int len = strlen(src);
    int start = (len > n) ? (len - n) : 0;
    int i = 0;
    while (src[start]) {
        dest[i++] = src[start++];
    }
    dest[i] = '\0';
}

void str_mid(char* dest, const char* src, int start, int len) {
    int i = 0;
    int pos = start - 1; // BASIC uses 1-based indexing
    if (pos < 0) pos = 0;
    while (i < len && src[pos]) {
        dest[i++] = src[pos++];
    }
    dest[i] = '\0';
}

char str_chr(int code) {
    return (char)(code & 0xFF);
}

int str_asc(const char* s) {
    return s && s[0] ? (unsigned char)s[0] : 0;
}

int str_instr(const char* haystack, const char* needle) {
    if (!haystack || !needle) return 0;
    const char* p = haystack;
    int pos = 1; // BASIC uses 1-based indexing
    while (*p) {
        const char* h = p;
        const char* n = needle;
        while (*h && *n && *h == *n) {
            h++; n++;
        }
        if (!*n) return pos;
        p++; pos++;
    }
    return 0;
}

int math_abs(int x) {
    return x < 0 ? -x : x;
}

int math_sqr(int x) {
    if (x < 0) return 0;
    if (x == 0) return 0;
    // Simple integer square root
    int result = 0;
    int bit = 1 << 14; // Start with a reasonable bit
    while (bit > x) bit >>= 2;
    while (bit != 0) {
        if (x >= result + bit) {
            x -= result + bit;
            result = (result >> 1) + bit;
        } else {
            result >>= 1;
        }
        bit >>= 2;
    }
    return result;
}

int math_int(int x) {
    return x; // Already integer in our implementation
}

int math_sgn(int x) {
    if (x > 0) return 1;
    if (x < 0) return -1;
    return 0;
}

static int print_tab_pos = 0;

int starts_with(const char* str, const char* prefix) {
    while (*prefix) {
        if (*str != *prefix) return 0;
        str++;
        prefix++;
    }
    return 1;
}

char* skip_spaces(char* s) {
    while (*s == ' ') s++;
    return s;
}

void handle_print(char* args) {
    args = skip_spaces(args);
    
    if (starts_with(args, "TAB(")) {
        args += 4;
        int pos = parse_solve_expression(args);
        
        while (*args && *args != ')') args++;
        if (*args == ')') args++;
        args = skip_spaces(args);
        
        while (print_tab_pos < pos) {
            os_api->print_shellc(' ');
            print_tab_pos++;
        }
        
        if (*args == ';') args = skip_spaces(args + 1);
    }
    
    if (*args) {
        os_api->print_shell(args);
        print_tab_pos += strlen(args);
    }
    
    os_api->print_shellc('\n');
    print_tab_pos = 0;
}


void print_in_line(char* args){
    args = skip_spaces(args);
    os_api->print_shell(args);
}

void handle_end(void) {
    os_api->print_shell("[Program ended]\n");
}


uint8_t push_to_return_stack(uint8_t line_number){
    if(return_stack_pointr>=100){
        return 0;
    }
    returnaddresses[return_stack_pointr++]=line_number;
    return 1;
}

uint8_t pop_return_stack(){
    return_stack_pointr--;    
    return returnaddresses[return_stack_pointr];
}


void get_word_safe(char* line, char* dest_buffer, size_t max_len){
    int j = 0;
    while (*line && *line != '\n' && *line != '\0' && *line != ' ' && j < max_len - 1) {
        dest_buffer[j++] = *line++;
    }
    dest_buffer[j] = '\0';
}



static int parse_factor() {
    expr_ptr = skip_spaces(expr_ptr);
    
    if ((*expr_ptr >= 'A' && *expr_ptr <= 'Z') || (*expr_ptr >= 'a' && *expr_ptr <= 'z')) {
        char func_name[10];
        int i = 0;
        char* start = expr_ptr;
        
        while ((*expr_ptr >= 'A' && *expr_ptr <= 'Z') || (*expr_ptr >= 'a' && *expr_ptr <= 'z')) {
            if (i < 9) func_name[i++] = *expr_ptr;
            expr_ptr++;
        }
        func_name[i] = '\0';
        
        expr_ptr = skip_spaces(expr_ptr);
        
        if (*expr_ptr == '(') {
            expr_ptr++;
            int arg = parse_expr();
            expr_ptr = skip_spaces(expr_ptr);
            if (*expr_ptr == ')') expr_ptr++;
            
            if (str_eq(func_name, "ABS") || str_eq(func_name, "abs")) return math_abs(arg);
            if (str_eq(func_name, "SQR") || str_eq(func_name, "sqr")) return math_sqr(arg);
            if (str_eq(func_name, "INT") || str_eq(func_name, "int")) return math_int(arg);
            if (str_eq(func_name, "SGN") || str_eq(func_name, "sgn")) return math_sgn(arg);
            if (str_eq(func_name, "RND") || str_eq(func_name, "rnd")) return get_random(arg);
            if (str_eq(func_name, "ASC") || str_eq(func_name, "asc")) {
                if (arg >= 0 && arg < 26) {
                    return str_asc(get_string_var('A' + arg));
                }
                return 0;
            }
            return arg;
        }
        
        expr_ptr = start;
    }
    
    if (*expr_ptr == '(') {
        expr_ptr++;
        int val = parse_expr();
        expr_ptr = skip_spaces(expr_ptr);
        if (*expr_ptr == ')') expr_ptr++;
        return val;
    }

    if (*expr_ptr >= '0' && *expr_ptr <= '9') {
        char num_buf[32];
        int i = 0;
        int has_decimal = 0;
        
        while ((*expr_ptr >= '0' && *expr_ptr <= '9') || *expr_ptr == '.') {
            if (*expr_ptr == '.') has_decimal = 1;
            if (i < 31) num_buf[i++] = *expr_ptr;
            expr_ptr++;
        }
        num_buf[i] = '\0';
        
        if (has_decimal) {
            return str_to_fixed(num_buf);
        } else {
            return str_to_int(num_buf);
        }
    }

    if (*expr_ptr >= 'a' && *expr_ptr <= 'z') {
        int val = int_vars[*expr_ptr - 'a'];
        expr_ptr++;
        return val;
    }

    if (*expr_ptr >= 'A' && *expr_ptr <= 'Z') {
        char* s = get_string_var(*expr_ptr);
        expr_ptr++;
        return (s && s[0]) ? (uint8_t)s[0] : 0;
    }

    return 0;
}

static int parse_term() {
    int val = parse_factor();
    expr_ptr = skip_spaces(expr_ptr);
    while (*expr_ptr == '*' || *expr_ptr == '/' || *expr_ptr == '%') {
        char op = *expr_ptr++;
        int next_val = parse_factor();
        
        int is_fixed = (val >= FIXED_SCALE || val <= -FIXED_SCALE || 
                       next_val >= FIXED_SCALE || next_val <= -FIXED_SCALE);
        
        if (op == '*') {
            if (is_fixed) {
                val = fixed_mul(val, next_val);
            } else {
                val *= next_val;
            }
        }
        else if (op == '/') {
            if (next_val != 0) {
                if (is_fixed) {
                    val = fixed_div(val, next_val);
                } else {
                    val = val / next_val;
                }
            } else {
                val = 0;
            }
        }
        else if (op == '%') {
            val = (next_val != 0) ? val % next_val : 0;
        }
        expr_ptr = skip_spaces(expr_ptr);
    }
    return val;
}

static int parse_expr() {
    int val = parse_term();
    expr_ptr = skip_spaces(expr_ptr);
    os_api->print_shell(expr_ptr);
    while (*expr_ptr == '+' || *expr_ptr == '-') {
        char op = *expr_ptr++;
        int next_val = parse_term();
        if (op == '+') val += next_val;
        else val -= next_val;
        expr_ptr = skip_spaces(expr_ptr);
    }
    char s[100];
    int_to_str(val,s);
    os_api->print_shell(s);
    return val;
}

uint32_t parse_solve_expression(char* expression) {
    if (expression[0] >= 'A' && expression[0] <= 'Z' && (expression[1] == '=' || expression[1] == ' ')) {
        
         char left_var = expression[0];
         char* remaining = skip_spaces(expression + 1);
         if (*remaining == '=') {
             remaining = skip_spaces(remaining + 1);
             if (*remaining == '\"') {
                 char literal[64];
                 int i = 0; remaining++;
                 while (*remaining && *remaining != '\"' && i < 63) literal[i++] = *remaining++;
                 literal[i] = '\0';
                 return str_eq(get_string_var(left_var), literal);
             } else if (*remaining >= 'A' && *remaining <= 'Z') {
                 return str_eq(get_string_var(left_var), get_string_var(*remaining));
             }
         }
    }

    expr_ptr = expression;
    int val = parse_expr();
    
    expr_ptr = skip_spaces(expr_ptr);
    if (*expr_ptr == '>' || *expr_ptr == '<' || *expr_ptr == '=') {
        char op = *expr_ptr++;
        int next_val = parse_expr();
        if (op == '>') return val > next_val;
        if (op == '<') return val < next_val;
        if (op == '=') return val == next_val;
    }
    
    return val;
}

uint32_t get_random(uint32_t max) {
    return os_api->get_random(max);
}

void busy_delay(uint32_t count) {
    os_api->busy_delay(count);
}

void handle_poke(char* args) {
    char addr_str[16];
    get_word_safe(args, addr_str, 16);
    args = skip_spaces(args + strlen(addr_str));
    uint32_t addr = (uint32_t)str_to_int(addr_str);
    uint8_t val = (uint8_t)str_to_int(args);
    *(volatile uint8_t*)addr = val;
}

int handle_peek(char* args) {
    char var = args[0];
    args = skip_spaces(args + 1);
    uint32_t addr = (uint32_t)str_to_int(args);
    if (var >= 'a' && var <= 'z') {
        int_vars[var - 'a'] = *(volatile uint8_t*)addr;
    }
    return 0;
}

void handle_locate(char* args) {
    char row_str[16];
    get_word_safe(args, row_str, 16);
    args = skip_spaces(args + strlen(row_str));
    int row = str_to_int(row_str);
    int col = str_to_int(args);
    os_api->move_cursor(row, col);
}

void handle_color(char* args) {
    uint8_t color = (uint8_t)str_to_int(args);
    os_api->set_color(color);
}

void handle_beep(char* args) {
    char freq_str[16];
    get_word_safe(args, freq_str, 16);
    args = skip_spaces(args + strlen(freq_str));
    int freq = str_to_int(freq_str);
    int dur = str_to_int(args);
    os_api->beep(freq, dur);
}

void handle_pset(char* args) {
    char x_str[16];
    get_word_safe(args, x_str, 16);
    args = skip_spaces(args + strlen(x_str));
    char y_str[16];
    get_word_safe(args, y_str, 16);
    args = skip_spaces(args + strlen(y_str));
    int x = str_to_int(x_str);
    int y = str_to_int(y_str);
    int color = str_to_int(args);
    os_api->draw_pixel(x, y, (uint8_t)color);
}

void handle_play(char* args) {
    args = skip_spaces(args);
    if (*args == '\"') args++;
    while (*args && *args != '\"') {
        char note = *args++;
        int freq = 0;
        switch (note) {
            case 'C': freq = 261; break;
            case 'D': freq = 293; break;
            case 'E': freq = 329; break;
            case 'F': freq = 349; break;
            case 'G': freq = 392; break;
            case 'A': freq = 440; break;
            case 'B': freq = 493; break;
            case ' ': os_api->busy_delay(100); continue;
            default: continue;
        }
        os_api->beep(freq, 100);
        os_api->busy_delay(50);
    }
}

void handle_line(char* args) {
    char x1_s[16], y1_s[16], x2_s[16], y2_s[16];
    get_word_safe(args, x1_s, 16); args = skip_spaces(args + strlen(x1_s));
    get_word_safe(args, y1_s, 16); args = skip_spaces(args + strlen(y1_s));
    get_word_safe(args, x2_s, 16); args = skip_spaces(args + strlen(x2_s));
    get_word_safe(args, y2_s, 16); args = skip_spaces(args + strlen(y2_s));
    int x1 = str_to_int(x1_s), y1 = str_to_int(y1_s);
    int x2 = str_to_int(x2_s), y2 = str_to_int(y2_s);
    int color = str_to_int(args);

    int dx = (x2 > x1) ? (x2 - x1) : (x1 - x2);
    int dy = (y2 > y1) ? (y2 - y1) : (y1 - y2);
    int steps = (dx > dy) ? dx : dy;
    if (steps == 0) return;

    for (int i = 0; i <= steps; i++) {
        int px = x1 + (x2 - x1) * i / steps;
        int py = y1 + (y2 - y1) * i / steps;
        os_api->draw_pixel(px, py, (uint8_t)color);
    }
}

void handle_rect(char* args) {
    char x1_s[16], y1_s[16], x2_s[16], y2_s[16];
    get_word_safe(args, x1_s, 16); args = skip_spaces(args + strlen(x1_s));
    get_word_safe(args, y1_s, 16); args = skip_spaces(args + strlen(y1_s));
    get_word_safe(args, x2_s, 16); args = skip_spaces(args + strlen(x2_s));
    get_word_safe(args, y2_s, 16); args = skip_spaces(args + strlen(y2_s));
    int x1 = str_to_int(x1_s), y1 = str_to_int(y1_s);
    int x2 = str_to_int(x2_s), y2 = str_to_int(y2_s);
    int color = str_to_int(args);

    for (int x = x1; x <= x2; x++) {
        os_api->draw_pixel(x, y1, (uint8_t)color);
        os_api->draw_pixel(x, y2, (uint8_t)color);
    }
    for (int y = y1; y <= y2; y++) {
        os_api->draw_pixel(x1, y, (uint8_t)color);
        os_api->draw_pixel(x2, y, (uint8_t)color);
    }
}

void handle_circle(char* args) {
    char xc_s[16], yc_s[16], r_s[16];
    get_word_safe(args, xc_s, 16); args = skip_spaces(args + strlen(xc_s));
    get_word_safe(args, yc_s, 16); args = skip_spaces(args + strlen(yc_s));
    get_word_safe(args, r_s, 16); args = skip_spaces(args + strlen(r_s));
    int xc = str_to_int(xc_s), yc = str_to_int(yc_s), r = str_to_int(r_s);
    int color = str_to_int(args);

    int x = 0, y = r;
    int d = 3 - 2 * r;
    while (y >= x) {
        os_api->draw_pixel(xc+x, yc+y, color); os_api->draw_pixel(xc-x, yc+y, color);
        os_api->draw_pixel(xc+x, yc-y, color); os_api->draw_pixel(xc-x, yc-y, color);
        os_api->draw_pixel(xc+y, yc+x, color); os_api->draw_pixel(xc-y, yc+x, color);
        os_api->draw_pixel(xc+y, yc-x, color); os_api->draw_pixel(xc-y, yc-x, color);
        x++;
        if (d > 0) { y--; d = d + 4 * (x - y) + 10; }
        else d = d + 4 * x + 6;
    }
}

void handle_restore(void) {
    data_line_ptr = 0;
    data_char_ptr = NULL;
}

void handle_read_data(char* args) {
    char var = args[0];
    
    while (data_line_ptr < program_line_count) {
        if (data_char_ptr == NULL) {
            char* line = program_lines[data_line_ptr];
            line = skip_spaces(line);
            if (starts_with(line, "DATA")) {
                data_char_ptr = skip_spaces(line + 4);
            } else {
                data_line_ptr++;
                continue;
            }
        }

        // Extract value from data_char_ptr
        char val_str[64];
        int i = 0;
        while (*data_char_ptr && *data_char_ptr != ',' && i < 63) {
            val_str[i++] = *data_char_ptr++;
        }
        val_str[i] = '\0';
        if (*data_char_ptr == ',') data_char_ptr++;
        data_char_ptr = skip_spaces(data_char_ptr);
        
        if (*data_char_ptr == '\0') data_char_ptr = NULL; // Move to next line next time

        if (var >= 'a' && var <= 'z') int_vars[var - 'a'] = str_to_int(val_str);
        else if (var >= 'A' && var <= 'Z') set_string_var(var, val_str);
        return;
    }
}

void handle_dim(char* args) {
    char var = args[0];
    args = skip_spaces(args + 1);
    int size = str_to_int(args);
    if (array_count < MAX_ARRAYS) {
        arrays[array_count].var = var;
        arrays[array_count].size = (size < ARRAY_SIZE) ? size : ARRAY_SIZE;
        array_count++;
    }
}

void handle_list() {
    for (int i = 0; i < program_line_count; i++) {
        os_api->print_shell(program_lines[i]);
        os_api->print_shellc('\n');
    }
}

void handle_read(char* args) {
    char var = args[0];
    args = skip_spaces(args + 1);
    char filename[16];
    get_word_safe(args, filename, 16);
    
    uint8_t file_buf[SECTOR_SIZE];
    if (os_api->load_file(filename, file_buf) == 0) {
        if (var >= 'a' && var <= 'z') {
            int_vars[var - 'a'] = str_to_int((char*)file_buf);
        } else if (var >= 'A' && var <= 'Z') {
            set_string_var(var, (char*)file_buf);
        }
    }
}

void handle_write(char* args) {
    char filename[16];
    get_word_safe(args, filename, 16);
    args = skip_spaces(args + strlen(filename));
    char var = args[0];
    
    char file_buf[SECTOR_SIZE];
    if (var >= 'a' && var <= 'z') {
        int_to_str(int_vars[var - 'a'], file_buf);
    } else if (var >= 'A' && var <= 'Z') {
        char* s = get_string_var(var);
        size_t i = 0;
        while (s[i] && i < SECTOR_SIZE - 1) {
            file_buf[i] = s[i];
            i++;
        }
        file_buf[i] = '\0';
    }
    
    os_api->save_file(filename, (uint8_t*)file_buf);
}

void handle_left(char* args) {
    char dest = args[0];
    args = skip_spaces(args + 1);
    char src = args[0];
    args = skip_spaces(args + 1);
    int len = str_to_int(args);
    char* s = get_string_var(src);
    char tmp[STRING_BLOCK_SIZE];
    int i = 0;
    for (; i < len && s[i]; i++) tmp[i] = s[i];
    tmp[i] = '\0';
    set_string_var(dest, tmp);
}

void handle_set_array(char* args) {
    char var = args[0];
    args = skip_spaces(args + 1);
    int index = str_to_int(args);
    while (*args && *args != ' ') args++;
    int val = str_to_int(skip_spaces(args));
    for (int i = 0; i < array_count; i++) {
        if (arrays[i].var == var && index < arrays[i].size) {
            arrays[i].data[index] = val;
            break;
        }
    }
}

void handle_get_array(char* args) {
    char dest = args[0];
    args = skip_spaces(args + 1);
    char var = args[0];
    args = skip_spaces(args + 1);
    int index = str_to_int(args);
    if (dest >= 'a' && dest <= 'z') {
        for (int i = 0; i < array_count; i++) {
            if (arrays[i].var == var && index < arrays[i].size) {
                int_vars[dest - 'a'] = arrays[i].data[index];
                break;
            }
        }
    }
}

void execute_program(void) {
    uint8_t execution_status=1;
    int function_count=0;
    int program_counter=-1; 
    uint8_t if_flag=0;
    uint8_t must_execute=1;
    
    while (must_execute) {
        program_counter++;
        if(program_counter>=program_line_count){
            break;
        }
        char* line = program_lines[program_counter];
        line = skip_spaces(line);

            char command[16];
            int j = 0;
            while (*line && *line != ' ' && *line != ':' && j < 15) {
                command[j++] = *line++;
            }
            command[j] = '\0';
            line = skip_spaces(line);


         if (str_eq(command, "FUNC")) {
            execution_status=0;
            for(int k=0;k<16;k++){
                if(line[k]==' ' || line[k]=='\n' || line[k]=='\0'){
                    functions[function_count].name[k]='\0';
                    break;
                }
                functions[function_count].name[k]=line[k];
            }
            functions[function_count].line=program_counter;
            functions[function_count].name[15]='\0';
            function_count++;
            continue;
        }
         else if (str_eq(command, "ENDFUNC")) {
             if(execution_status==1){
                program_counter=pop_return_stack();
            }
            else if(execution_status==0){
                execution_status=1;
            }
            continue;
        }


        


        if(str_eq(command,"IF")){
            uint32_t result=parse_solve_expression(line);
            char c[12];
            int_to_str(result,c);
            if(result>=1){
                execution_status=1;
            }else{
                execution_status=0;
            }
            continue;
        }
        else if(str_eq(command,"ELIF")){
            if(execution_status==1){
                execution_status=0;
                if_flag=1;
                continue;
            }

            if(if_flag==1){
                continue;
            }
            uint32_t result=parse_solve_expression(line);
            char c[12];
            int_to_str(result,c);
            if(result>=1){
                execution_status=1;
            }else{
                execution_status=0;
            }
            continue;
        }
          else  if(str_eq(command,"ELSE")){
            if(execution_status==1){
                if_flag=1;
                execution_status=0;
                continue;
            }

            if(if_flag==1){
                continue;
            }
            execution_status=1;
            continue;
        }
        else if(str_eq(command,"ENDIF")){
            execution_status=1;
            if_flag=0;
            continue;
        }


        if(execution_status==0){
            continue;
        }

        // === Match and execute ===
        if (str_eq(command, "PRINT")) {
            handle_print(line);
        }
        else if (str_eq(command, "NEWPAGE")) {
            os_api->save_vga(0);
            os_api->screen_clear_shell();
        }
         else if (str_eq(command, "PARSEINT")) {
           char* var=line;
           char* value=skip_spaces(line+1);
            if(*var>='a' && *var<='z') {
                if(value[0]>='A' && value[0]<='Z'){
                    int_vars[*var - 'a']=str_to_int(get_string_var(value[0]));
                }
            }
        }
        else if (str_eq(command, "TOSTRING")) {
           char* var=line;
           char* value=skip_spaces(line+1);
            if(*var>='A' && *var<='Z') {
                if(value[0]>='a' && value[0]<='z'){
                    char c[20];
                    int_to_str(int_vars[value[0]-'a'],c);
                    set_string_var(*var,c);
                }
            }
        }
         else if (str_eq(command, "LOADPAGE")) {
            os_api->load_vga(0);
        }
        else  if (str_eq(command, "PRINTI")) {
            print_in_line(line);
        }
        else if (str_eq(command, "EVAL")) {
            char* var=line;
            char* value=skip_spaces(line+1);
            if(*var>='a' && *var<='z') {
                int_vars[*var - 'a']=parse_solve_expression(value);
            }
        }
        else if (str_eq(command, "PRINTEXPR")) {
            char* var=line;
            uint32_t num=parse_solve_expression(var);
            char c[10];
            int_to_str(num,c);
            handle_print(c);
        }
        else if (str_eq(command, "GOTO")) {
            int target = str_to_int(line);
            for (int i = 0; i < program_line_count; i++) {
                if (line_numbers[i] == target) {
                    program_counter = i - 1;
                    break;
                }
            }
        }
        else if (str_eq(command, "CLS")) {
            os_api->screen_clear_shell();
        }
        else if (str_eq(command, "DELAY")) {
            busy_delay(str_to_int(line));
        }
        else if (str_eq(command, "RANDOM")) {
            char var = line[0];
            char* max_str = skip_spaces(line + 1);
            if (var >= 'a' && var <= 'z') {
                int_vars[var - 'a'] = get_random(str_to_int(max_str));
            }
        }
        else if (str_eq(command, "COMMENT") || str_eq(command, "REM")) {
        }
        else if (str_eq(command, "INPUT")) {
            char var = line[0];
            line= skip_spaces(line + 1);
            os_api->print_shell(line);
            char input[64];
            for(int i=0;i<63;i++){
                char c=os_api->keyboard_read();
                if(c=='\n' || c=='\r'){
                    input[i]='\0';
                    break;
                }
                input[i]=c;
                os_api->print_shellc(c);
            }
            input[63]='\0';
            if(var>='A' && var<='Z'){
                set_string_var(var, input);
            }

            if(var>='a' && var<='z'){
                int_vars[var-'a']=str_to_int(input);
            }
            
            os_api->print_shellc('\n');
        }
        else if(str_eq(command,"CALL")){
            char word[16];
            get_word_safe(line, word, 16);
            for(int x=0;x<FUNCTIONS_NUM;x++){
                if(str_eq(word,functions[x].name)){
                    push_to_return_stack(program_counter);
                    program_counter=functions[x].line;
                    break;
                }
            }
        }
        else if (str_eq(command, "LET")) {
            char* var=line;
            char* value=skip_spaces(line+1);
            if(*var>='A' && *var<='Z') {
                char* val_ptr = value;
                if (*val_ptr == '\"') {
                    val_ptr++;
                    char tmp[STRING_BLOCK_SIZE];
                    int i = 0;
                    while (*val_ptr && *val_ptr != '\"' && i < STRING_BLOCK_SIZE-1) tmp[i++] = *val_ptr++;
                    tmp[i] = '\0';
                    set_string_var(*var, tmp);
                } else if (*val_ptr >= 'A' && *val_ptr <= 'Z') {
                    char src1 = *val_ptr;
                    char* next = skip_spaces(val_ptr + 1);
                    if (*next == '+') {
                        char* src2_ptr = skip_spaces(next + 1);
                        if (*src2_ptr >= 'A' && *src2_ptr <= 'Z') {
                            char combined[STRING_BLOCK_SIZE];
                            char* s1 = get_string_var(src1);
                            char* s2 = get_string_var(*src2_ptr);
                            int i = 0;
                            while (*s1 && i < STRING_BLOCK_SIZE-1) combined[i++] = *s1++;
                            while (*s2 && i < STRING_BLOCK_SIZE-1) combined[i++] = *s2++;
                            combined[i] = '\0';
                            set_string_var(*var, combined);
                        }
                    } else {
                        set_string_var(*var, value);
                    }
                }
            
            }
            if(*var>='a' && *var<='z') {
                int_vars[*var - 'a']=str_to_int(value);
            }
        }
        else if (str_eq(command, "PRINTVAR")) {
            if(line[0] >= 'A' && line[0] <= 'Z') {
                handle_print(get_string_var(line[0]));
            }
            else if(line[0] >= 'a' && line[0] <= 'z'){
                 char c[12];
                int_to_str(int_vars[line[0]-'a'],c);
                handle_print(c);
            }
        }
        else if (str_eq(command, "POKE")) {
            handle_poke(line);
        }
        else if (str_eq(command, "PEEK")) {
            handle_peek(line);
        }
        else if (str_eq(command, "LOCATE")) {
            handle_locate(line);
        }
        else if (str_eq(command, "COLOR")) {
            handle_color(line);
        }
        else if (str_eq(command, "FOR")) {
            char var = line[0];
            char* start_str = skip_spaces(line + 1);
            char end_word[16];
            get_word_safe(start_str, end_word, 16);
            int start_val = str_to_int(end_word);
            char* to_ptr = skip_spaces(start_str + strlen(end_word));
            if (to_ptr[0] == 'T' && to_ptr[1] == 'O') to_ptr = skip_spaces(to_ptr + 2);
            int end_val = str_to_int(to_ptr);
            
            if (var >= 'a' && var <= 'z') {
                int_vars[var - 'a'] = start_val;
                if (for_stack_ptr < FOR_STACK_SIZE) {
                    for_stack[for_stack_ptr].var = var;
                    for_stack[for_stack_ptr].end = end_val;
                    for_stack[for_stack_ptr].line = program_counter;
                    for_stack_ptr++;
                }
            }
        }
        else if (str_eq(command, "NEXT")) {
            char var = line[0];
            if (for_stack_ptr > 0 && for_stack[for_stack_ptr-1].var == var) {
                int_vars[var - 'a']++;
                if (int_vars[var - 'a'] <= for_stack[for_stack_ptr-1].end) {
                    program_counter = for_stack[for_stack_ptr-1].line;
                } else {
                    for_stack_ptr--;
                }
            }
        }
        else if (str_eq(command, "CHR$")) {
            char var = line[0];
            char* code_str = skip_spaces(line + 1);
            int code = str_to_int(code_str);
            char s[2] = {(char)code, 0};
            set_string_var(var, s);
        }
        else if (str_eq(command, "ASC")) {
            char var = line[0];
            char* str_var_ptr = skip_spaces(line + 1);
            char str_var = str_var_ptr[0];
            if (var >= 'a' && var <= 'z' && str_var >= 'A' && str_var <= 'Z') {
                char* s = get_string_var(str_var);
                int_vars[var - 'a'] = (uint8_t)s[0];
            }
        }
        else if (str_eq(command, "INKEY$")) {
            char var = line[0];
            char c = os_api->keyboard_getchar();
            char s[2] = {c, 0};
            set_string_var(var, s);
        }
        else if (str_eq(command, "BEEP")) {
            handle_beep(line);
        }
        else if (str_eq(command, "SCREEN")) {
            os_api->set_vga_mode((uint8_t)str_to_int(line));
        }
        else if (str_eq(command, "PSET")) {
            handle_pset(line);
        }
        else if (str_eq(command, "LINE")) {
            handle_line(line);
        }
        else if (str_eq(command, "RECT")) {
            handle_rect(line);
        }
        else if (str_eq(command, "CIRCLE")) {
            handle_circle(line);
        }
        else if (str_eq(command, "READ")) {
            handle_read_data(line);
        }
        else if (str_eq(command, "RESTORE")) {
            handle_restore();
        }
        else if (str_eq(command, "DIM")) {
            handle_dim(line);
        }
        else if (str_eq(command, "LEN")) {
            char var = line[0];
            char* str_var_ptr = skip_spaces(line + 1);
            if (var >= 'a' && var <= 'z' && str_var_ptr[0] >= 'A' && str_var_ptr[0] <= 'Z') {
                int_vars[var - 'a'] = strlen(get_string_var(str_var_ptr[0]));
            }
        }
        else if (str_eq(command, "MID$")) {
            char dest = line[0];
            char* s_ptr = skip_spaces(line + 1);
            char src = s_ptr[0];
            s_ptr = skip_spaces(s_ptr + 1);
            int start = str_to_int(s_ptr);
            while (*s_ptr && *s_ptr != ' ') s_ptr++;
            int len = str_to_int(skip_spaces(s_ptr));
            char* s = get_string_var(src);
            char tmp[STRING_BLOCK_SIZE];
            int i = 0;
            for (; i < len && s[start + i]; i++) tmp[i] = s[start + i];
            tmp[i] = '\0';
            set_string_var(dest, tmp);
        }
        else if (str_eq(command, "LEFT$")) {
            handle_left(line);
        }
        else if (str_eq(command, "SETARRAY")) {
            handle_set_array(line);
        }
        else if (str_eq(command, "GETARRAY")) {
            handle_get_array(line);
        }
        else if (str_eq(command, "PLAY")) {
            handle_play(line);
        }
        else if (str_eq(command, "FILES")) {
            os_api->list_files();
        }
        else if (str_eq(command, "KILL")) {
            os_api->delete_file(line);
        }
        else if (str_eq(command, "SAVE")) {
            char buffer[4096];
            int offset = 0;
            for (int i = 0; i < program_line_count; i++) {
                char line_num_str[10];
                int_to_str(line_numbers[i], line_num_str);
                int l = strlen(line_num_str);
                for (int j = 0; j < l; j++) buffer[offset++] = line_num_str[j];
                buffer[offset++] = ' ';
                l = strlen(program_lines[i]);
                for (int j = 0; j < l; j++) buffer[offset++] = program_lines[i][j];
                buffer[offset++] = '\n';
            }
            buffer[offset] = '\0';
            os_api->save_file(line, (uint8_t*)buffer);
        }
        else if (str_eq(command, "LOAD")) {
            char buffer[4096];
            if (os_api->load_file(line, (uint8_t*)buffer) == 0) {
                prepare_environment();
                parse_program(buffer);
            }
        }
        else if (str_eq(command, "WHILE")) {
            if (parse_solve_expression(line)) {
                if (while_stack_ptr < WHILE_STACK_SIZE) {
                    while_stack[while_stack_ptr].line = program_counter;
                    int l = 0;
                    while (line[l] && l < 63) { while_stack[while_stack_ptr].condition[l] = line[l]; l++; }
                    while_stack[while_stack_ptr].condition[l] = '\0';
                    while_stack_ptr++;
                }
            } else {
                int depth = 1;
                while (depth > 0 && program_counter < program_line_count - 1) {
                    program_counter++;
                    char* next_line = skip_spaces(program_lines[program_counter]);
                    if (starts_with(next_line, "WHILE")) depth++;
                    if (starts_with(next_line, "WEND")) depth--;
                }
            }
        }
        else if (str_eq(command, "WEND")) {
            if (while_stack_ptr > 0) {
                if (parse_solve_expression(while_stack[while_stack_ptr-1].condition)) {
                    program_counter = while_stack[while_stack_ptr-1].line;
                } else {
                    while_stack_ptr--;
                }
            }
        }
        else if (str_eq(command, "LEN")) {
            char var = line[0];
            char* s_var_ptr = skip_spaces(line + 1);
            if (var >= 'a' && var <= 'z' && *s_var_ptr >= 'A' && *s_var_ptr <= 'Z') {
                int_vars[var - 'a'] = strlen(get_string_var(*s_var_ptr));
            }
        }
        else if (str_eq(command, "VAL")) {
            char var = line[0];
            char* s_var_ptr = skip_spaces(line + 1);
            if (var >= 'a' && var <= 'z' && *s_var_ptr >= 'A' && *s_var_ptr <= 'Z') {
                int_vars[var - 'a'] = str_to_int(get_string_var(*s_var_ptr));
            }
        }
        else if (str_eq(command, "STR$")) {
            char var = line[0];
            char* i_var_ptr = skip_spaces(line + 1);
            if (var >= 'A' && var <= 'Z' && *i_var_ptr >= 'a' && *i_var_ptr <= 'z') {
                char tmp[16];
                int_to_str(int_vars[*i_var_ptr - 'a'], tmp);
                set_string_var(var, tmp);
            }
        }
        else if (str_eq(command, "LIST")) {
            handle_list();
        }
        else if (str_eq(command, "NEW")) {
            prepare_environment();
            program_line_count = 0;
        }
        else if (str_eq(command, "RUN")) {
            program_counter = -1; // Restart
            handle_restore();
        }
        else if (str_eq(command, "READFILE")) {
            handle_read(line);
        }
        else if (str_eq(command, "WRITEFILE")) {
            handle_write(line);
        }
        else if (str_eq(command, "END")) {
            handle_end();
            break; 
        }
        else if (str_eq(command, "RIGHT$")) {
            char dest = line[0];
            char* s_ptr = skip_spaces(line + 1);
            char src = s_ptr[0];
            s_ptr = skip_spaces(s_ptr + 1);
            int len = parse_solve_expression(s_ptr);
            char tmp[STRING_BLOCK_SIZE];
            str_right(tmp, get_string_var(src), len);
            set_string_var(dest, tmp);
        }
        else if (str_eq(command, "CHR$")) {
            char dest = line[0];
            char* val_ptr = skip_spaces(line + 1);
            int code = parse_solve_expression(val_ptr);
            char tmp[2];
            tmp[0] = str_chr(code);
            tmp[1] = '\0';
            set_string_var(dest, tmp);
        }
        else if (str_eq(command, "ASC")) {
            char dest = line[0];
            char* s_ptr = skip_spaces(line + 1);
            if (dest >= 'a' && dest <= 'z' && *s_ptr >= 'A' && *s_ptr <= 'Z') {
                int_vars[dest - 'a'] = str_asc(get_string_var(*s_ptr));
            }
        }
        else if (str_eq(command, "INSTR")) {
            char dest = line[0];
            char* s_ptr = skip_spaces(line + 1);
            char haystack = s_ptr[0];
            s_ptr = skip_spaces(s_ptr + 1);
            char needle = s_ptr[0];
            if (dest >= 'a' && dest <= 'z' && haystack >= 'A' && haystack <= 'Z' && needle >= 'A' && needle <= 'Z') {
                int_vars[dest - 'a'] = str_instr(get_string_var(haystack), get_string_var(needle));
            }
        }
        else if (str_eq(command, "ON")) {
            char* expr_end = line;
            while (*expr_end && *expr_end != ' ') expr_end++;
            char expr_buf[64];
            int i = 0;
            char* p = line;
            while (p < expr_end && i < 63) expr_buf[i++] = *p++;
            expr_buf[i] = '\0';
            
            int index = parse_solve_expression(expr_buf);
            char* rest = skip_spaces(expr_end);
            
            int is_gosub = 0;
            if (starts_with(rest, "GOSUB")) {
                is_gosub = 1;
                rest = skip_spaces(rest + 5);
            } else if (starts_with(rest, "GOTO")) {
                rest = skip_spaces(rest + 4);
            }
            
            int line_nums[10];
            int count = 0;
            while (*rest && count < 10) {
                char num_buf[16];
                i = 0;
                while (*rest && *rest != ',' && *rest != ' ' && i < 15) num_buf[i++] = *rest++;
                num_buf[i] = '\0';
                line_nums[count++] = str_to_int(num_buf);
                rest = skip_spaces(rest);
                if (*rest == ',') rest = skip_spaces(rest + 1);
            }
            
            if (index >= 1 && index <= count) {
                int target = line_nums[index - 1];
                for (int j = 0; j < program_line_count; j++) {
                    if (line_numbers[j] == target) {
                        if (is_gosub) {
                            push_to_return_stack(program_counter);
                        }
                        program_counter = j - 1;
                        break;
                    }
                }
            }
        }
        else if (str_eq(command, "FPRINT")) {
            char var = line[0];
            if (var >= 'a' && var <= 'z') {
                char buf[32];
                fixed_to_str(int_vars[var - 'a'], buf);
                os_api->print_shell(buf);
                os_api->print_shellc('\n');
            }
        }
        else {
            os_api->print_shell("[Unknown command: ");
            os_api->print_shell(command);
            os_api->print_shell("]\n");
        }
        
    }
}

void clear_memory(uint32_t start_addr, uint32_t size) {
    uint8_t* ptr = (uint8_t*)start_addr;
    for (uint32_t i = 0; i < size; i++) ptr[i] = 0;
}
 
void prepare_environment(void) {
    clear_memory(PROGRAM_MEM_START, 4096);
    // clear_memory(TEXT_BUFFER_ADDR, 4096);
    clear_all_string_vars();
    for (int i = 0; i < MAX_LINES; i++) line_numbers[i] = 0;
    handle_restore();
    while_stack_ptr = 0;
    for_stack_ptr = 0;
    tiny_free_all();
}
 
void run(){

}


void load_test_program() {
    char* program = (char*)TEXT_BUFFER_ADDR;

    // A simple BASIC-like test program
    const char* test_code =
        "10 PRINT HELLO WORLD\n"
        "20 PRINT OKAY\n"
        "30 END\n";

    // Copy test program into memory
    int i = 0;
    while (test_code[i] != '\0') {
        program[i] = test_code[i];
        i++;
    }
    program[i] = '\0';  // null terminate
}

void graceful_exit(){
    clear_memory(TEXT_BUFFER_ADDR, 0x10000); 
}

void main(){
    init_api();
    init_program();
    prepare_environment();
    parse_program((char*)TEXT_BUFFER_ADDR);
    execute_program();
    graceful_exit();
}