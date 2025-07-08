#include "bitboard.h"
#include <stdio.h>

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

/**
 * @brief Prints a raw 64-bit integer as a Connect 4 board for debugging.
 * @param board The bitboard to print.
 */
void print_bitboard(uint64_t board) {
    for (int row = HEIGHT; row >= 0; --row) {
        if (row == HEIGHT) {
            printf("PAD | ");
        } else {
            printf(" %d  | ", row);
        }

        for (int col = 0; col < WIDTH; ++col) {
            uint64_t bit_index = col * PHEIGHT + row;
            uint64_t mask = 1ULL << bit_index;

            if (board & mask) {
                printf("1 ");
            } else {
                printf("0 ");
            }
        }
        printf("|\n");
        if (row == HEIGHT) {
             printf("----+-----------------+\n");
        }
    }
    printf("----+-----------------+\n");
    printf("      0 1 2 3 4 5 6  (Column Index)\n");
    printf("-------------------------\n");
}

/**
 * @brief Prints a user-friendly representation of the game state with 'X' and 'O'.
 * @param state A pointer to the current game state.
 */
void print_board(const GameState* state) {
    printf("Board State (Moves: %d)\n", state->moves);
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
