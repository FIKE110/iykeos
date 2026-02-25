/*
 * TINYBASIC.c - A complete TinyBASIC interpreter for IYKE OS
 * Features: Structured programming, file management, graphics, calculations,
 *           variables, datatypes, functions - Turing complete
 */

#include <stddef.h>
#include <stdint.h>

/* ============================================================================
 * OS API STRUCTURE
 * ============================================================================ */

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
    void (*stack_push)(uint32_t);
    uint32_t (*stack_pop)(void);
    uint32_t (*stack_peek)(void);
    int (*stack_is_empty)(void);
    int (*stack_is_full)(void);
    void (*stack_clear)(void);
    uint8_t (*push_string_to_stack)(char* str);
    uint8_t (*pop_string_from_stack)(char* str);
    void (*reverse_string)(char* str);

} os_api_t;


#define OS_API_ADDR 0x5F0F0
static os_api_t* os_api;

/* ============================================================================
 * MEMORY LAYOUT AND CONSTANTS
 * Memory starts at 0xA00000 to avoid conflict with OS at 0xB00000
 * ============================================================================ */
#define PROGRAM_MEM_START   0xA00000
#define FILE_BUFFER_ADDR    0xA10000
#define VARIABLE_MEM_START  0xA20000
#define STRING_MEM_START    0xA30000
#define ARRAY_MEM_START     0xA40000
#define STACK_MEM_START     0xA50000
#define FUNCTION_MEM_START  0xA60000

#define MAX_LINES           500
#define MAX_LINE_LENGTH     120
#define MAX_STRING_VARS     52      // A-Z, A$-Z$
#define MAX_INT_VARS        52      // A-Z, A%-Z%
#define MAX_FLOAT_VARS      26      // A#-Z#
#define MAX_ARRAYS          20
#define MAX_ARRAY_SIZE      1000
#define MAX_STRING_LEN      256
#define MAX_FUNCTIONS       50
#define MAX_NESTING_LEVEL   32
#define MAX_GOSUB_DEPTH     50
#define MAX_FOR_DEPTH       50
#define MAX_WHILE_DEPTH     50
#define MAX_DATA_ITEMS      500

#define SECTOR_SIZE         512

/* ============================================================================
 * DATA TYPES
 * ============================================================================ */
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_ARRAY
} var_type_t;

typedef struct {
    char name[16];
    var_type_t type;
    union {
        int32_t i_val;
        float f_val;
    } value;
    char s_val[MAX_STRING_LEN];
} variable_t;

typedef struct {
    char name[16];
    int32_t data[MAX_ARRAY_SIZE];
    int size;
    int dimensions;
    int dim1_size;
    int dim2_size;
} array_t;

typedef struct {
    char name[32];
    int line_num;
    int return_line;
} function_t;

typedef struct {
    int line_num;
    char var;
    int32_t end_val;
    int32_t step;
} for_stack_entry_t;

typedef struct {
    int line_num;
    int eval_result;
} while_stack_entry_t;

typedef struct {
    int line_num;
} gosub_stack_entry_t;

typedef struct {
    char* line;
    int line_num;
} program_line_t;

/* ============================================================================
 * GLOBAL STATE
 * ============================================================================ */
static char (*program_lines)[MAX_LINE_LENGTH] = (char (*)[MAX_LINE_LENGTH])PROGRAM_MEM_START;
static int line_numbers[MAX_LINES];
static int program_line_count = 0;
static int current_line = 0;
static int running = 0;

// Variables
static int32_t int_vars[MAX_INT_VARS];
static float float_vars[MAX_FLOAT_VARS];
static char string_vars[MAX_STRING_VARS][MAX_STRING_LEN];
static uint8_t var_initialized[MAX_INT_VARS + MAX_FLOAT_VARS + MAX_STRING_VARS];

// Arrays
static array_t arrays[MAX_ARRAYS];
static int array_count = 0;

// Functions
static function_t functions[MAX_FUNCTIONS];
static int function_count = 0;

// Stacks
static for_stack_entry_t for_stack[MAX_FOR_DEPTH];
static int for_stack_ptr = 0;

static while_stack_entry_t while_stack[MAX_WHILE_DEPTH];
static int while_stack_ptr = 0;

static gosub_stack_entry_t gosub_stack[MAX_GOSUB_DEPTH];
static int gosub_stack_ptr = 0;

// DATA/READ
static char* data_items[MAX_DATA_ITEMS];
static int data_item_count = 0;
static int data_read_ptr = 0;

// IF/ELSE state
static int if_result_stack[MAX_NESTING_LEVEL];
static int if_stack_ptr = 0;
static int skip_to_endif = 0;

// Expression parser
static char* expr_ptr;

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */
static void init_api(void) {
    os_api = (os_api_t*)OS_API_ADDR;
}

static int str_eq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

static int starts_with(const char* str, const char* prefix) {
    while (*prefix) {
        if (*str != *prefix) return 0;
        str++;
        prefix++;
    }
    return 1;
}

static char* skip_spaces(char* s) {
    while (*s == ' ' || *s == '\t') s++;
    return s;
}

static size_t my_strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

static void my_strcpy(char* dest, const char* src) {
    while (*src) *dest++ = *src++;
    *dest = '\0';
}

static void my_strcat(char* dest, const char* src) {
    while (*dest) dest++;
    while (*src) *dest++ = *src++;
    *dest = '\0';
}

static int my_strcmp(const char* a, const char* b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return (*a - *b);
}

static int my_strncmp(const char* a, const char* b, int n) {
    int i = 0;
    while (i < n && *a && *b && *a == *b) {
        a++;
        b++;
        i++;
    }
    if (i == n) return 0;
    return (*a - *b);
}

static char my_toupper(char c) {
    if (c >= 'a' && c <= 'z') return c - 32;
    return c;
}

