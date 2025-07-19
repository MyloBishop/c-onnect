#ifndef ENGINE_H
#define ENGINE_H

#include "bitboard.h"

// Global counter for the number of nodes searched by the solver.
extern uint64_t g_nodes_searched;

/**
 * @brief Initializes the solver's internal state (e.g., column exploration order).
 * Must be called once at startup.
 */
void init_solver(void);

/**
 * @brief Resets the solver's node counter.
 */
void reset_solver(void);

/**
 * @brief Solves the given Connect4 position.
 * @param state A constant pointer to the game state to solve.
 * @param weak If true, performs a weak solve (only determines win/loss/draw, not score).
 * @return The score of the position. Positive for a win, negative for a loss, 0 for a draw.
 * The score represents the number of moves until victory.
 */
int solve(const GameState* state, bool weak);

#endif // ENGINE_H
