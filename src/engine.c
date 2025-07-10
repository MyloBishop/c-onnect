#include "engine.h"
#include "bitboard.h"
#include <stddef.h>
#include <assert.h>

uint64_t g_nodes_searched = 0;
int g_move_order[WIDTH];

void fill_move_order(int* arr) {
    assert(arr != NULL);

    int center = (WIDTH - 1) / 2;
    arr[0] = center;
    int count = 1;

    for (int i = 1; count < WIDTH; ++i) {
        int left = center - i;
        if (left >= 0) {
            arr[count++] = left;
        }

        if (count >= WIDTH) break;

        int right = center + i;
        if (right < WIDTH) {
            arr[count++] = right;
        }
    }
}

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
    for (int i = 0; i < WIDTH; i++) {
        int col = g_move_order[i];

        if (!can_play(state, col)) {continue;}

        GameState new_state = *state;
        make_move(&new_state, col);

        int score = -negamax(&new_state, -beta, -alpha);
        
        // Pruning
        if (score >= beta) return score;
        if (score > alpha) alpha = score;
    }

    return alpha;
}
