#include "engine.h"
#include "bitboard.h"
#include "table.h"
#include "ordering.h"
#include <stddef.h>
#include <assert.h>

uint64_t g_nodes_searched = 0;

int negamax(GameState* const state, int alpha, int beta) {
    assert(alpha < beta);
    g_nodes_searched++;

    uint64_t moves_mask = possible_non_losing_moves(state);
    if (moves_mask == 0) { // If all moves are losing, return the worst score
        return -(POSITION_COUNT - state->moves) / 2;
    }

    if (state->moves >= POSITION_COUNT - 2) { // Check for draw game
        return 0;
    }

    // Lower bound alpha as opponent cannot win next move
    int min = -(POSITION_COUNT - 2 - state->moves) / 2;
    if (alpha < min) {
        alpha = min;
        if (alpha >= beta) return alpha;
    }

    // Upper bound beta as we cannot win immediately
    int max = (POSITION_COUNT - 1 - state->moves) / 2;
    if (beta > max) {
        beta = max;
        if (alpha >= beta) return beta;
    }

    // Check transposition table
    uint64_t key = get_key(state);
    uint8_t table_val = get_table(key);

    if (table_val) {
        if (table_val > BOUND_SEPARATOR) { // It's a lower bound
            int min_score = DECODE_LOWER_BOUND(table_val);
            if (alpha < min_score) {
                alpha = min_score;
                if (alpha >= beta) return alpha;
            }
        } else { // It's an upper bound
            int max_score = DECODE_UPPER_BOUND(table_val);
            if (beta > max_score) {
                beta = max_score;
                if (alpha >= beta) return beta;
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
        assert(score >= MIN_SCORE && score <= MAX_SCORE);

        if (score >= beta) {
            put_table(get_key(state), VAL_LOWER_BOUND(score));
            return score;
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    put_table(get_key(state), VAL_UPPER_BOUND(alpha));
    return alpha;
}

int solve(GameState* const state) {
    int min = -(POSITION_COUNT - state->moves) / 2;
    int max = (POSITION_COUNT + 1 - state->moves) / 2;

    if (can_win_next(state)) {
        return max;
    }

    while (min < max) {
        int med = min + (max - min) / 2;
        
        // Bias towards 0
        if (med <= 0 && min / 2 < med) {
            med = min / 2;
        } else if (med >= 0 && max / 2 > med) {
            med = max / 2;
        }

        int r = negamax(state, med, med + 1);

        if (r <= med) {
            max = r;
        } else {
            min = r;
        }
    }
    return min;
}
