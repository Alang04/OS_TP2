#ifndef TRON_RENDER_TEXT_H
#define TRON_RENDER_TEXT_H

#include <stdint.h>
#include "types.h"

// Inicializa el backend de render de texto/ASCII para una grilla dada
void render_text_init(int grid_w, int grid_h);

// Secuencia de dibujo (opcionalmente vacía si redibujás todo de una)
void render_text_begin(void);
void render_text_draw(const GameSnapshot *snap);
void render_text_end(void);

// Limpia pantalla / HUD
void render_text_clear(void);

#endif
