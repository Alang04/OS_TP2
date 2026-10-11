#include <stdint.h>
#include "../include/game.h"
#include "../include/config.h"
#include "../include/types.h"

struct TronGame {
    // Grid (static size from config)
    uint8_t cells[TRON_GRID_H][TRON_GRID_W];

    // Timing
    uint64_t last_step_ticks;  // last step tick reference (0 means not started)
    int ticks_per_step;        // logical speed (ticks per step)
    int step_count;            // steps in current round

    // Mode/state
    GameMode mode;             // one- or two-player
    GameState state;           // PLAYING / PAUSED / GAME_OVER
    Winner winner;             // if round ended
    int ai_enabled;            // if P2 is AI-controlled

    // Scores
    int score_p1;
    int score_p2;

    // Players
    PlayerState p1;
    PlayerState p2;
};

static struct TronGame g_state;

// Helpers
static inline int in_bounds(int x, int y){
    return (x >= 0 && x < TRON_GRID_W && y >= 0 && y < TRON_GRID_H);
}

static inline int is_opposite(Direction a, Direction b){
    return ((a == DIR_UP && b == DIR_DOWN) ||
            (a == DIR_DOWN && b == DIR_UP) ||
            (a == DIR_LEFT && b == DIR_RIGHT) ||
            (a == DIR_RIGHT && b == DIR_LEFT));
}

static void clear_grid(struct TronGame *g){
    for(int y = 0; y < TRON_GRID_H; y++){
        for(int x = 0; x < TRON_GRID_W; x++){
            g->cells[y][x] = CELL_EMPTY;
        }
    }
}

static void place_initial_positions(struct TronGame *g){
    // P1 left-center, P2 right-center
    g->p1.x = TRON_GRID_W / 4;
    g->p1.y = TRON_GRID_H / 2;
    g->p1.dir = DIR_RIGHT;
    g->p1.pending_dir = g->p1.dir;
    g->p1.alive = 1;

    g->p2.x = (3 * TRON_GRID_W) / 4;
    g->p2.y = (TRON_GRID_H) / 2;
    g->p2.dir = DIR_LEFT;
    g->p2.pending_dir = g->p2.dir;
    g->p2.alive = 1;
}

TronGame * game_create(GameMode mode){
    struct TronGame *g = &g_state;
    g->mode = mode;
    g->ai_enabled = (mode == MODE_ONE_PLAYER) ? 1 : 0;
    g->ticks_per_step = TRON_DEFAULT_TICKS_PER_STEP;
    g->last_step_ticks = 0;
    g->step_count = 0;
    g->winner = WIN_NONE;
    g->state = STATE_PLAYING;
    g->score_p1 = 0;
    g->score_p2 = 0;
    clear_grid(g);
    place_initial_positions(g);
    return g;
}

void game_destroy(TronGame *g){
    (void)g; // static state; nothing to free
}

void game_reset_round(TronGame *g){
    if(!g) return;
    clear_grid(g);
    place_initial_positions(g);
    g->last_step_ticks = 0;
    g->step_count = 0;
    g->winner = WIN_NONE;
    g->state = STATE_PLAYING;
}

void game_enable_ai(TronGame *g, int enabled){
    if(!g) return;
    g->ai_enabled = enabled ? 1 : 0;
}

void game_set_speed(TronGame *g, int ticks_per_step){
    if(!g) return;
    if(ticks_per_step < 1) ticks_per_step = 1;
    if(ticks_per_step > 100) ticks_per_step = 100; // límite razonable
    g->ticks_per_step = ticks_per_step;
}

void game_queue_turn(TronGame *g, int player_id, Direction dir){
    if(!g) return;
    if(player_id == 1){
        if(!is_opposite(g->p1.dir, dir)){
            g->p1.pending_dir = dir;
        }
    } else if(player_id == 2){
        if(!is_opposite(g->p2.dir, dir)){
            g->p2.pending_dir = dir;
        }
    }
}

static void dir_to_vec(Direction d, int *dx, int *dy){
    int vx = 0, vy = 0;
    if(d == DIR_UP) vy = -1; else if(d == DIR_DOWN) vy = 1;
    else if(d == DIR_LEFT) vx = -1; else if(d == DIR_RIGHT) vx = 1;
    *dx = vx; *dy = vy;
}

