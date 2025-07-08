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

    // Check if the opponent can win on the next move
    for (int col = 0; col < WIDTH; col++) {
        if (can_play(state, col) && is_winning_move(state, col)) {
            // Return the best possible score for a winning move
            return (WIDTH*HEIGHT + 1 - state->moves) / 2;
        }
    }

    // Upper bound beta as we cannot win immediately
    int max = (WIDTH*HEIGHT - 1 - state->moves) / 2;
    if (beta > max) {
        beta = max;
        if (alpha >= beta) return beta;
    }

    // Iterate through child nodes
    for (int col = 0; col < WIDTH; col++) {
        if (!can_play(state, col)) {continue;}

        // Make a move on a copy of the state to avoid undoing
        GameState next_state = *state;
        make_move(&next_state, col);

        int score = -negamax(&next_state, -beta, -alpha);
        
        // Pruning
        if (score >= beta) return score;
        if (score > alpha) alpha = score;
    }

    return alpha;
}
