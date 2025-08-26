#include "interface.h"
#include <stdio.h>

// Draws the current state of the game board to the console.
void draw_board(const GameState* state, const Player* p1, const Player* p2) {
    printf("\n");
    for (int r = HEIGHT - 1; r >= 0; r--) {
        printf("| ");
        for (int c = 0; c < WIDTH; c++) {
            uint64_t pos = 1ULL << (c * PHEIGHT + r);
            if (!(state->mask & pos)) {
                printf(". ");
            } else {
                // Determine which player's piece is on the square.
                bool is_current_player_piece = (state->current_position & pos) != 0;
                if ((state->moves % 2) == 0) { // Player 1 just moved.
                     printf("%c ", is_current_player_piece ? p1->symbol : p2->symbol);
                } else { // Player 2 just moved.
                     printf("%c ", is_current_player_piece ? p2->symbol : p1->symbol);
                }
            }
        }
        printf("|\n");
    }
    printf("-------------------\n| 1 2 3 4 5 6 7 |\n\n");
}


// Announces the winner of the game or if it's a draw.
void announce_result(const Player* winner) {
    if (winner) {
        printf("Game over! Player %c wins!\n", winner->symbol);
    } else {
        printf("Game over! It's a draw.\n");
    }
}