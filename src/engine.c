#include "engine.h"
#include "bitboard.h"
#include "table.h"
#include "ordering.h"
#include <stddef.h>
#include <assert.h>

uint64_t g_nodes_searched = 0;

int negamax(GameState* const state, int alpha, int beta) {
    assert(alpha < beta);

    g_nodes_searched++; // Increment node count on every entry

    // Get a bitmask of moves that do not lose in one turn.
    uint64_t moves_mask = possible_non_losing_moves(state);
    if (moves_mask == 0) { // If all moves are losing, return the worst score
        return -(WIDTH*HEIGHT - state->moves) / 2;
    }

    if (state->moves >= WIDTH * HEIGHT - 2) {
        return 0;
    }

    // Lower bound alpha as opponent cannot win next move
    int min = -(WIDTH*HEIGHT - 2 - state->moves) / 2;
    if (alpha < min) {
        alpha = min;
        if (alpha >= beta) return alpha; // Prune
    }

    // Upper bound beta as we cannot win immediately
    int max = (WIDTH*HEIGHT -  - state->moves) / 2;
    if (beta > max) {
        beta = max;
        if (alpha >= beta) return beta; // Prune
    }

    // Check transposition table
    uint64_t key = get_key(state);
    uint8_t table_val = get_table(key);

    if (table_val) {
        // Check if the stored value is a lower bound (a "fail-high" node)
        if (table_val > UPPER_BOUND_THRESHOLD) {
            int min_score = unmap_val(table_val - LOWER_BOUND_OFFSET);

            if (alpha < min_score) {
                alpha = min_score;
                if (alpha >= beta) return alpha; // Prune
            }
        }
        // Otherwise, it's an upper bound (a "fail-low" node)
        else {
            max = unmap_val(table_val);
            if (beta > max) {
                beta = max;
                if (alpha >= beta) return beta; // Prune
            }
        }
    }

    int move_order[WIDTH];
    int num_moves = sort_moves(state, move_order, moves_mask);

    for (int i = 0; i < num_moves; i++) {
        int col = move_order[i];

        GameState new_state = *state;
        make_move(&new_state, col);

        int score = -negamax(&new_state, -beta, -alpha);

        // Pruning
        if (score >= beta) {
            // Store lower bound
            put_table(get_key(state), map_val(score) + LOWER_BOUND_OFFSET);
            return score;
        }
        if (score > alpha) alpha = score;
    }

    put_table(get_key(state), map_val(alpha));
    return alpha;
}

int solve(GameState* const state) {
    int min = -(WIDTH * HEIGHT - state->moves) / 2;
    int max = (WIDTH * HEIGHT + 1 - state->moves) / 2;
    
    if (can_win_next(state)) {
        return max;
    }

    while (min < max) {
        int med = min + (max - min) / 2;
        
        // bias towards 0
        if (med <= 0 && min / 2 < med) {
            med = min / 2;
        } else if (med >= 0 && max / 2 > med) {
            med = max / 2;
        }

        int r = negamax(state, med, med + 1);

        if (r <= med) max = r;
        else min = r;
    }
    return min;
}
