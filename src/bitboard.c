#include "bitboard.h"

/*
Bitboard representation
Each column has a padding bit so that the bottom mask can quickly check filled positions

.  .  .  .  .  .  .   <- Padding row (bit 6)
5 12 19 26 33 40 47
4 11 18 25 32 39 46
3 10 17 24 31 38 45
2  9 16 23 30 37 44
1  8 15 22 29 36 43
0  7 14 21 28 35 42
*/

// Note: Most functions have been moved to bitboard.h as `static inline`
// to resolve linker errors and improve performance. Only larger, non-inlined
// functions are left here.

// Debugging functions

#ifdef DEBUG
#include <stdio.h>

void print_bitboard(uint64_t board) {
    printf("Bitboard State:\n");
    printf("   +---------------+\n");

    for (int r = HEIGHT; r >= 0; --r) {
        // Column labels 
        if (r == HEIGHT) {
            printf(" P | ");
        } else {
            printf(" %d | ", r);
        }

        for (int c = 0; c < WIDTH; ++c) {
            // Create a mask for the current bit
            uint64_t bit_index = c * PHEIGHT + r;
            uint64_t mask = 1ULL << bit_index;

            // Check for a 1 set in current position and print result
            if (board & mask) {
                printf("X ");
            } else {
                printf(". ");
            }
        }
        printf("|\n");

        if (r == HEIGHT) {
            printf("   +---------------+\n");
        }
    }

    printf("   +---------------+\n");
    printf("     0 1 2 3 4 5 6\n\n");
}


void print_board(const GameState* state) {
    printf("Board State (Moves: %d)\n", state->moves);
    printf("+---------------+\n");

    // Player 'X' is the player who just moved
    uint64_t p1_board = state->current_player ^ state->filled;
    // Player 'O' is the current player
    uint64_t p2_board = state->current_player;

    for (int r = HEIGHT - 1; r >= 0; --r) {
        printf("| ");
        for (int c = 0; c < WIDTH; ++c) {
            // Create a mask for the current bit
            uint64_t bit_index = c * PHEIGHT + r;
            uint64_t mask = 1ULL << bit_index;

            // Check the mask against each player's board and print the result
            if (p1_board & mask) {
                printf("X ");
            } else if (p2_board & mask) {
                printf("O ");
            } else {
                printf(". ");
            }
        }
        printf("|\n");
    }
    printf("+---------------+\n");
    printf("  1 2 3 4 5 6 7\n\n");
}

#endif
