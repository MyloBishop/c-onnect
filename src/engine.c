#include "engine.h"
#include "table.h"
#include "ordering.h"

#include <assert.h>
#include <limits.h>

// --- Engine State ---

uint64_t g_nodes_searched;
static int column_order[WIDTH];

// --- Score Encoding/Decoding for Transposition Table ---

/**
 * @brief Encodes a score and its bound type (alpha/beta) into a single uint8_t.
 * The value 0 is reserved for "entry not found".
 *
 * The score range is [MIN_SCORE, MAX_SCORE].
 *
 * Upper bounds (alpha) are mapped to the range:
 * [1, MAX_SCORE - MIN_SCORE + 1]
 *
 * Lower bounds (beta) are mapped to the range:
 * [MAX_SCORE - MIN_SCORE + 2, MAX_SCORE - MIN_SCORE + 2 + (MAX_SCORE - MIN_SCORE)]
 *
 * This ensures that lower bound values are always greater than any upper bound value,
 * allowing us to distinguish them with a simple comparison.
 */

// Returns an encoded value for an upper bound score (alpha)
static inline uint8_t encode_upper_bound(int score) {
    return (uint8_t)(score - MIN_SCORE + 1);
}
// Returns an encoded value for a lower bound score (beta)
static inline uint8_t encode_lower_bound(int score) {
    return (uint8_t)(score + MAX_SCORE - 2 * MIN_SCORE + 2);
}
// Decodes a stored value, assuming it's an upper bound
static inline int decode_upper_bound(uint8_t value) {
    return (int)value + MIN_SCORE - 1;
}
// Decodes a stored value, assuming it's a lower bound
static inline int decode_lower_bound(uint8_t value) {
    return (int)value + 2 * MIN_SCORE - MAX_SCORE - 2;
}
// A value is a lower bound if it's greater than this threshold
static inline bool is_lower_bound(uint8_t val) {
    return val >= MAX_SCORE - MIN_SCORE + 2;
}

// --- Private Functions ---

static int negamax(const GameState* P, int alpha, int beta) {
    assert(alpha < beta);
    assert(!can_win_next(P));

    g_nodes_searched++;

    if (is_draw(P)) {
        return 0;
    }

    uint64_t possible = possible_non_losing_moves(P);
    if (possible == 0) { // If no non-losing moves, we lose
        return -((WIDTH * HEIGHT - P->moves) / 2);
    }
    
    int min = -(WIDTH * HEIGHT - 2 - P->moves) / 2;
    if (alpha < min) {
        alpha = min;
        if(alpha >= beta) return alpha;
    }

    // Check if the search window is outside the possible score range
    int max_score = (WIDTH * HEIGHT - 1 - P->moves) / 2;
    if (beta > max_score) {
        beta = max_score;
        if (alpha >= beta) return beta;
    }
    
    // Check transposition table
    const uint64_t key = get_key(P);
    uint8_t val = table_get(key);
    if (val != 0) {
        if (is_lower_bound(val)) { // We have a lower bound
            int lower_bound = decode_lower_bound(val);
            if (alpha < lower_bound) {
                alpha = lower_bound;
                if (alpha >= beta) return alpha;
            }
        } else { // We have an upper bound
            int upper_bound = decode_upper_bound(val);
            if (beta > upper_bound) {
                beta = upper_bound;
                if (alpha >= beta) return beta;
            }
        }
    }

    MoveSorter sorter;
    sorter_init(&sorter);
    for (int i = WIDTH; i-- > 0; ) {
        uint64_t move = possible & column_mask(column_order[i]);
        if (move) {
            sorter_add(&sorter, move, move_score(P, move));
        }
    }

   // The loop over moves
    uint64_t next_move;
    while ((next_move = sorter_get_next(&sorter))) {
        GameState P2 = *P;
        play_move(&P2, bitboard_to_col(next_move));

        int score = -negamax(&P2, -beta, -alpha);

        if (score >= beta) {
            table_put(key, encode_lower_bound(score));
            return score; // Correct beta-cutoff
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    // Store and return the final alpha value.
    table_put(key, encode_upper_bound(alpha));
    return alpha;
}

// --- Public API Implementations ---

void init_solver(void) {
    // Initializes column exploration order, starting from the center columns.
    for (int i = 0; i < WIDTH; i++) {
        column_order[i] = WIDTH / 2 + (1 - 2 * (i % 2)) * ((i + 1) / 2);
    }
    reset_solver();
}

void reset_solver(void) {
    g_nodes_searched = 0;
}

int solve(const GameState* state, bool weak) {
    if (can_win_next(state)) {
        return (WIDTH * HEIGHT + 1 - state->moves) / 2;
    }

    int min = -(WIDTH * HEIGHT - state->moves) / 2;
    int max = (WIDTH * HEIGHT + 1 - state->moves) / 2;
    if (weak) {
        min = -1;
        max = 1;
    }

    while (min < max) {
        int med = min + (max - min) / 2;
        if (med <= 0 && min / 2 < med) med = min / 2;
        else if (med >= 0 && max / 2 > med) med = max / 2;

        int r = negamax(state, med, med + 1);
        if (r > med) {
            min = r;
        } else {
            max = r;
        }
    }
    return min;
}

int find_best_move(const GameState* state) {
    int best_move = -1;
    int best_score = INT_MIN;

    // Get all possible non-losing moves.
    uint64_t moves = possible_non_losing_moves(state);
    if (moves == 0) { // If all moves lead to a loss, find any valid move.
        // This is a fallback, in a solved game this branch should not be hit
        // unless the game is already lost.
        for (int col = 0; col < WIDTH; ++col) {
            if (can_play(state, col)) {
                moves |= column_mask(col);
            }
        }
    }

    // Iterate through each column to find potential moves
    for (int col = 0; col < WIDTH; ++col) {
        uint64_t move = moves & column_mask(col);
        if (move) {
            // Create a copy of the state to simulate the move
            GameState next_state = *state;
            play_move(&next_state, col);

            // The score is negated because negamax returns the score from the opponent's perspective.
            int score = -solve(&next_state, false);

            // If this move is better than any found so far, update the best move.
            if (score > best_score) {
                best_score = score;
                best_move = col;
            }
        }
    }
    return best_move;
}