#include "interface.h"
#include <stdio.h>

void draw_board(const GameState* state, const Player* p1, const Player* p2) {
    printf("\n");
    for (int r = HEIGHT - 1; r >= 0; r--) {
        printf("| ");
        for (int c = 0; c < WIDTH; c++) {
            uint64_t pos = 1ULL << (c * PHEIGHT + r);
            if (!(state->mask & pos)) {
                printf(". ");
            } else {
                // Determine if the piece belongs to the current player ('X') or the other ('O')
                // This logic correctly displays the board from a neutral perspective.
                bool is_current_player_piece = (state->current_position & pos) != 0;
                // If an even number of moves have been played, the first player ('O') is the "current" one in the state
                if ((state->moves % 2) == 0) {
                     printf("%c ", is_current_player_piece ? p1->symbol : p2->symbol);
                } else { // otherwise the second player ('X') is
                     printf("%c ", is_current_player_piece ? p2->symbol : p1->symbol);
                }
            }
        }
        printf("|\n");
    }
    printf("-------------------\n| 1 2 3 4 5 6 7 |\n\n");
}


void announce_result(const Player* winner) {
    if (winner) {
        printf("Game over! Player %c wins!\n", winner->symbol);
    } else {
        printf("Game over! It's a draw.\n");
    }
}