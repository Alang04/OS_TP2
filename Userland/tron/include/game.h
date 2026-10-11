// API de solo consumo (Tron): lógica de juego y snapshot para render/IA
// Contrato: funciones no bloqueantes; snapshot es copia de lectura del estado.
#ifndef TRON_GAME_H
#define TRON_GAME_H

#include <stdint.h>
#include "types.h"
#include "config.h"

typedef struct TronGame TronGame;

// Crea una instancia del juego con parámetros iniciales.
// - mode: 1P vs IA o 2 jugadores locales
// - grid_w/h: dimensiones en celdas
// Retorna puntero a estado o 0 si falla asignación/validación.
TronGame * game_create(GameMode mode);

// Libera recursos de la instancia de juego.
void game_destroy(TronGame *g);

// Reinicia la ronda (posiciones, trazas, ganador) manteniendo puntuaciones.
void game_reset_round(TronGame *g);

// Habilita/deshabilita IA
void game_enable_ai(TronGame *g, int enabled);

// Ajusta la velocidad lógica (ticks por step). Mínimo 1.
void game_set_speed(TronGame *g, int ticks_per_step);

// Encola un cambio de dirección a aplicar en el próximo step.
// player_id: 1 para P1, 2 para P2.
void game_queue_turn(TronGame *g, int player_id, Direction dir);

// Avanza el juego sólo si han transcurrido los ticks necesarios desde el último step.
// now_ticks: valor actual de sys_ticks().
// Retorna 1 si se avanzó un step; 0 si no correspondía aún.
int game_update(TronGame *g, uint64_t now_ticks);

// Consulta si la ronda actual terminó y el ganador.
int game_is_over(const TronGame *g);
Winner game_winner(const TronGame *g);

// Obtiene un snapshot (copia de lectura) del estado para render/IA.
void game_get_snapshot(const TronGame *g, GameSnapshot *out);

#endif

