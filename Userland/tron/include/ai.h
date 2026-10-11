#ifndef TRON_AI_H
#define TRON_AI_H

#include "types.h"

// Decide próximo giro para el jugador indicado según dificultad.
// Retorna 1 si propone un giro válido y escribe en out_dir; 0 si no decide.
int ai_decide_next_dir(const GameSnapshot *snap, int player_id, int difficulty, Direction *out_dir);

#endif