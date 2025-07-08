#include <stdio.h>
#include "bitboard.h"

/*
Bitboard representation
Each column has a padding bit so that the bottom mask can quickly check filled positions

.  .  .  .  .  .  .   <- Padding row
5 12 19 26 33 40 47
4 11 18 25 32 39 46
3 10 17 24 31 38 45
2  9 16 23 30 37 44
1  8 15 22 29 36 43
0  7 14 21 28 35 42 
*/

inline bool is_win(const uint64_t pos) {
    uint64_t m;

    // Horizontal
    m = pos & (pos >> PHEIGHT);
    if (m & (m >> (2 * PHEIGHT))) return true;

    // Vertical
    m = pos & (pos >> 1);
    if (m & (m >> 2)) return true;

    // Diagonal (y = x)
    m = pos & (pos >> (PHEIGHT + 1));
    if (m & (m >> (2 * (PHEIGHT + 1)))) return true;

    // Diagonal (y = -x)
    m = pos & (pos >> (PHEIGHT - 1));
    if (m & (m >> (2 * (PHEIGHT - 1)))) return true;

    return false;
}

inline bool is_draw(const GameState* state) {
    return state->moves == HEIGHT * WIDTH;
}

inline bool can_play(const GameState* state, int col) {
    uint64_t top_bit_mask = 1ULL << (col * PHEIGHT + HEIGHT);
    return (state->bottom & top_bit_mask) == 0;
}

inline int get_legal_moves(const GameState* state, int* move_list) {
    int i = 0;
    for (int col = 0; col < WIDTH; col++) {
        if (can_play(state, col)) {
            move_list[i] = col;
            i++;
        }
    }
    return i;
}

inline void make_move(GameState* const state, int col) {
    const uint64_t mask = (1ULL << PHEIGHT) - 1;
    const uint64_t col_mask = mask << (col * PHEIGHT);
    const uint64_t dropped_location = state->bottom & col_mask;

    state->current_player |= dropped_location;
    state->filled |= dropped_location;
    state->bottom ^= dropped_location | (dropped_location << 1);

    state->current_player ^= state->filled; 
    state->moves++;
}

inline void undo_move(GameState* const state, int col) {
    const uint64_t mask = (1ULL << PHEIGHT) - 1;
    const uint64_t col_mask = mask << (col * PHEIGHT);
    const uint64_t dropped_location = (state->bottom & col_mask) >> 1;

    state->moves--;
    state->current_player ^= state->filled;

    state->current_player ^= dropped_location;
    state->filled ^= dropped_location;
    state->bottom ^= dropped_location | (dropped_location << 1);
}

inline bool is_winning_move(GameState* const state, int col) {
    make_move(state, col);
    bool win = is_win(state->filled ^ state->current_player);
    undo_move(state, col);
    return win;
}

void print_bitboard(uint64_t board) {
    for (int row = HEIGHT; row >= 0; --row) {
        if (row == HEIGHT) {
            printf("PAD | ");
        } else {
            printf(" %d  | ", row);
        }

        // Loop through each column
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
