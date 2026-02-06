#include  "stddef.h"
#include "stdint.h"


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


#define OS_API_ADDR 0x5F0F0

os_api_t* os_api;
#define VGA_COLOR_GREEN        0x02

char number_map[10]={'0','1','2','3','4','5','6','7','8','9'};
char* lose_message="You have guessed the wrong number\n";
char* go_high_message="Choose a higher number from 0-9\n";
char* go_low_message="Choose a low number from 0-9\n";
int guesses=5;
char guess;


void init_api(void){
    os_api=(os_api_t*) OS_API_ADDR;
}


void start_game_init(){
    os_api->save_vga(0);
    os_api->screen_clear_shell();
    os_api->print_shell("WELCOME TO IYKE GUESSING GAME\nTO QUIT PRESS q(Q) \n\n");
    os_api->print_shell("YOU CAN GUESS BETWEEN 1-9 \n");
    guess=number_map[os_api->get_random(10)];
}


int char2int(char guess){
    if(guess>='0' && guess<='9'){
        return guess-48;
    }
    return -1;
}

void exit_game(){
     os_api->load_vga(0);
}

char input(){
    return os_api->keyboard_read();
}


void print_remaining_guesses(){
    char number_map[]={'0','1','2','3','4','5','6','7','8','9'};
    os_api->print_shell("You have ");
     os_api->print_shellc(number_map[guesses]);
     os_api->print_shell(" guesses remaining !!! \n");
}

void print_guess(char guess){
        os_api->print_shell("You guessed ");
        os_api->print_shellc(guess);
        os_api->print_shellc('\n');
}

int print_direction(int guess,int answer){
    if(guess<=-1){
        os_api->print_shell("Invalid character \n");
        return -1;
    }

    if(guess>answer){
        os_api->print_shell("Try a Lower number \n");
        return 0;
    }
    else if(guess<answer){
        os_api->print_shell("Try Higher number \n");
        return 0;
    }else{
        os_api->print_shell("You guessed the number correctly \n");
        return 1;
    }
    
}

char get_answer(){
    return guess;
}

print_banner_border(){
    int count=4;
    while(count>0){
        os_api->print_shell("====================");
        count--;
    }
    os_api->print_shell("\n");
}

print_congrats(char guess){
    print_banner_border();
    os_api->print_shell("          CONGRATULATIONS YOU WON THE GAME, THE NUMBER ");
    os_api->print_shellc(guess);
    os_api->print_shell(" IS CORRECT\n");
   print_banner_border();
    os_api->print_shell("Click on (y/Y) to continue or on any KEY to exit the game");
}



void main(void){
    init_api();
    start_game_init();
    while(1==1){
        print_remaining_guesses();
        char guess=input();
        if(guess=='Q' || guess=='q'){
            break;
        }
        print_guess(guess);
        int result=print_direction(char2int(guess),char2int(get_answer()));
        os_api->print_shell("\n");
        guesses--;
        if(result==1 || guesses<=0){
            print_congrats(guess);
            char con=input();
            if(con=='Y' || con=='y'){
                os_api->screen_clear_shell();
                guesses=5;
                continue;
            }else{
                break;
            }
        }
    }
    exit_game();
}


