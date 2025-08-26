#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "engine.h"
#include "bitboard.h"
#include "table.h"
#include "player.h"
#include "interface.h"

// Parses a command-line argument to determine the player type.
PlayerType parse_player_type(const char* arg) {
    if (strcmp(arg, "ai") == 0) return PLAYER_TYPE_AI;
    return PLAYER_TYPE_HUMAN; // Default to human player.
}

int main(int argc, char *argv[]) {
    // Handle command-line arguments for player types.
    if (argc != 3 && argc != 1) {
        fprintf(stderr, "Usage: %s [human|ai] [human|ai]\n", argv[0]);
        fprintf(stderr, "Defaulting to: human ai\n");
    }

    init_solver();
    init_table();

    // Setup players based on arguments or defaults.
    Player p1 = { .type = (argc == 3) ? parse_player_type(argv[1]) : PLAYER_TYPE_HUMAN, .symbol = 'O' };
    Player p2 = { .type = (argc == 3) ? parse_player_type(argv[2]) : PLAYER_TYPE_AI, .symbol = 'X' };
    Player* current_player = &p1;

    GameState game;
    init_gamestate(&game);

    // Main game loop.
    while (game.moves < WIDTH * HEIGHT) {
        draw_board(&game, &p1, &p2);
        printf("Player %c's turn (%s).\n", current_player->symbol, current_player->type == PLAYER_TYPE_AI ? "AI" : "Human");

        int move = get_player_move(current_player, &game);
        if (move < 0) {
            printf("Player %c has no moves and forfeits.\n", current_player->symbol);
            break;
        }

        // Check for a winning move before playing it.
        if (is_winning_move(&game, move)) {
            play_move(&game, move);
            draw_board(&game, &p1, &p2);
            announce_result(current_player);
            free_table();
            return 0;
        }

        play_move(&game, move);

        // Switch to the next player.
        current_player = (current_player == &p1) ? &p2 : &p1;
    }

    // If the loop finishes, the board is full, resulting in a draw.
    if (game.moves == WIDTH * HEIGHT) {
        draw_board(&game, &p1, &p2);
        announce_result(NULL); // NULL indicates a draw.
    }

    free_table();
    return 0;
}