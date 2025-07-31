#include "player.h"
#include <stdio.h>

// A private helper function to get input from a human.
static int get_human_move(const GameState* game) {
    int col = -1;
    while (1) {
        printf("Enter your move (1-7): ");
        char buffer[10];
        if (fgets(buffer, sizeof(buffer), stdin)) {
            // Safely parse integer from buffer
            if (sscanf(buffer, "%d", &col) == 1 && col >= 1 && col <= WIDTH) {
                if (can_play(game, col - 1)) {
                    return col - 1; // Return 0-indexed column
                } else {
                    printf("Error: Column %d is full.\n", col);
                }
            } else {
                printf("Error: Invalid input. Please enter a number from 1 to 7.\n");
            }
        }
    }
}

// The public function that dispatches to the correct move-getter.
int get_player_move(const Player* player, const GameState* game) {
    switch (player->type) {
        case PLAYER_TYPE_AI:
            printf("AI is thinking...\n");
            return find_best_move(game);
        case PLAYER_TYPE_HUMAN:
        default:
            return get_human_move(game);
    }
}
