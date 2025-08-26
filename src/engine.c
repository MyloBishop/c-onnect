#include "engine.h"
#include "table.h"
#include "ordering.h"
#include "book.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

// Engine State
uint64_t g_nodes_searched;
static int column_order[WIDTH];

// Score Encoding/Decoding for Transposition Table
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

// Private Functions
static int negamax(const GameState* P, int alpha, int beta) {
    assert(alpha < beta);
    assert(!can_win_next(P)); // The parent should have already checked for winning moves.

    g_nodes_searched++;

    if (is_draw(P)) {
        return 0;
    }

    // We can prune moves that let the opponent win on the next turn.
    uint64_t possible = possible_non_losing_moves(P);
    if (possible == 0) { // If no non-losing moves, we lose.
        return -((WIDTH * HEIGHT - P->moves) / 2);
    }
    
    // Tighten the search window based on the best/worst possible score from this position.
    int min = -(WIDTH * HEIGHT - 2 - P->moves) / 2;
    if (alpha < min) {
        alpha = min;
        if(alpha >= beta) return alpha;
    }
    int max_score = (WIDTH * HEIGHT - 1 - P->moves) / 2;
    if (beta > max_score) {
        beta = max_score;
        if (alpha >= beta) return beta;
    }
    
    // Probe the transposition table for a stored score.
    const uint64_t key = get_key(P);
    uint8_t val = table_get(key);
    if (val != 0) {
        if (is_lower_bound(val)) { // We have a lower bound.
            int lower_bound = decode_lower_bound(val);
            if (alpha < lower_bound) {
                alpha = lower_bound;
                if (alpha >= beta) return alpha;
            }
        } else { // We have an upper bound.
            int upper_bound = decode_upper_bound(val);
            if (beta > upper_bound) {
                beta = upper_bound;
                if (alpha >= beta) return beta;
            }
        }
    }

    // Order moves to improve alpha-beta pruning efficiency.
    MoveSorter sorter;
    sorter_init(&sorter);
    for (int i = WIDTH; i-- > 0; ) {
        uint64_t move = possible & column_mask(column_order[i]);
        if (move) {
            sorter_add(&sorter, move, move_score(P, move));
        }
    }

    // The loop over moves.
    uint64_t next_move;
    while ((next_move = sorter_get_next(&sorter))) {
        GameState P2 = *P;
        play_move(&P2, bitboard_to_col(next_move));

        // Recursive call for the opponent with a flipped score and window.
        int score = -negamax(&P2, -beta, -alpha);

        if (score >= beta) {
            // Store a lower bound in the transposition table.
            table_put(key, encode_lower_bound(score));
            return score; // Beta-cutoff: opponent will avoid this line.
        }
        if (score > alpha) {
            alpha = score; // Found a new best move.
        }
    }

    // Store the final alpha value (an upper bound) and return it.
    table_put(key, encode_upper_bound(alpha));
    return alpha;
}

// Public API Implementations
void init_solver(void) {
    // Initialize column order to check center columns first, which are generally stronger.
    for (int i = 0; i < WIDTH; i++) {
        column_order[i] = WIDTH / 2 + (1 - 2 * (i % 2)) * ((i + 1) / 2);
    }
    reset_solver();
    init_book();
    atexit(free_book); // Ensure memory is freed on exit.
}

void reset_solver(void) {
    g_nodes_searched = 0;
}

int solve(const GameState* state, bool weak) {
    // If we can win on the next move, return the score for the fastest win.
    if (can_win_next(state)) {
        return (WIDTH * HEIGHT + 1 - state->moves) / 2;
    }

    // Set the initial score search range.
    int min = -(WIDTH * HEIGHT - state->moves) / 2;
    int max = (WIDTH * HEIGHT + 1 - state->moves) / 2;
    if (weak) { // A "weak" solve only checks for win/loss/draw.
        min = -1;
        max = 1;
    }

    // Binary search the score to find the exact value.
    while (min < max) {
        int med = min + (max - min) / 2;
        // Tweak the search pivot to be closer to 0, a more likely score, to speed up convergence.
        if (med <= 0 && min / 2 < med) med = min / 2;
        else if (med >= 0 && max / 2 > med) med = max / 2;

        int r = negamax(state, med, med + 1); // Use a minimal window search.
        if (r > med) {
            min = r; // The score is in [r, max].
        } else {
            max = r; // The score is in [min, r].
        }
    }
    return min;
}

int find_best_move(const GameState* state) {
    // Check the opening book for a move in the early game.
    if (state->moves < MAX_BOOK_DEPTH) {
        #ifdef DEBUG
        fprintf(stderr, "DEBUG: Checking book for state with %d moves. Key components (Mask/Pos): %llu / %llu\n", 
               state->moves, state->mask, state->current_position);
        #endif
        int book_move = -1;
        uint128_t key = book_compute_key(state);
        if (book_get_move(key, &book_move)) {
            assert(can_play(state, book_move));
            return book_move;
        }
    }

    int best_move = -1;
    int best_score = INT_MIN;
    
    // Iterate through all valid moves to find the one with the best score.
    for (int col = 0; col < WIDTH; ++col) {
        uint64_t move = possible(state) & column_mask(col);
        if (move) {
            // An immediate win is always the best possible move.
            if (is_winning_move(state, col)) return col;

            GameState next_state = *state;
            play_move(&next_state, col);

            // The score of our move is the negative of the opponent's score after our move.
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