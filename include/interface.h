#ifndef INTERFACE_H
#define INTERFACE_H

#include "bitboard.h"
#include "player.h"

/**
 * @brief Draws the current board state.
 * @param state Pointer to the game state.
 * @param p1 The first player (draws as 'O').
 * @param p2 The second player (draws as 'X').
 */
void draw_board(const GameState* state, const Player* p1, const Player* p2);

/**
 * @brief Announces the result of the game.
 * @param winner A pointer to the winning player, or NULL for a draw.
 */
void announce_result(const Player* winner);

#endif // INTERFACE_H