#ifndef TRON_INPUT_H
#define TRON_INPUT_H

#include <stdint.h>
#include "types.h"

#define MAX_KEYS_PER_TICKS 16

// Eventos discretos de input por frame/step
typedef struct {
    int has_p1_turn;   // 0/1
    Direction p1_turn; // válido si has_p1_turn

    int has_p2_turn;   // 0/1
    Direction p2_turn; // válido si has_p2_turn

    int pause_toggled; // 0/1
    int quit_requested;// 0/1
} InputEvents;

// Inicializa estado interno del input si fuera necesario
void input_init(void);

// Lee teclado de forma no bloqueante y llena eventos para un paso
void input_poll(InputEvents *out);

// Mapea un caracter crudo a dirección (o devuelve -1 si no aplica)
int input_map_key_to_dir_for_p1(char c, Direction *out);
int input_map_key_to_dir_for_p2(char c, Direction *out);

#endif