static void int_to_str(int32_t num, char* buf) {
    int i = 0;
    int sign = (num < 0) ? 1 : 0;
    if (sign) num = -num;
    
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

static int32_t str_to_int(const char* s) {
    int32_t result = 0;
    int sign = 1;
    
    while (*s == ' ' || *s == '\t') s++;
    
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') s++;
    
    while (*s >= '0' && *s <= '9') {
        result = result * 10 + (*s - '0');
        s++;
    }
    return result * sign;
}

static float str_to_float(const char* s) {
    float result = 0.0f;
    float frac = 0.0f;
    float divisor = 1.0f;
    int sign = 1;
    int in_frac = 0;
    
    while (*s == ' ' || *s == '\t') s++;
    
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') s++;
    
    while ((*s >= '0' && *s <= '9') || *s == '.') {
        if (*s == '.') {
            in_frac = 1;
            s++;
            continue;
        }
        if (in_frac) {
            frac = frac * 10 + (*s - '0');
            divisor *= 10;
        } else {
            result = result * 10 + (*s - '0');
        }
        s++;
    }
    
    return sign * (result + frac / divisor);
}

static void float_to_str(float f, char* buf, int decimals) {
    if (f < 0) {
        *buf++ = '-';
        f = -f;
    }
    
    int32_t whole = (int32_t)f;
    float frac = f - whole;
    
    int_to_str(whole, buf);
    while (*buf) buf++;
    
    *buf++ = '.';
    
    for (int i = 0; i < decimals; i++) {
        frac *= 10;
        int digit = (int)frac;
        *buf++ = '0' + digit;
        frac -= digit;
    }
    *buf = '\0';
}

static void get_word(char* line, char* dest, int max_len) {
    int i = 0;
    line = skip_spaces(line);
    while (*line && *line != ' ' && *line != '\t' && *line != '\n' && 
           *line != ',' && *line != ';' && *line != '(' && *line != ')' &&
           *line != '+' && *line != '-' && *line != '*' && *line != '/' &&
           *line != '=' && *line != '<' && *line != '>' && i < max_len - 1) {
        dest[i++] = my_toupper(*line++);
    }
    dest[i] = '\0';
}

static int get_line_index(int line_num) {
    for (int i = 0; i < program_line_count; i++) {
        if (line_numbers[i] == line_num) return i;
    }
    return -1;
}

static void error_msg(const char* msg) {
    os_api->print_shell("ERROR at line ");
    char buf[12];
    int_to_str(line_numbers[current_line], buf);
    os_api->print_shell(buf);
    os_api->print_shell(": ");
    os_api->print_shell(msg);
    os_api->print_shell("\n");
    running = 0;
}

/* ============================================================================
 * VARIABLE MANAGEMENT
 * ============================================================================ */
static int get_var_index(char name, var_type_t type) {
    if (name >= 'A' && name <= 'Z') {
        if (type == TYPE_INT) return name - 'A';
        if (type == TYPE_FLOAT) return name - 'A';
        if (type == TYPE_STRING) return name - 'A';
    }
    return -1;
}

static int32_t* get_int_var(char name) {
    int idx = get_var_index(name, TYPE_INT);
    if (idx >= 0) {
        var_initialized[idx] = 1;
        return &int_vars[idx];
    }
    return NULL;
}

static float* get_float_var(char name) {
    int idx = get_var_index(name, TYPE_FLOAT);
    if (idx >= 0) {
        var_initialized[idx + MAX_INT_VARS] = 1;
        return &float_vars[idx];
    }
    return NULL;
}

static char* get_string_var(char name) {
    int idx = get_var_index(name, TYPE_STRING);
    if (idx >= 0) {
        var_initialized[idx + MAX_INT_VARS + MAX_FLOAT_VARS] = 1;
        return string_vars[idx];
    }
    return NULL;
}

static int find_array(const char* name) {
    for (int i = 0; i < array_count; i++) {
        if (my_strcmp(arrays[i].name, name) == 0) return i;
    }
    return -1;
}

static int32_t get_array_val(int arr_idx, int idx1, int idx2) {
    if (arr_idx < 0 || arr_idx >= array_count) return 0;
    int offset = idx1;
    if (arrays[arr_idx].dimensions == 2) {
        offset = idx1 * arrays[arr_idx].dim2_size + idx2;
    }
    if (offset >= 0 && offset < arrays[arr_idx].size) {
        return arrays[arr_idx].data[offset];
    }
    return 0;
}

static void set_array_val(int arr_idx, int idx1, int idx2, int32_t val) {
    if (arr_idx < 0 || arr_idx >= array_count) return;
    int offset = idx1;
    if (arrays[arr_idx].dimensions == 2) {
        offset = idx1 * arrays[arr_idx].dim2_size + idx2;
    }
    if (offset >= 0 && offset < arrays[arr_idx].size) {
        arrays[arr_idx].data[offset] = val;
    }
}

/* ============================================================================
 * EXPRESSION PARSER
 * ============================================================================ */
static int32_t parse_expression(void);

static int32_t parse_factor(void) {
    expr_ptr = skip_spaces(expr_ptr);
    
    // Parenthesized expression
    if (*expr_ptr == '(') {
        expr_ptr++;
        int32_t val = parse_expression();
        expr_ptr = skip_spaces(expr_ptr);
        if (*expr_ptr == ')') expr_ptr++;
        return val;
    }
    
    // Unary minus
    if (*expr_ptr == '-') {
        expr_ptr++;
        return -parse_factor();
    }
    
    // Unary plus
    if (*expr_ptr == '+') {
        expr_ptr++;
        return parse_factor();
    }
    
    // NOT operator
    if (my_strncmp(expr_ptr, "NOT", 3) == 0 && 
        (*(expr_ptr + 3) == ' ' || *(expr_ptr + 3) == '(' || *(expr_ptr + 3) == '\0')) {
        expr_ptr += 3;
        return !parse_factor();
    }
    
    // Function call or variable
    if ((*expr_ptr >= 'A' && *expr_ptr <= 'Z') ||
        (*expr_ptr >= 'a' && *expr_ptr <= 'z')) {
        char name[32];
        int i = 0;
        char* start = expr_ptr;
        
        while ((*expr_ptr >= 'A' && *expr_ptr <= 'Z') ||
               (*expr_ptr >= 'a' && *expr_ptr <= 'z') ||
               (*expr_ptr >= '0' && *expr_ptr <= '9') || *expr_ptr == '_' ||
               *expr_ptr == '$' || *expr_ptr == '%' || *expr_ptr == '#') {
            if (i < 31) name[i++] = my_toupper(*expr_ptr);
            expr_ptr++;
        }
        name[i] = '\0';
        
        expr_ptr = skip_spaces(expr_ptr);
        
        // Check for array access
        if (*expr_ptr == '(') {
            int arr_idx = find_array(name);
            if (arr_idx >= 0) {
                expr_ptr++;
                int32_t idx1 = parse_expression();
                expr_ptr = skip_spaces(expr_ptr);
                int32_t idx2 = 0;
                if (*expr_ptr == ',') {
                    expr_ptr++;
                    idx2 = parse_expression();
                    expr_ptr = skip_spaces(expr_ptr);
                }
                if (*expr_ptr == ')') expr_ptr++;
                return get_array_val(arr_idx, idx1, idx2);
            }
        }
        
        // Check for function call
        if (*expr_ptr == '(') {
            expr_ptr++;
            // Parse function arguments if needed
            while (*expr_ptr && *expr_ptr != ')') expr_ptr++;
            if (*expr_ptr == ')') expr_ptr++;
            return 0; // Placeholder for function call result
        }
        
        // Built-in functions
        if (str_eq(name, "RND")) {
            expr_ptr = skip_spaces(expr_ptr);
            if (*expr_ptr == '(') {
                expr_ptr++;
                int32_t max = parse_expression();
                expr_ptr = skip_spaces(expr_ptr);
                if (*expr_ptr == ')') expr_ptr++;
                if (max <= 0) max = 32767;
                return (int32_t)(os_api->get_random(max));
            }
            return (int32_t)(os_api->get_random(32767));
        }
        
        if (str_eq(name, "ABS")) {
            expr_ptr = skip_spaces(expr_ptr);
            if (*expr_ptr == '(') {
                expr_ptr++;
                int32_t val = parse_expression();
                expr_ptr = skip_spaces(expr_ptr);
                if (*expr_ptr == ')') expr_ptr++;
                return (val < 0) ? -val : val;
            }
        }
        
        if (str_eq(name, "SQR")) {
            expr_ptr = skip_spaces(expr_ptr);
            if (*expr_ptr == '(') {
                expr_ptr++;
                int32_t val = parse_expression();
                expr_ptr = skip_spaces(expr_ptr);
                if (*expr_ptr == ')') expr_ptr++;
                if (val < 0) return 0;
                int32_t res = 0;
                int32_t bit = 1 << 15;
                while (bit) {
                    int32_t temp = res + bit;
                    if (temp * temp <= val) res = temp;
                    bit >>= 1;
                }
                return res;
            }
        }
        
        if (str_eq(name, "LEN")) {
            expr_ptr = skip_spaces(expr_ptr);
            if (*expr_ptr == '(') {
                expr_ptr++;
                char* arg_start = expr_ptr;
                while (*arg_start && *arg_start != ')') arg_start++;
                *arg_start = '\0';
                char* str_val = get_string_var(my_toupper(*expr_ptr));
                *arg_start = ')';
                expr_ptr = arg_start;
                if (*expr_ptr == ')') expr_ptr++;
                if (str_val) return (int32_t)my_strlen(str_val);
                return 0;
            }
        }
        
        if (str_eq(name, "ASC")) {
            expr_ptr = skip_spaces(expr_ptr);
            if (*expr_ptr == '(') {
                expr_ptr++;
                char* str_val = get_string_var(my_toupper(*expr_ptr));
                expr_ptr = skip_spaces(expr_ptr + 1);
                if (*expr_ptr == ')') expr_ptr++;
                if (str_val && str_val[0]) return (int32_t)(uint8_t)str_val[0];
                return 0;
            }
        }
        
        if (str_eq(name, "VAL")) {
            expr_ptr = skip_spaces(expr_ptr);
            if (*expr_ptr == '(') {
                expr_ptr++;
                char* str_val = get_string_var(my_toupper(*expr_ptr));
                expr_ptr = skip_spaces(expr_ptr + 1);
                if (*expr_ptr == ')') expr_ptr++;
                if (str_val) return str_to_int(str_val);
                return 0;
            }
        }
        
        if (str_eq(name, "INSTR")) {
            expr_ptr = skip_spaces(expr_ptr);
            if (*expr_ptr == '(') {
                expr_ptr++;
                char* haystack = get_string_var(my_toupper(*expr_ptr));
                expr_ptr = skip_spaces(expr_ptr + 1);
                if (*expr_ptr == ',') expr_ptr++;
                expr_ptr = skip_spaces(expr_ptr);
                char* needle = get_string_var(my_toupper(*expr_ptr));
                expr_ptr = skip_spaces(expr_ptr + 1);
                if (*expr_ptr == ')') expr_ptr++;
                if (haystack && needle) {
                    char* found = haystack;
                    while (*found) {
                        if (my_strncmp(found, needle, my_strlen(needle)) == 0) {
                            return (int32_t)(found - haystack + 1);
                        }
                        found++;
                    }
                }
                return 0;
            }
        }
        
        if (str_eq(name, "PEEK")) {
            expr_ptr = skip_spaces(expr_ptr);
            if (*expr_ptr == '(') {
                expr_ptr++;
                int32_t addr = parse_expression();
                expr_ptr = skip_spaces(expr_ptr);
                if (*expr_ptr == ')') expr_ptr++;
                return *(volatile uint8_t*)addr;
            }
        }
        
        // Check for integer variable
        if (name[1] == '\0' || (name[1] == '%' && name[2] == '\0')) {
            char var_name = name[0];
            int32_t* var = get_int_var(var_name);
            if (var) return *var;
        }
        
        return 0;
    }
    
    // Hex number
    if (*expr_ptr == '&' && (*(expr_ptr + 1) == 'H' || *(expr_ptr + 1) == 'h')) {
        expr_ptr += 2;
        int32_t result = 0;
        while ((*expr_ptr >= '0' && *expr_ptr <= '9') ||
               (*expr_ptr >= 'A' && *expr_ptr <= 'F') ||
               (*expr_ptr >= 'a' && *expr_ptr <= 'f')) {
            result *= 16;
            if (*expr_ptr >= '0' && *expr_ptr <= '9')
                result += *expr_ptr - '0';
            else if (*expr_ptr >= 'A' && *expr_ptr <= 'F')
                result += *expr_ptr - 'A' + 10;
            else
                result += *expr_ptr - 'a' + 10;
            expr_ptr++;
        }
        return result;
    }
    
    // Binary number
    if (*expr_ptr == '&' && (*(expr_ptr + 1) == 'B' || *(expr_ptr + 1) == 'b')) {
        expr_ptr += 2;
        int32_t result = 0;
        while (*expr_ptr == '0' || *expr_ptr == '1') {
            result = result * 2 + (*expr_ptr - '0');
            expr_ptr++;
        }
        return result;
    }
    
    // Decimal number
    if (*expr_ptr >= '0' && *expr_ptr <= '9') {
        int32_t result = 0;
        while (*expr_ptr >= '0' && *expr_ptr <= '9') {
            result = result * 10 + (*expr_ptr - '0');
            expr_ptr++;
        }
        return result;
    }
    
    return 0;
}

static int32_t parse_power(void) {
    int32_t val = parse_factor();
    expr_ptr = skip_spaces(expr_ptr);
    
    while (*expr_ptr == '^') {
        expr_ptr++;
        int32_t exp = parse_factor();
        int32_t result = 1;
        for (int32_t i = 0; i < exp; i++) result *= val;
        val = result;
        expr_ptr = skip_spaces(expr_ptr);
    }
    return val;
}

static int32_t parse_term(void) {
    int32_t val = parse_power();
    expr_ptr = skip_spaces(expr_ptr);
    
    while (*expr_ptr == '*' || *expr_ptr == '/' || *expr_ptr == '\\' || *expr_ptr == '%') {
        char op = *expr_ptr++;
        int32_t next_val = parse_power();
        
        if (op == '*') val *= next_val;
        else if (op == '/') val = (next_val != 0) ? val / next_val : 0;
        else if (op == '\\') val = (next_val != 0) ? val / next_val : 0;
        else if (op == '%') val = (next_val != 0) ? val % next_val : 0;
        
        expr_ptr = skip_spaces(expr_ptr);
    }
    return val;
}

static int32_t parse_add_sub(void) {
    int32_t val = parse_term();
    expr_ptr = skip_spaces(expr_ptr);
    
    while (*expr_ptr == '+' || *expr_ptr == '-') {
        char op = *expr_ptr++;
        int32_t next_val = parse_term();
        if (op == '+') val += next_val;
        else val -= next_val;
        expr_ptr = skip_spaces(expr_ptr);
    }
    return val;
}

static int32_t parse_comparison(void) {
    int32_t val = parse_add_sub();
    expr_ptr = skip_spaces(expr_ptr);
    
    while (1) {
        if (*expr_ptr == '=') {
            expr_ptr++;
            int32_t next_val = parse_add_sub();
            val = (val == next_val) ? 1 : 0;
        } else if (*expr_ptr == '<' && *(expr_ptr + 1) == '>') {
            expr_ptr += 2;
            int32_t next_val = parse_add_sub();
            val = (val != next_val) ? 1 : 0;
        } else if (*expr_ptr == '<' && *(expr_ptr + 1) == '=') {
            expr_ptr += 2;
            int32_t next_val = parse_add_sub();
            val = (val <= next_val) ? 1 : 0;
        } else if (*expr_ptr == '>' && *(expr_ptr + 1) == '=') {
            expr_ptr += 2;
            int32_t next_val = parse_add_sub();
            val = (val >= next_val) ? 1 : 0;
        } else if (*expr_ptr == '<') {
            expr_ptr++;
            int32_t next_val = parse_add_sub();
            val = (val < next_val) ? 1 : 0;
        } else if (*expr_ptr == '>') {
            expr_ptr++;
            int32_t next_val = parse_add_sub();
            val = (val > next_val) ? 1 : 0;
        } else {
            break;
        }
        expr_ptr = skip_spaces(expr_ptr);
    }
    return val;
}

static int32_t parse_and(void) {
    int32_t val = parse_comparison();
    expr_ptr = skip_spaces(expr_ptr);
    
    while (my_strncmp(expr_ptr, "AND", 3) == 0 &&
           (*(expr_ptr + 3) == ' ' || *(expr_ptr + 3) == '\0')) {
        expr_ptr += 3;
        int32_t next_val = parse_comparison();
        val = val && next_val;
        expr_ptr = skip_spaces(expr_ptr);
    }
    return val;
}

static int32_t parse_or(void) {
    int32_t val = parse_and();
    expr_ptr = skip_spaces(expr_ptr);
    
    while (my_strncmp(expr_ptr, "OR", 2) == 0 &&
           (*(expr_ptr + 2) == ' ' || *(expr_ptr + 2) == '\0')) {
        expr_ptr += 2;
        int32_t next_val = parse_and();
        val = val || next_val;
        expr_ptr = skip_spaces(expr_ptr);
    }
    return val;
}

static int32_t parse_expression(void) {
    return parse_or();
}

static int32_t eval_expression(char* expr) {
    expr_ptr = expr;
    return parse_expression();
}

static int eval_string_expr(char* expr, char* result) {
    expr = skip_spaces(expr);
    result[0] = '\0';
    
    // String literal
    if (*expr == '"') {
        expr++;
        int i = 0;
        while (*expr && *expr != '"' && i < MAX_STRING_LEN - 1) {
            result[i++] = *expr++;
        }
        result[i] = '\0';
        return 1;
    }
    
    // String variable
    if ((*expr >= 'A' && *expr <= 'Z') || (*expr >= 'a' && *expr <= 'z')) {
        char name = my_toupper(*expr);
        char* var_val = get_string_var(name);
        if (var_val) {
            my_strcpy(result, var_val);
            return 1;
        }
    }
    
    // STR$ function
    if (starts_with(expr, "STR$") || starts_with(expr, "str$")) {
        expr += 4;
        expr = skip_spaces(expr);
        if (*expr == '(') {
            expr++;
            int32_t val = eval_expression(expr);
            int_to_str(val, result);
            return 1;
        }
    }
    
    // CHR$ function
    if (starts_with(expr, "CHR$") || starts_with(expr, "chr$")) {
        expr += 4;
        expr = skip_spaces(expr);
        if (*expr == '(') {
            expr++;
            int32_t val = eval_expression(expr);
            result[0] = (char)val;
            result[1] = '\0';
            return 1;
        }
    }
    
    // LEFT$ function
    if (starts_with(expr, "LEFT$") || starts_with(expr, "left$")) {
        expr += 5;
        expr = skip_spaces(expr);
        if (*expr == '(') {
            expr++;
            char* src = get_string_var(my_toupper(*expr));
            expr = skip_spaces(expr + 1);
            if (*expr == ',') expr++;
            expr = skip_spaces(expr);
            int32_t len = eval_expression(expr);
            if (src) {
                int slen = my_strlen(src);
                if (len > slen) len = slen;
                for (int i = 0; i < len; i++) result[i] = src[i];
                result[len] = '\0';
            }
            return 1;
        }
    }
    
    // RIGHT$ function
    if (starts_with(expr, "RIGHT$") || starts_with(expr, "right$")) {
        expr += 6;
        expr = skip_spaces(expr);
        if (*expr == '(') {
            expr++;
            char* src = get_string_var(my_toupper(*expr));
            expr = skip_spaces(expr + 1);
            if (*expr == ',') expr++;
            expr = skip_spaces(expr);
            int32_t len = eval_expression(expr);
            if (src) {
                int slen = my_strlen(src);
                int start = (slen > len) ? slen - len : 0;
                my_strcpy(result, src + start);
            }
            return 1;
        }
    }
    
    // MID$ function
    if (starts_with(expr, "MID$") || starts_with(expr, "mid$")) {
        expr += 4;
        expr = skip_spaces(expr);
        if (*expr == '(') {
            expr++;
            char* src = get_string_var(my_toupper(*expr));
            expr = skip_spaces(expr + 1);
            if (*expr == ',') expr++;
            expr = skip_spaces(expr);
            int32_t start = eval_expression(expr) - 1;
            expr_ptr = skip_spaces(expr_ptr);
            int32_t len = my_strlen(src) - start;
            if (*expr_ptr == ',') {
                expr_ptr++;
                len = eval_expression(expr_ptr);
            }
            if (src && start >= 0) {
                int i = 0;
                while (i < len && src[start + i]) {
                    result[i] = src[start + i];
                    i++;
                }
                result[i] = '\0';
            }
            return 1;
        }
    }
    
    return 0;
}

/* ============================================================================
 * PROGRAM PARSING
 * ============================================================================ */
static void clear_memory(void) {
    for (int i = 0; i < MAX_LINES; i++) {
        line_numbers[i] = 0;
        program_lines[i][0] = '\0';
    }
    program_line_count = 0;
    
    for (int i = 0; i < MAX_INT_VARS; i++) int_vars[i] = 0;
    for (int i = 0; i < MAX_FLOAT_VARS; i++) float_vars[i] = 0.0f;
    for (int i = 0; i < MAX_STRING_VARS; i++) string_vars[i][0] = '\0';
    for (int i = 0; i < MAX_INT_VARS + MAX_FLOAT_VARS + MAX_STRING_VARS; i++) {
        var_initialized[i] = 0;
    }
    
    array_count = 0;
    function_count = 0;
    for_stack_ptr = 0;
    while_stack_ptr = 0;
    gosub_stack_ptr = 0;
    if_stack_ptr = 0;
    data_item_count = 0;
    data_read_ptr = 0;
    skip_to_endif = 0;
}

static void parse_program(char* source) {
    int line_idx = 0;
    int char_idx = 0;
    int in_line_num = 1;
    line_numbers[0] = 0;
    
    while (*source && line_idx < MAX_LINES) {
        char c = *source++;
        
        if (c == '\r') continue;
        
        if (c == '\n') {
            program_lines[line_idx][char_idx] = '\0';
            line_idx++;
            char_idx = 0;
            in_line_num = 1;
            if (line_idx < MAX_LINES) line_numbers[line_idx] = 0;
        } else {
            if (in_line_num) {
                if (c >= '0' && c <= '9') {
                    line_numbers[line_idx] = line_numbers[line_idx] * 10 + (c - '0');
                } else if (c == ' ' || c == '\t') {
                    in_line_num = 0;
                } else {
                    in_line_num = 0;
                    if (char_idx < MAX_LINE_LENGTH - 1) {
                        program_lines[line_idx][char_idx++] = c;
                    }
                }
            } else {
                if (char_idx < MAX_LINE_LENGTH - 1) {
                    program_lines[line_idx][char_idx++] = c;
                }
            }
        }
    }
    
    if (char_idx > 0 && line_idx < MAX_LINES) {
        program_lines[line_idx][char_idx] = '\0';
        line_idx++;
    }
    
    program_line_count = line_idx;
    
    // Parse DATA statements
    data_item_count = 0;
    for (int i = 0; i < program_line_count; i++) {
        char* line = skip_spaces(program_lines[i]);
        if (starts_with(line, "DATA") || starts_with(line, "data")) {
            line += 4;
            line = skip_spaces(line);
            
            char item[256];
            int item_idx = 0;
            while (*line && data_item_count < MAX_DATA_ITEMS) {
                if (*line == ',') {
                    item[item_idx] = '\0';
                    char* data_item = (char*)FILE_BUFFER_ADDR + 0x10000 + data_item_count * 64;
                    my_strcpy(data_item, item);
                    data_items[data_item_count++] = data_item;
                    item_idx = 0;
                    line++;
                    line = skip_spaces(line);
                } else if (*line == '"') {
                    line++;
                    while (*line && *line != '"' && item_idx < 255) {
                        item[item_idx++] = *line++;
                    }
                    if (*line == '"') line++;
                } else {
                    item[item_idx++] = *line++;
                }
            }
            if (item_idx > 0 && data_item_count < MAX_DATA_ITEMS) {
                item[item_idx] = '\0';
                char* data_item = (char*)FILE_BUFFER_ADDR + 0x10000 + data_item_count * 64;
                my_strcpy(data_item, item);
                data_items[data_item_count++] = data_item;
            }
        }
    }
}

/* ============================================================================
 * COMMAND HANDLERS
 * ============================================================================ */
static void cmd_print(char* args) {
    args = skip_spaces(args);
    
    while (*args) {
        args = skip_spaces(args);
        
        if (*args == '\0') break;
        
        // Check for TAB(
        if (starts_with(args, "TAB(") || starts_with(args, "tab(")) {
            args += 4;
            int32_t pos = eval_expression(args);
            args = expr_ptr;
            args = skip_spaces(args);
            if (*args == ')') args++;
            
            // Move to position
            int current_col = 0;
            while (current_col < pos) {
                os_api->print_shellc(' ');
                current_col++;
            }
        }
        // String literal
        else if (*args == '"') {
            args++;
            while (*args && *args != '"') {
                os_api->print_shellc(*args++);
            }
            if (*args == '"') args++;
        }
        // String expression
        else if ((*args >= 'A' && *args <= 'Z') || (*args >= 'a' && *args <= 'z')) {
            char result[MAX_STRING_LEN];
            if (eval_string_expr(args, result)) {
                os_api->print_shell(result);
                args = expr_ptr;
            } else {
                // Try numeric expression
                int32_t val = eval_expression(args);
                char buf[32];
                int_to_str(val, buf);
                os_api->print_shell(buf);
                args = expr_ptr;
            }
        }
        // Numeric expression
        else if ((*args >= '0' && *args <= '9') || *args == '-' || *args == '(' ||
                 *args == '&') {
            int32_t val = eval_expression(args);
            char buf[32];
            int_to_str(val, buf);
            os_api->print_shell(buf);
            args = expr_ptr;
        }
        
        args = skip_spaces(args);
        
        // Handle semicolon (no newline) and comma (tab)
        if (*args == ';') {
            args++;
            args = skip_spaces(args);
        } else if (*args == ',') {
            args++;
            // Print spaces to next tab stop
            for (int i = 0; i < 8; i++) os_api->print_shellc(' ');
        } else {
            break;
        }
    }
    
    os_api->print_shellc('\n');
}

static void  cmd_input(char* args) {
    args = skip_spaces(args);
    
    // Check for prompt string
    if (*args == '"') {
        args++;
        while (*args && *args != '"') {
            os_api->print_shellc(*args++);
        }
        if (*args == '"') args++;
        args = skip_spaces(args);
        if (*args == ';' || *args == ',') args++;
        args = skip_spaces(args);
    }
    
    // Get variable name
    char var_name = my_toupper(*args);
    if (var_name < 'A' || var_name > 'Z') {
        error_msg("Invalid variable name");
        return;
    }
    
    // Read input
    char input_buf[MAX_STRING_LEN];
    int idx = 0;
    
    while (idx < MAX_STRING_LEN - 1) {
        char c = os_api->keyboard_read();
        if (c == '\n' || c == '\r') {
            break;
        } else if (c == '\b' && idx > 0) {
            idx--;
            os_api->print_shellc('\b');
            os_api->print_shellc(' ');
            os_api->print_shellc('\b');
        } else if (c >= 32 && c < 127) {
            input_buf[idx++] = c;
            os_api->print_shellc(c);
        }
    }
    input_buf[idx] = '\0';
    os_api->print_shellc('\n');
    
    // Check if string variable (has $ suffix)
    if (*(args + 1) == '$' || *(args + 1) == '$') {
        char* var = get_string_var(var_name);
        if (var) my_strcpy(var, input_buf);
    } else {
        int32_t* var = get_int_var(var_name);
        if (var) *var = str_to_int(input_buf);
    }
}


static void cmd_let(char* args) {
    args = skip_spaces(args);
    
    char var_name = my_toupper(*args);
    if (var_name < 'A' || var_name > 'Z') {
        error_msg("Invalid variable name");
        return;
    }
    args++;
    
    // Check for array assignment
    if (*args == '(') {
        args++;
        int32_t idx1 = eval_expression(args);
        args = expr_ptr;
        args = skip_spaces(args);
        int32_t idx2 = 0;
        if (*args == ',') {
            args++;
            idx2 = eval_expression(args);
            args = expr_ptr;
        }
        args = skip_spaces(args);
        if (*args == ')') args++;
        
        args = skip_spaces(args);
        if (*args == '=') args++;
        args = skip_spaces(args);
        
        int32_t val = eval_expression(args);
        
        char arr_name[2] = {var_name, '\0'};
        int arr_idx = find_array(arr_name);
        if (arr_idx >= 0) {
            set_array_val(arr_idx, idx1, idx2, val);
        }
        return;
    }
    
    // Check for string variable
    if (*args == '$') {
        args++;
        args = skip_spaces(args);
        if (*args == '=') args++;
        args = skip_spaces(args);
        
        char* var = get_string_var(var_name);
        if (var) {
            char result[MAX_STRING_LEN];
            if (eval_string_expr(args, result)) {
                my_strcpy(var, result);
            }
        }
        return;
    }
    
    // Check for integer/float variable
    args = skip_spaces(args);
    if (*args == '=') args++;
    args = skip_spaces(args);
    
    int32_t* var = get_int_var(var_name);
    if (var) {
        *var = eval_expression(args);
    }
}

static void cmd_goto(char* args) {
    int32_t target = eval_expression(args);
    int idx = get_line_index(target);
    if (idx >= 0) {
        current_line = idx - 1;
    } else {
        error_msg("Line number not found");
    }
}

static void cmd_gosub(char* args) {
    int32_t target = eval_expression(args);
    int idx = get_line_index(target);
    if (idx >= 0) {
        if (gosub_stack_ptr < MAX_GOSUB_DEPTH) {
            gosub_stack[gosub_stack_ptr++].line_num = current_line;
            current_line = idx - 1;
        } else {
            error_msg("GOSUB stack overflow");
        }
    } else {
        error_msg("Line number not found");
    }
}

static void cmd_return(void) {
    if (gosub_stack_ptr > 0) {
        current_line = gosub_stack[--gosub_stack_ptr].line_num;
    } else {
        error_msg("RETURN without GOSUB");
    }
}

static void cmd_if(char* args) {
    int32_t result = eval_expression(args);
    
    if (if_stack_ptr < MAX_NESTING_LEVEL) {
        if_result_stack[if_stack_ptr++] = result;
    }
    
    if (!result) {
        skip_to_endif = 1;
    }
}

static void cmd_else(void) {
    if (if_stack_ptr > 0) {
        if_result_stack[if_stack_ptr - 1] = !if_result_stack[if_stack_ptr - 1];
        skip_to_endif = !if_result_stack[if_stack_ptr - 1];
    }
}

static void cmd_elseif(char* args) {
    if (if_stack_ptr > 0) {
        if (if_result_stack[if_stack_ptr - 1]) {
            skip_to_endif = 1;
        } else {
            int32_t result = eval_expression(args);
            if_result_stack[if_stack_ptr - 1] = result;
            skip_to_endif = !result;
        }
    }
}

static void cmd_endif(void) {
    if (if_stack_ptr > 0) {
        if_stack_ptr--;
    }
    skip_to_endif = 0;
}

static void cmd_for(char* args) {
    args = skip_spaces(args);
    char var_name = my_toupper(*args);
    if (var_name < 'A' || var_name > 'Z') {
        error_msg("Invalid FOR variable");
        return;
    }
    args++;
    args = skip_spaces(args);
    
    if (*args != '=') {
        error_msg("Expected = in FOR");
        return;
    }
    args++;
    
    int32_t start_val = eval_expression(args);
    args = expr_ptr;
    args = skip_spaces(args);
    
    if (!starts_with(args, "TO") && !starts_with(args, "to")) {
        error_msg("Expected TO in FOR");
        return;
    }
    args += 2;
    
    int32_t end_val = eval_expression(args);
    args = expr_ptr;
    
    int32_t step = 1;
    args = skip_spaces(args);
    if (starts_with(args, "STEP") || starts_with(args, "step")) {
        args += 4;
        step = eval_expression(args);
    }
    
    int32_t* var = get_int_var(var_name);
    if (var) {
        *var = start_val;
        
        if (for_stack_ptr < MAX_FOR_DEPTH) {
            for_stack[for_stack_ptr].line_num = current_line;
            for_stack[for_stack_ptr].var = var_name;
            for_stack[for_stack_ptr].end_val = end_val;
            for_stack[for_stack_ptr].step = step;
            for_stack_ptr++;
        } else {
            error_msg("FOR stack overflow");
        }
    }
}

static void cmd_next(char* args) {
    args = skip_spaces(args);
    char var_name = my_toupper(*args);
    
    if (for_stack_ptr > 0) {
        for_stack_entry_t* entry = &for_stack[for_stack_ptr - 1];
        
        if (var_name >= 'A' && var_name <= 'Z' && entry->var != var_name) {
            error_msg("NEXT variable mismatch");
            return;
        }
        
        int32_t* var = get_int_var(entry->var);
        if (var) {
            *var += entry->step;
            
            int done = 0;
            if (entry->step > 0 && *var > entry->end_val) done = 1;
            if (entry->step < 0 && *var < entry->end_val) done = 1;
            
            if (done) {
                for_stack_ptr--;
            } else {
                current_line = entry->line_num;
            }
        }
    } else {
        error_msg("NEXT without FOR");
    }
}

static void cmd_while(char* args) {
    int32_t result = eval_expression(args);
    
    if (while_stack_ptr < MAX_WHILE_DEPTH) {
        while_stack[while_stack_ptr].line_num = current_line;
        while_stack[while_stack_ptr].eval_result = result;
        while_stack_ptr++;
    }
    
    if (!result) {
        // Skip to WEND
        int depth = 1;
        while (depth > 0 && current_line < program_line_count - 1) {
            current_line++;
            char* line = skip_spaces(program_lines[current_line]);
            if (starts_with(line, "WHILE") || starts_with(line, "while")) {
                depth++;
            } else if (starts_with(line, "WEND") || starts_with(line, "wend")) {
                depth--;
            }
        }
        while_stack_ptr--;
    }
}

static void cmd_wend(void) {
    if (while_stack_ptr > 0) {
        current_line = while_stack[while_stack_ptr - 1].line_num - 1;
    } else {
        error_msg("WEND without WHILE");
    }
}

static void cmd_dim(char* args) {
    args = skip_spaces(args);
    
    char name[32];
    int i = 0;
    while (*args && *args != '(' && i < 31) {
        name[i++] = my_toupper(*args++);
    }
    name[i] = '\0';
    
    if (*args != '(') {
        error_msg("Expected ( in DIM");
        return;
    }
    args++;
    
    int32_t dim1 = eval_expression(args);
    args = expr_ptr;
    args = skip_spaces(args);
    
    int32_t dim2 = 0;
    int dimensions = 1;
    if (*args == ',') {
        args++;
        dim2 = eval_expression(args);
        args = expr_ptr;
        dimensions = 2;
    }
    
    args = skip_spaces(args);
    if (*args == ')') args++;
    
    if (array_count < MAX_ARRAYS) {
        my_strcpy(arrays[array_count].name, name);
        arrays[array_count].dimensions = dimensions;
        arrays[array_count].dim1_size = dim1 + 1;
        arrays[array_count].dim2_size = (dimensions == 2) ? dim2 + 1 : 1;
        arrays[array_count].size = arrays[array_count].dim1_size * arrays[array_count].dim2_size;
        if (arrays[array_count].size > MAX_ARRAY_SIZE) {
            arrays[array_count].size = MAX_ARRAY_SIZE;
        }
        for (int j = 0; j < arrays[array_count].size; j++) {
            arrays[array_count].data[j] = 0;
        }
        array_count++;
    } else {
        error_msg("Too many arrays");
    }
}

static void cmd_read(char* args) {
    args = skip_spaces(args);
    
    while (*args) {
        char var_name = my_toupper(*args);
        int is_string = 0;
        
        args++;
        if (*args == '$') {
            is_string = 1;
            args++;
        }
        
        if (data_read_ptr < data_item_count) {
            if (is_string) {
                char* var = get_string_var(var_name);
                if (var) {
                    my_strcpy(var, data_items[data_read_ptr]);
                }
            } else {
                int32_t* var = get_int_var(var_name);
                if (var) {
                    *var = str_to_int(data_items[data_read_ptr]);
                }
            }
            data_read_ptr++;
        } else {
            error_msg("Out of DATA");
        }
        
        args = skip_spaces(args);
        if (*args == ',') {
            args++;
            args = skip_spaces(args);
        } else {
            break;
        }
    }
}

static void cmd_restore(void) {
    data_read_ptr = 0;
}

static void cmd_def(char* args) {
    // DEF FNname(x) = expression
    // Parse function definition
    args = skip_spaces(args);
    
    if (!starts_with(args, "FN") && !starts_with(args, "fn")) {
        error_msg("Expected FN in DEF");
        return;
    }
    args += 2;
    
    char func_name[32];
    get_word(args, func_name, 32);
    
    // Store function definition (simplified - just mark line)
    if (function_count < MAX_FUNCTIONS) {
        my_strcpy(functions[function_count].name, func_name);
        functions[function_count].line_num = current_line;
        function_count++;
    }
}

static void cmd_screen(char* args) {
    int32_t mode = eval_expression(args);
    os_api->set_vga_mode((uint8_t)mode);
}

static void cmd_pset(char* args) {
    int32_t x = eval_expression(args);
    args = expr_ptr;
    args = skip_spaces(args);
    if (*args == ',') args++;
    int32_t y = eval_expression(args);
    args = expr_ptr;
    args = skip_spaces(args);
    if (*args == ',') args++;
    int32_t color = eval_expression(args);
    
    os_api->draw_pixel(x, y, (uint8_t)color);
}

static void cmd_line(char* args) {
    int32_t x1 = eval_expression(args);
    args = expr_ptr;
    args = skip_spaces(args);
    if (*args == ',') args++;
    int32_t y1 = eval_expression(args);
    args = expr_ptr;
    args = skip_spaces(args);
    if (*args == ',') args++;
    int32_t x2 = eval_expression(args);
    args = expr_ptr;
    args = skip_spaces(args);
    if (*args == ',') args++;
    int32_t y2 = eval_expression(args);
    args = expr_ptr;
    args = skip_spaces(args);
    if (*args == ',') args++;
    int32_t color = eval_expression(args);
    
    // Bresenham's line algorithm
    int dx = (x2 > x1) ? (x2 - x1) : (x1 - x2);
    int dy = (y2 > y1) ? (y2 - y1) : (y1 - y2);
    int steps = (dx > dy) ? dx : dy;
    if (steps == 0) steps = 1;
    
    for (int i = 0; i <= steps; i++) {
        int px = x1 + (x2 - x1) * i / steps;
        int py = y1 + (y2 - y1) * i / steps;
        os_api->draw_pixel(px, py, (uint8_t)color);
    }
}

static void cmd_circle(char* args) {
    int32_t xc = eval_expression(args);
    args = expr_ptr;
    args = skip_spaces(args);
    if (*args == ',') args++;
    int32_t yc = eval_expression(args);
    args = expr_ptr;
    args = skip_spaces(args);
    if (*args == ',') args++;
    int32_t r = eval_expression(args);
    args = expr_ptr;
    args = skip_spaces(args);
    if (*args == ',') args++;
    int32_t color = eval_expression(args);
    
    int x = 0, y = r;
    int d = 3 - 2 * r;
    
    while (y >= x) {
        os_api->draw_pixel(xc + x, yc + y, color);
        os_api->draw_pixel(xc - x, yc + y, color);
        os_api->draw_pixel(xc + x, yc - y, color);
        os_api->draw_pixel(xc - x, yc - y, color);
        os_api->draw_pixel(xc + y, yc + x, color);
        os_api->draw_pixel(xc - y, yc + x, color);
        os_api->draw_pixel(xc + y, yc - x, color);
        os_api->draw_pixel(xc - y, yc - x, color);
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

static void cmd_cls(void) {
    os_api->screen_clear_shell();
}

static void cmd_color(char* args) {
    int32_t color = eval_expression(args);
    os_api->set_color((uint8_t)color);
}

static void cmd_locate(char* args) {
    int32_t row = eval_expression(args);
    args = expr_ptr;
    args = skip_spaces(args);
    if (*args == ',') args++;
    int32_t col = eval_expression(args);
    
    os_api->move_cursor(row, col);
}

static void cmd_beep(char* args) {
    int32_t freq = 1000;
    int32_t dur = 100;
    
    if (*args) {
        freq = eval_expression(args);
        args = expr_ptr;
        args = skip_spaces(args);
        if (*args == ',') {
            args++;
            dur = eval_expression(args);
        }
    }
    
    os_api->beep(freq, dur);
}

static void cmd_delay(char* args) {
    int32_t ms = eval_expression(args);
    os_api->busy_delay(ms);
}

static void cmd_poke(char* args) {
    int32_t addr = eval_expression(args);
    args = expr_ptr;
    args = skip_spaces(args);
    if (*args == ',') args++;
    int32_t val = eval_expression(args);
    
    *(volatile uint8_t*)addr = (uint8_t)val;
}

static void cmd_load(char* args) {
    args = skip_spaces(args);
    
    char filename[32];
    if (*args == '"') {
        args++;
        int i = 0;
        while (*args && *args != '"' && i < 31) {
            filename[i++] = *args++;
        }
        filename[i] = '\0';
    } else {
        get_word(args, filename, 32);
    }
    
    uint8_t* buffer = (uint8_t*)FILE_BUFFER_ADDR;
    if (os_api->fat16_file_load(filename, buffer) == 0) {
        clear_memory();
        parse_program((char*)buffer);
    } else {
        os_api->print_shell("File not found: ");
        os_api->print_shell(filename);
        os_api->print_shell("\n");
    }
}

static void cmd_save(char* args) {
    args = skip_spaces(args);
    
    char filename[32];
    if (*args == '"') {
        args++;
        int i = 0;
        while (*args && *args != '"' && i < 31) {
            filename[i++] = *args++;
        }
        filename[i] = '\0';
    } else {
        get_word(args, filename, 32);
    }
    
    // Build program text
    char buffer[16384];
    int offset = 0;
    
    for (int i = 0; i < program_line_count; i++) {
        char line_num_str[12];
        int_to_str(line_numbers[i], line_num_str);
        int len = my_strlen(line_num_str);
        for (int j = 0; j < len && offset < 16383; j++) {
            buffer[offset++] = line_num_str[j];
        }
        buffer[offset++] = ' ';
        
        len = my_strlen(program_lines[i]);
        for (int j = 0; j < len && offset < 16383; j++) {
            buffer[offset++] = program_lines[i][j];
        }
        buffer[offset++] = '\n';
    }
    
    os_api->fat16_file_save(filename, (uint8_t*)buffer, offset);
}

static void cmd_list(void) {
    for (int i = 0; i < program_line_count; i++) {
        char buf[12];
        int_to_str(line_numbers[i], buf);
        os_api->print_shell(buf);
        os_api->print_shellc(' ');
        os_api->print_shell(program_lines[i]);
        os_api->print_shellc('\n');
    }
}

static void cmd_files(void) {
    os_api->list_files();
}

static void cmd_kill(char* args) {
    args = skip_spaces(args);
    
    char filename[32];
    if (*args == '"') {
        args++;
        int i = 0;
        while (*args && *args != '"' && i < 31) {
            filename[i++] = *args++;
        }
        filename[i] = '\0';
    } else {
        get_word(args, filename, 32);
    }
    
    os_api->delete_file(filename);
}

static void cmd_new(void) {
    clear_memory();
}

static void cmd_run(void) {
    current_line = 0;
    for_stack_ptr = 0;
    while_stack_ptr = 0;
    gosub_stack_ptr = 0;
    if_stack_ptr = 0;
    data_read_ptr = 0;
    running = 1;
}

static void cmd_end(void) {
    running = 0;

    
}

static void cmd_rem(char* args) {
    // Do nothing - it's a comment
    (void)args;
}

/* ============================================================================
 * COMMAND EXECUTION
 * ============================================================================ */
static void execute_line(char* line) {
    line = skip_spaces(line);
    if (*line == '\0') return;
    
    char cmd[32];
    get_word(line, cmd, 32);
    char* args = skip_spaces(line + my_strlen(cmd));
    
    // Handle skip_to_endif
    if (skip_to_endif) {
        if (str_eq(cmd, "IF") || str_eq(cmd, "IF")) {
            if_stack_ptr++;
        } else if (str_eq(cmd, "ENDIF") || str_eq(cmd, "ENDIF")) {
            if (if_stack_ptr > 0) if_stack_ptr--;
            if (if_stack_ptr == 0) skip_to_endif = 0;
        } else if ((str_eq(cmd, "ELSE") || str_eq(cmd, "ELSE")) && if_stack_ptr == 1) {
            skip_to_endif = 0;
            cmd_else();
        } else if ((str_eq(cmd, "ELSEIF") || str_eq(cmd, "ELSEIF")) && if_stack_ptr == 1) {
            skip_to_endif = 0;
            cmd_elseif(args);
        }
        return;
    }
    
    // Direct assignment without LET
    if ((cmd[0] >= 'A' && cmd[0] <= 'Z') && (cmd[1] == '\0' || cmd[1] == '$' || cmd[1] == '%' || cmd[1] == '(')) {
        cmd_let(line);
        return;
    }
    
    // Command dispatch
    if (str_eq(cmd, "PRINT") || str_eq(cmd, "?")) {
        cmd_print(args);
    }
    else if (str_eq(cmd, "INPUT")) {
        cmd_input(args);
    }
    else if (str_eq(cmd, "LET")) {
        cmd_let(args);
    }
    else if (str_eq(cmd, "GOTO")) {
        cmd_goto(args);
    }
    else if (str_eq(cmd, "GOSUB")) {
        cmd_gosub(args);
    }
    else if (str_eq(cmd, "RETURN")) {
        cmd_return();
    }
    else if (str_eq(cmd, "IF")) {
        cmd_if(args);
    }
    else if (str_eq(cmd, "ELSE")) {
        cmd_else();
    }
    else if (str_eq(cmd, "ELSEIF")) {
        cmd_elseif(args);
    }
    else if (str_eq(cmd, "ENDIF")) {
        cmd_endif();
    }
    else if (str_eq(cmd, "FOR")) {
        cmd_for(args);
    }
    else if (str_eq(cmd, "NEXT")) {
        cmd_next(args);
    }
    else if (str_eq(cmd, "WHILE")) {
        cmd_while(args);
    }
    else if (str_eq(cmd, "WEND")) {
        cmd_wend();
    }
    else if (str_eq(cmd, "DIM")) {
        cmd_dim(args);
    }
    else if (str_eq(cmd, "READ")) {
        cmd_read(args);
    }
    else if (str_eq(cmd, "DATA")) {
        // DATA is processed during parsing
    }
    else if (str_eq(cmd, "RESTORE")) {
        cmd_restore();
    }
    else if (str_eq(cmd, "DEF")) {
        cmd_def(args);
    }
    else if (str_eq(cmd, "SCREEN")) {
        cmd_screen(args);
    }
    else if (str_eq(cmd, "PSET")) {
        cmd_pset(args);
    }
    else if (str_eq(cmd, "LINE")) {
        cmd_line(args);
    }
    else if (str_eq(cmd, "CIRCLE")) {
        cmd_circle(args);
    }
    else if (str_eq(cmd, "CLS")) {
        cmd_cls();
    }
    else if (str_eq(cmd, "COLOR")) {
        cmd_color(args);
    }
    else if (str_eq(cmd, "LOCATE")) {
        cmd_locate(args);
    }
    else if (str_eq(cmd, "BEEP")) {
        cmd_beep(args);
    }
    else if (str_eq(cmd, "SLEEP") || str_eq(cmd, "DELAY")) {
        cmd_delay(args);
    }
    else if (str_eq(cmd, "POKE")) {
        cmd_poke(args);
    }
    else if (str_eq(cmd, "LOAD")) {
        cmd_load(args);
    }
    else if (str_eq(cmd, "SAVE")) {
        cmd_save(args);
    }
    else if (str_eq(cmd, "LIST")) {
        cmd_list();
    }
    else if (str_eq(cmd, "FILES")) {
        cmd_files();
    }
    else if (str_eq(cmd, "KILL") || str_eq(cmd, "DELETE")) {
        cmd_kill(args);
    }
    else if (str_eq(cmd, "NEW")) {
        cmd_new();
    }
    else if (str_eq(cmd, "RUN")) {
        cmd_run();
    }
    else if (str_eq(cmd, "END") || str_eq(cmd, "STOP")) {
        cmd_end();
    }
    else if (str_eq(cmd, "REM") || str_eq(cmd, "'") || str_eq(cmd, "//")) {
        cmd_rem(args);
    }
    else {
        // Unknown command
        os_api->print_shell("Unknown command: ");
        os_api->print_shell(cmd);
        os_api->print_shell("\n");
    }
}

static void execute_program(void) {
    running = 1;
    current_line = 0;
    
    while (running && current_line < program_line_count) {
        execute_line(program_lines[current_line]);
        current_line++;
    }
    
    os_api->print_shell("[Program ended]\n");
}

/* ============================================================================
 * MAIN ENTRY POINT
 * ============================================================================ */
void main(void) {
    char filename[32];
    uint8_t* buffer;
    
    init_api();
    clear_memory();
    
    
    // Check stack for filename (passed from shell)
    // Stack should have: flag '@' followed by filename address
    int use_stack = 0;
    uint32_t filename_addr = 0;

    if (os_api->stack_peek() == '@') {
        os_api->stack_pop();  // Remove flag '@'
        os_api->pop_string_from_stack(filename);
         os_api->reverse_string(filename);
         use_stack=1;
    }
    

    if (!use_stack) {
        os_api->print_shell("Enter .BAS filename to run: ");
        
        // Read filename from keyboard
        int idx = 0;
        while (idx < 31) {
            char c = os_api->keyboard_read();
            if (c == '\n' || c == '\r') {
                break;
            } else if (c == '\b' && idx > 0) {
                idx--;
                os_api->print_shellc('\b');
                os_api->print_shellc(' ');
                os_api->print_shellc('\b');
            } else if (c >= 32 && c < 127) {
                filename[idx++] = c;
                os_api->print_shellc(c);
            }
        }
        filename[idx] = '\0';

    }
    
    // Load and execute the file
    buffer = (uint8_t*)FILE_BUFFER_ADDR;
    if (os_api->fat16_file_load(filename, buffer) == 0) {
        os_api->print_shell("\n");
        parse_program((char*)buffer);
        execute_program();
    } else {
        os_api->print_shell("Error: Could not load file '");
        os_api->print_shell(filename);
        os_api->print_shell("'\n");
        os_api->print_shell("\nPress any key to exit...\n");
        os_api->keyboard_read();
    }
}
