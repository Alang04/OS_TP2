#include "../include/ai.h"
#include "../include/types.h"

static void dir_to_vec(Direction d, int *dx, int *dy){
    int vx = 0, vy = 0;
    if(d == DIR_UP) vy = -1; else if(d == DIR_DOWN) vy = 1;
    else if(d == DIR_LEFT) vx = -1; else if(d == DIR_RIGHT) vx = 1;
    *dx = vx; *dy = vy;
}

static Direction turn_left(Direction d){
    if(d == DIR_UP) return DIR_LEFT;
    if(d == DIR_DOWN) return DIR_RIGHT;
    if(d == DIR_LEFT) return DIR_DOWN;
    return DIR_UP; // RIGHT -> UP
}

static Direction turn_right(Direction d){
    if(d == DIR_UP) return DIR_RIGHT;
    if(d == DIR_DOWN) return DIR_LEFT;
    if(d == DIR_LEFT) return DIR_UP;
    return DIR_DOWN; // RIGHT -> DOWN
}

static int in_bounds(const GameSnapshot *snap, int x, int y){
    return (x >= 0 && x < snap->grid.width && y >= 0 && y < snap->grid.height);
}

static int is_safe(const GameSnapshot *snap, int x, int y){
    if(!in_bounds(snap, x, y)) return 0;
    const uint8_t *cells = snap->grid.cells;
    int w = snap->grid.width;
    return cells[y * w + x] == CELL_EMPTY;
}

int ai_decide_next_dir(const GameSnapshot *snap, int player_id, int difficulty, Direction *out_dir){
    (void)difficulty; // only one level for now
    if(!snap || !out_dir) return 0;
    const PlayerView *p = (player_id == 1) ? &snap->p1 : &snap->p2;
    if(!p->alive) return 0;

    int dx=0, dy=0;
    dir_to_vec(p->dir, &dx, &dy);
    int nx = p->x + dx;
    int ny = p->y + dy;

    if(is_safe(snap, nx, ny)){
        return 0; // keep direction, no turn proposed
    }

    Direction left = turn_left(p->dir);
    Direction right = turn_right(p->dir);

    int ldx=0, ldy=0, rdx=0, rdy=0;
    dir_to_vec(left, &ldx, &ldy);
    dir_to_vec(right, &rdx, &rdy);

    if(is_safe(snap, p->x + ldx, p->y + ldy)){
        *out_dir = left;
        return 1;
    }
    if(is_safe(snap, p->x + rdx, p->y + rdy)){
        *out_dir = right;
        return 1;
    }

    // last resort: try reverse if safe
    Direction rev;
    if(p->dir == DIR_UP) rev = DIR_DOWN;
    else if(p->dir == DIR_DOWN) rev = DIR_UP;
    else if(p->dir == DIR_LEFT) rev = DIR_RIGHT;
    else rev = DIR_LEFT;

    int rbx=0, rby=0;
    dir_to_vec(rev, &rbx, &rby);
    if(is_safe(snap, p->x + rbx, p->y + rby)){
        *out_dir = rev;
        return 1;
    }

    return 0; // no safe options
}
