#include <stdint.h>
#include <stddef.h>
#include "../include/menu.h"
#include "../include/config.h"
#include "../include/tron_sound.h"
#include "../../c/include/userlib.h"

static void put_ch(char c){ sys_write(STDOUT, &c, 1); }
static void put_str(const char *s){ if(!s) return; for(const char *p=s; *p; ++p) put_ch(*p); }

static void put_num(uint64_t v){
    char b[32];
    num_to_str(v, b, 10);
    for(char *p=b; *p; ++p) put_ch(*p);
}

static void clear_screen(void){ sys_clear(); }

static int read_key_nonblock(char *out){
    int r = sys_read(out, 1);
    if(r <= 0) return 0;
    char c = *out;
    if(c >= 'A' && c <= 'Z') c += ('a' - 'A');
    *out = c;
    return 1;
}

static void wait_small_delay(void){
    // Pequeño spin para no saturar el CPU; usamos ticks
    uint64_t start = sys_ticks();
    while((int)(sys_ticks() - start) < 1) { /* ~1 tick */ tron_music_update(); }
}

static int clamp(int v, int lo, int hi){
    if(v < lo) return lo;
    if(v > hi) return hi;
    return v;
}

static void draw_header(void){
    put_str("==================== TRON ====================\n");
}

static int show_main_menu(void){
    clear_screen();
    draw_header();
    put_str("Seleccione un modo:\n\n");
    put_str("  [1] 1 Jugador vs IA\n");
    put_str("  [2] 2 Jugadores locales\n");
    put_str("  [q] Salir\n\n");
    put_str("Use la tecla indicada para elegir.\n");

    for(;;){
        char c;
        if(read_key_nonblock(&c)){
            if(c == '1') return 1;
            if(c == '2') return 2;
            if(c == 'q') return 0; // salir
        }
        tron_music_update();
        wait_small_delay();
    }
}

static int submenu_one_player(MenuConfig *cfg){
    // Defaults
    cfg->mode = MODE_ONE_PLAYER;
    if(cfg->ticks_per_step <= 0) cfg->ticks_per_step = TRON_DEFAULT_TICKS_PER_STEP;

    for(;;){
        clear_screen();
        draw_header();
        put_str("Modo: 1 Jugador vs IA\n\n");
        put_str("Opciones:\n");
        put_str("  Velocidad (ticks/step, 1..10): "); put_num((uint64_t)cfg->ticks_per_step); put_str("\n\n");
        put_str("Controles:\n");
        put_str("  '+' / '-' ajustan velocidad\n");
        put_str("  [enter] Comenzar    [b] Volver\n");

        char c;
        if(read_key_nonblock(&c)){
            if(c == '+'){
                cfg->ticks_per_step = clamp(cfg->ticks_per_step - 1, 1, 10); // menos ticks => más rápido
            } else if(c == '-'){
                cfg->ticks_per_step = clamp(cfg->ticks_per_step + 1, 1, 10);
            } else if(c == '\n' || c == '\r'){
                return 1; // comenzar
            } else if(c == 'b'){
                return 0; // volver
            }
        }
        tron_music_update();
        wait_small_delay();
    }
}

static int submenu_two_players(MenuConfig *cfg){
    cfg->mode = MODE_TWO_PLAYERS;
    if(cfg->ticks_per_step <= 0) cfg->ticks_per_step = TRON_DEFAULT_TICKS_PER_STEP;

    for(;;){
        clear_screen();
        draw_header();
        put_str("Modo: 2 Jugadores locales\n\n");
        put_str("Opciones:\n");
        put_str("  Velocidad (ticks/step, 1..10): "); put_num((uint64_t)cfg->ticks_per_step); put_str("\n\n");
        put_str("Controles:\n");
        put_str("  '+' / '-' ajustan velocidad\n");
        put_str("  [enter] Comenzar    [b] Volver\n");

        char c;
        if(read_key_nonblock(&c)){
            if(c == '+'){
                cfg->ticks_per_step = clamp(cfg->ticks_per_step - 1, 1, 10);
            } else if(c == '-'){
                cfg->ticks_per_step = clamp(cfg->ticks_per_step + 1, 1, 10);
            } else if(c == '\n' || c == '\r'){
                return 1; // comenzar
            } else if(c == 'b'){
                return 0; // volver
            }
        }
        tron_music_update();
        wait_small_delay();
    }
}

int menu_show_and_get_config(MenuConfig *out){
    if(!out) return 0;
    // Valores por defecto antes de entrar a submenús
    out->ticks_per_step = TRON_DEFAULT_TICKS_PER_STEP;

    for(;;){
        // Asegura que haya música de menú corriendo
        tron_music_play_menu();
        int choice = show_main_menu();
        if(choice == 0){
            tron_music_stop();
            return 0; // salir
        } else if(choice == 1){
            tron_music_stop();
            if(submenu_one_player(out)) return 1;
        } else if(choice == 2){
            tron_music_stop();
            if(submenu_two_players(out)) return 1;
        }
        // si el usuario eligió volver en submenú, se repite el main menu
    }
}
