#include "engine.h"
#include "bitboard.h"
#include <stdio.h> // Required for printf

// Global variable to count the number of nodes searched.
u_int64_t g_nodes_searched = 0;

int negamax(GameState* const state, int alpha, int beta) {
    g_nodes_searched++; // Increment node count on every entry

    if (is_draw(state)) {
        return 0;
    }

    // can the player win on the next move
    for (int col = 0; col < WIDTH; col++) {
        if (can_play(state, col) && is_winning_move(state, col)) {
            return (WIDTH*HEIGHT - state->moves + 1) / 2;
        }
    }

    // upper bound beta as we cannot win immediately
    int max = (WIDTH*HEIGHT - state->moves - 1) / 2;
    if (beta > max) {
        beta = max;
        if (alpha >= beta) return beta;
    }

    // iterate through child nodes
    for (int col = 0; col < WIDTH; col++) {
        if (!can_play(state, col)) {continue;}

        make_move(state, col);
        int score = -negamax(state, -beta, -alpha);
        undo_move(state, col);

        if (score >= beta) return score;
        if (score > alpha) alpha = score;
    }

    return alpha;
}