int game_update(TronGame *g, uint64_t now_ticks){
    if(!g) return 0;
    if(g->state != STATE_PLAYING) return 0;

    if(g->last_step_ticks == 0){
        g->last_step_ticks = now_ticks;
        return 0;
    }

    if((int)(now_ticks - g->last_step_ticks) < g->ticks_per_step){
        return 0; // not time yet
    }

    g->last_step_ticks = now_ticks;

    // Apply pending turns
    if(g->p1.alive) g->p1.dir = g->p1.pending_dir;
    if(g->p2.alive) g->p2.dir = g->p2.pending_dir;

    // Leave trail on current positions
    if(g->p1.alive && in_bounds(g->p1.x, g->p1.y)){
        g->cells[g->p1.y][g->p1.x] = CELL_TRAIL_P1;
    }
    if(g->p2.alive && in_bounds(g->p2.x, g->p2.y)){
        g->cells[g->p2.y][g->p2.x] = CELL_TRAIL_P2;
    }

    // Compute intended new positions
    int dx1=0, dy1=0, dx2=0, dy2=0;
    dir_to_vec(g->p1.dir, &dx1, &dy1);
    dir_to_vec(g->p2.dir, &dx2, &dy2);
    int nx1 = g->p1.x + dx1;
    int ny1 = g->p1.y + dy1;
    int nx2 = g->p2.x + dx2;
    int ny2 = g->p2.y + dy2;

    int p1_crash = 0, p2_crash = 0;

    // Head-on into same cell => both crash
    if(nx1 == nx2 && ny1 == ny2 && g->p1.alive && g->p2.alive){
        p1_crash = p2_crash = 1;
    } else {
        // Check individual collisions
        if(g->p1.alive){
            if(!in_bounds(nx1, ny1)) p1_crash = 1;
            else if(g->cells[ny1][nx1] != CELL_EMPTY) p1_crash = 1;
        }
        if(g->p2.alive){
            if(!in_bounds(nx2, ny2)) p2_crash = 1;
            else if(g->cells[ny2][nx2] != CELL_EMPTY) p2_crash = 1;
        }
    }

    // Apply movements if not crashed
    if(g->p1.alive && !p1_crash){ g->p1.x = nx1; g->p1.y = ny1; }
    if(g->p2.alive && !p2_crash){ g->p2.x = nx2; g->p2.y = ny2; }

    // Update alive flags
    if(p1_crash) g->p1.alive = 0;
    if(p2_crash) g->p2.alive = 0;

    // Mark new positions as trail immediately? In classic Tron the head occupies a cell but leaves trail behind; we keep head implicit and cells contain trails only.
    if(g->p1.alive){ g->cells[g->p1.y][g->p1.x] = CELL_TRAIL_P1; }
    if(g->p2.alive){ g->cells[g->p2.y][g->p2.x] = CELL_TRAIL_P2; }

    g->step_count++;

    // Check end of round
    if(!g->p1.alive && !g->p2.alive){
        g->winner = WIN_TIE;
        g->state = STATE_GAME_OVER;
    } else if(!g->p1.alive){
        g->winner = WIN_P2;
        g->state = STATE_GAME_OVER;
        g->score_p2++;
    } else if(!g->p2.alive){
        g->winner = WIN_P1;
        g->state = STATE_GAME_OVER;
        g->score_p1++;
    }

    return 1;
}

int game_is_over(const TronGame *g){
    if(!g) return 1;
    return g->state == STATE_GAME_OVER;
}

Winner game_winner(const TronGame *g){
    if(!g) return WIN_NONE;
    return g->winner;
}

void game_get_snapshot(const TronGame *g, GameSnapshot *out){
    if(!g || !out) return;
    out->mode = g->mode;
    out->state = g->state;
    out->ticks_per_step = g->ticks_per_step;
    out->step_count = g->step_count;
    out->score_p1 = g->score_p1;
    out->score_p2 = g->score_p2;
    out->p1.x = g->p1.x; out->p1.y = g->p1.y; out->p1.dir = g->p1.dir; out->p1.alive = g->p1.alive; out->p1.color = TRON_COLOR_P1;
    out->p2.x = g->p2.x; out->p2.y = g->p2.y; out->p2.dir = g->p2.dir; out->p2.alive = g->p2.alive; out->p2.color = TRON_COLOR_P2;
    out->grid.width = TRON_GRID_W;
    out->grid.height = TRON_GRID_H;
    out->grid.cells = &g->cells[0][0];
    out->winner = g->winner;
}