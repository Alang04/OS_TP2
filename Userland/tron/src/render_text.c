#include <stdint.h>
#include "../include/render_text.h"
#include "../../c/include/userlib.h"

static int g_w = 0, g_h = 0;

void render_text_init(int grid_w, int grid_h){
    g_w = grid_w; g_h = grid_h;
    sys_clear();
}

void render_text_begin(void){

}

static void put_str(const char *s){
    if(!s) return;
    for(const char *p = s; *p; ++p){
        sys_write(STDOUT, p, 1);
    }
}

static void put_num(uint64_t v){
    char b[32];
    num_to_str(v, b, 10);
    for(char *p = b; *p; ++p){
        sys_write(STDOUT, p, 1);
    }
}

void render_text_draw(const GameSnapshot *snap){
    if(!snap) return;

    // HUD
    put_str("TRON  [p: pausa] [q: menu] [WASD P1 / IJKL P2]\n");
    put_str("P1: "); put_num((uint64_t)snap->score_p1);
    put_str("    P2: "); put_num((uint64_t)snap->score_p2);
    put_str("    step: "); put_num((uint64_t)snap->step_count);
    put_str("\n");

    if(snap->state == STATE_PAUSED){
        put_str("== PAUSADO ==\n");
    } else if(snap->state == STATE_GAME_OVER){
        put_str("== FIN DE RONDA: ");
        if(snap->winner == WIN_P1) put_str("Gana P1");
        else if(snap->winner == WIN_P2) put_str("Gana P2");
        else put_str("Empate");
        put_str(" (presione 'p' para reiniciar)\n");
    }

    // Grid
    int w = snap->grid.width;
    int h = snap->grid.height;
    const uint8_t *cells = snap->grid.cells;

    for(int y = 0; y < h; ++y){
        for(int x = 0; x < w; ++x){
            char ch = '.';
            uint8_t c = cells[y * w + x];
            if(c == CELL_TRAIL_P1) ch = '1';
            else if(c == CELL_TRAIL_P2) ch = '2';
            else if(c == CELL_WALL) ch = '#';

            if(snap->p1.alive && x == snap->p1.x && y == snap->p1.y) ch = 'A';
            if(snap->p2.alive && x == snap->p2.x && y == snap->p2.y) ch = 'B';

            sys_write(STDOUT, &ch, 1);
        }
        char nl = '\n';
        sys_write(STDOUT, &nl, 1);
    }
}

void render_text_end(void){

}

void render_text_clear(void){
    sys_clear();
}