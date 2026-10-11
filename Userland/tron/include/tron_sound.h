#ifndef TRON_SOUND_H
#define TRON_SOUND_H

#include "types.h"

void tron_play_intro_song(void);

// Jingle al terminar la ronda; cambia según el ganador
void tron_play_game_over_song(Winner w);

// Música de fondo no bloqueante
void tron_music_play_menu(void);   // inicia música en loop para el menú
void tron_music_play_game(void);   // inicia música en loop para el juego
void tron_music_stop(void);        // detiene música y apaga parlante
void tron_music_update(void);      // debe llamarse periódicamente (menu/game loop)

#endif
