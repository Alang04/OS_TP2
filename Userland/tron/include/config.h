#ifndef TRON_CONFIG_H
#define TRON_CONFIG_H

#include <stdint.h>

// Dimensiones de la grilla (en celdas)
#define TRON_GRID_W 64
#define TRON_GRID_H 36

// Velocidad lógica: ticks por step (ajustable en menú)
// Nota: en este SO, 18 ticks ≈ 1s (aprox). 3 → ~6 steps/seg
#define TRON_DEFAULT_TICKS_PER_STEP 3

#define TRON_COLOR_BG   0x000000
#define TRON_COLOR_P1   0x00FFFF
#define TRON_COLOR_P2   0xFF00FF
#define TRON_COLOR_WALL 0x202020

// Mapeo de teclas (modo texto)
#define KEY_P1_UP    'w'
#define KEY_P1_DOWN  's'
#define KEY_P1_LEFT  'a'
#define KEY_P1_RIGHT 'd'

#define KEY_P2_UP    'i'
#define KEY_P2_DOWN  'k'
#define KEY_P2_LEFT  'j'
#define KEY_P2_RIGHT 'l'

#define KEY_PAUSE    'p'
#define KEY_QUIT     'q'

#endif