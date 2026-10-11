#include <stdint.h>
#include "../../c/include/userlib.h" 
#include "../include/input.h"
#include "../include/config.h"

void input_init(void){
    char c;
    while(sys_read(&c, 1) > 0){
        // vaciar buffer de teclado previo
    }

}

void input_poll(InputEvents *out){
    //Inicializo los eventos de salida en 0
    out -> has_p1_turn = 0;
    out -> has_p2_turn = 0;
    out -> pause_toggled = 0;
    out -> quit_requested = 0;
    
    int count = 0;
    int byte_readed = 1;
    while(count < MAX_KEYS_PER_TICKS && byte_readed && out -> quit_requested == 0){ 
        char c;
        byte_readed = sys_read(&c, 1);

        //Si sigue habiendo teclas en el buffer de teclado, sigo leyendo, sino vuelvo al loop principal
        if (byte_readed) {
            //Si la tecla es mayuscula, lo paso a minuscula
            if(c >= 'A' && c <= 'Z'){
                c += ('a' - 'A');
            }
            
            //Mapeo la tecla leida a un evento, a menos que se haya activado la pausa o el quit en el mismo ciclo
            Direction dir;
            if (c == KEY_PAUSE) {
                out->pause_toggled = 1;
            } else if (c == KEY_QUIT) {
                out->quit_requested = 1;
            } else if (!out->pause_toggled && !out->quit_requested && input_map_key_to_dir_for_p1(c, &dir)) {
                out->has_p1_turn = 1;
                out->p1_turn = dir;
            } else if (!out->pause_toggled && !out->quit_requested && input_map_key_to_dir_for_p2(c, &dir)) {
                out->has_p2_turn = 1;
                out->p2_turn = dir;
            } 
    
            count++;
        }
    }
}

int input_map_key_to_dir_for_p1(char c, Direction *out){
    int toReturn = 0;
    if(c == KEY_P1_UP){
        *out = DIR_UP;
        toReturn++;
    } else if(c == KEY_P1_DOWN){
        *out = DIR_DOWN;
        toReturn++;
    } else if(c == KEY_P1_LEFT){
        *out = DIR_LEFT;
        toReturn++;
    } else if(c == KEY_P1_RIGHT){
        *out = DIR_RIGHT;
        toReturn++;
    }
    return toReturn;

}

int input_map_key_to_dir_for_p2(char c, Direction *out){
    int toReturn = 0;
    if(c == KEY_P2_UP){
        *out = DIR_UP;
        toReturn++;
    } else if(c == KEY_P2_DOWN){
        *out = DIR_DOWN;
        toReturn++;
    } else if(c == KEY_P2_LEFT){
        *out = DIR_LEFT;
        toReturn++;
    } else if(c == KEY_P2_RIGHT){
        *out = DIR_RIGHT;
        toReturn++;
    }
    return toReturn;
}