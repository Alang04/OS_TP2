#ifndef TRON_MENU_H
#define TRON_MENU_H

#include <stdint.h>
#include "types.h"

// Configuración elegida desde los menús
typedef struct {
	GameMode mode;       // MODE_ONE_PLAYER o MODE_TWO_PLAYERS
	int ticks_per_step;  // velocidad lógica (>=1)
} MenuConfig;

// Muestra el menú principal y los submenús por modo.
// Devuelve 1 si el usuario eligió una configuración para jugar
// y escribe esa configuración en out.
// Devuelve 0 si el usuario eligió salir.
int menu_show_and_get_config(MenuConfig *out);

#endif

