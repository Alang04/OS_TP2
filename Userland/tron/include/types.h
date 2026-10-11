#ifndef TRON_TYPES_H
#define TRON_TYPES_H

#include <stdint.h>

// Celdas del tablero (representación compacta en userland)
typedef enum {
    CELL_EMPTY   = 0,
    CELL_TRAIL_P1 = 1,
    CELL_TRAIL_P2 = 2,
    CELL_WALL     = 3
} Cell;

// Direcciones cardinales
typedef enum {
    DIR_UP = 0,
    DIR_DOWN = 1,
    DIR_LEFT = 2,
    DIR_RIGHT = 3
} Direction;

// Modos de juego
typedef enum {
    MODE_ONE_PLAYER = 0,
    MODE_TWO_PLAYERS = 1
} GameMode;

// Estados de alto nivel del juego
typedef enum {
    STATE_MENU = 0,
    STATE_PLAYING = 1,
    STATE_PAUSED = 2,
    STATE_GAME_OVER = 3
} GameState;

// Resultado de una ronda
typedef enum {
    WIN_NONE = 0,   // en curso / sin ganador
    WIN_P1 = 1,
    WIN_P2 = 2,
    WIN_TIE = 3     // choque simultáneo cabeza a cabeza u otra condición de empate
} Winner;

// Datos de un jugador (para snapshots y HUD)
typedef struct {
    int x;
    int y;
    Direction dir;
    uint8_t alive;         // 0/1
    uint32_t color;        // color recomendado para render (modo gráfico)
} PlayerView;

//Datos del jugador, para el backend del juego
typedef struct {
    int x;
    int y;
    Direction dir;
    Direction pending_dir; // giro pendiente a aplicar en el próximo step
    uint8_t alive;         // 0/1
} PlayerState;

// Vista del tablero para render/IA (solo lectura)
typedef struct {
    int width;              // en celdas
    int height;             // en celdas
    const uint8_t *cells;   // width*height, valores de Cell
} GridView;

// Snapshot de estado para render/IA sin exponer estado interno completo
typedef struct {
    GameMode mode;
    GameState state;
    int ticks_per_step; // velocidad lógica
    int step_count;     // steps transcurridos en la ronda actual
    int score_p1;
    int score_p2;
    PlayerView p1;
    PlayerView p2;
    GridView grid;
    Winner winner;      // válido si state == STATE_GAME_OVER
} GameSnapshot;

#endif