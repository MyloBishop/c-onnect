#ifndef ENGINE_H
#define ENGINE_H

#include "bitboard.h"

void fill_move_order(int* arr);
int negamax(GameState* const state, int alpha, int beta);

#endif
