typedef struct {
    void (*print_shell)(char*);
    void (*load_vga)(int);
    void (*save_vga)(int);
    void (*screen_clear_shell)(void);
    char (*keyboard_read)(void);
} os_api_t;

#define OS_API_ADDR 0x5F0F0

os_api_t* os_api_file;

void init_api_file(void){
    os_api_file=(os_api_t*) OS_API_ADDR;
}


void print(char* msg){
    os_api_file->print_shell(msg);
}