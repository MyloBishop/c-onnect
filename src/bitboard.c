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

    // Iterate from the padding row down to the bottom row.
    for (int r = HEIGHT; r >= 0; --r) {
        // Print a label for the current row.
        if (r == HEIGHT) {
            printf(" P | ");
        } else {
            // Add a space for alignment with "PAD"
            printf(" %d | ", r);
        }

        // Print the contents of the row.
        for (int c = 0; c < WIDTH; ++c) {
            uint64_t bit_index = c * PHEIGHT + r;
            uint64_t mask = 1ULL << bit_index;

            if (board & mask) {
                printf("X "); // A piece (or filled padding bit)
            } else {
                printf(". "); // An empty spot
            }
        }
        printf("|\n");

        // Print a separator line after the padding row.
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
    char board[HEIGHT][WIDTH];

    // Initialize board with empty spaces
    for (int r = 0; r < HEIGHT; ++r) {
        for (int c = 0; c < WIDTH; ++c) {
            board[r][c] = '.';
        }
    }

    // Determine each player's positions
    // The player who just moved is in `current_player ^ filled`
    uint64_t p1_board = state->current_player ^ state->filled;
    uint64_t p2_board = state->current_player;

    // Populate the char array with 'X' and 'O'
    for (int c = 0; c < WIDTH; ++c) {
        for (int r = 0; r < HEIGHT; ++r) {
            uint64_t mask = 1ULL << (c * PHEIGHT + r);
            if (p1_board & mask) {
                board[r][c] = 'X';
            } else if (p2_board & mask) {
                board[r][c] = 'O';
            }
        }
    }

    // Print the board from top to bottom
    for (int r = HEIGHT - 1; r >= 0; --r) {
        printf("| ");
        for (int c = 0; c < WIDTH; ++c) {
            printf("%c ", board[r][c]);
        }
        printf("|\n");
    }
    printf("+---------------+\n");
    printf("  1 2 3 4 5 6 7\n\n");
}

#endif
