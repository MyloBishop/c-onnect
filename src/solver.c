#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#include "engine.h"
#include "bitboard.h"
#include "table.h"

// Sets up the board from a move string, returning 1 on success, 0 on error.
static int setup_board(GameState* game, const char* move_string) {
    init_gamestate(game);
    reset_solver();
    reset_table();

    for (size_t i = 0; i < strlen(move_string); ++i) {
        char move_char = move_string[i];
        if (!isdigit(move_char) || move_char == '0') {
            fprintf(stderr, "Error: Invalid char '%c' in position '%s'.\n", move_char, move_string);
            return 0;
        }
        
        int col = (move_char - '0') - 1; // Moves are 1-indexed in the string
        if (col < 0 || col >= WIDTH) {
            fprintf(stderr, "Error: Invalid column '%c' in position '%s'.\n", move_char, move_string);
            return 0;
        }
        if (!can_play(game, col)) {
            fprintf(stderr, "Error: Column %d is full for position '%s'.\n", col + 1, move_string);
            return 0;
        }
        // The solver assumes the input position has no win for either player.
        if (is_winning_move(game, col)) {
             fprintf(stderr, "Error: Position '%s' contains a winning move, which is not supported.\n", move_string);
             return 0;
        }
        play_move(game, col);
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <move_string>\n", argv[0]);
        return 1;
    }

    // Initialize solver modules
    init_solver();
    init_table();

    GameState game;
    if (!setup_board(&game, argv[1])) {
        free_table(); // Clean up on error
        return 1;
    }

    uint64_t key = get_key(&game);
    
    clock_t start = clock();
    int score = solve(&game, false);
    clock_t end = clock();

    double time_sec = ((double)(end - start)) / CLOCKS_PER_SEC;

    // Output format: score nodes_searched time_microseconds
    fprintf(stdout, "%llu %d %llu %lld\n", key, score, (unsigned long long)g_nodes_searched, (long long)(time_sec * 1e6));

    // Clean up resources
    free_table();

    return 0;
}
