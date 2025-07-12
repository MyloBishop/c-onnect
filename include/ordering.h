#ifndef ORDERING_H
#define ORDERING_H

#include <stdint.h>
#include <stdlib.h>
#include "bitboard.h"

typedef struct {
    int col;
    int score;
} MoveScore;

// Helper for qsort to sort moves in descending score order.
static int compare_moves(const void* a, const void* b) {
    const MoveScore* moveA = (const MoveScore*)a;
    const MoveScore* moveB = (const MoveScore*)b;
    return moveB->score - moveA->score;
}

static inline int popcount(uint64_t n) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(n);
#else
    int count = 0;
    while (n > 0) {
        n &= (n - 1);
        count++;
    }
    return count;
#endif
}

// Calculates scores and sorts them into the move_scores array.
static inline void order_moves(const GameState* state, MoveScore* move_scores) {
    for (int col = 0; col < WIDTH; col++) {
        move_scores[col].col = col;
        if (can_play(state, col)) {
            // Simulate the move to calculate its score
            const uint64_t play_bit = (state->filled + bottom_mask(col)) & column_mask(col);
            const uint64_t next_player_board = state->current_player | play_bit;
            const uint64_t next_filled_board = state->filled | play_bit;

            // The score is the number of new winning positions created by this move.
            const uint64_t opportunities = compute_winning_position(next_player_board, next_filled_board);
            move_scores[col].score = popcount(opportunities);
        } else {
            // Assign a very low score to unplayable columns
            move_scores[col].score = -1;
        }
    }
    
    qsort(move_scores, WIDTH, sizeof(MoveScore), compare_moves);
}

#endif
