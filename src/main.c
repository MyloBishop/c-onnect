#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "engine.h"
#include "bitboard.h"
#include "table.h"

extern uint64_t g_nodes_searched;

// Initializes the game state
void init_gamestate(GameState* state) {
    memset(state, 0, sizeof(GameState));
    reset_table();
}

// Sets up the board from a move string
int setup_board(GameState* game, const char* move_string) {
    init_gamestate(game);
    for (size_t i = 0; i < strlen(move_string); ++i) {
        char move_char = move_string[i];
        if (!isdigit(move_char)) {
            fprintf(stderr, "Error: Invalid char '%c' in position '%s'.\n", move_char, move_string);
            return 0;
        }
        // Moves are 1-indexed in the input string
        int col = (move_char - '0') - 1;
        if (col < 0 || col >= WIDTH) {
            fprintf(stderr, "Error: Invalid column '%c' in position '%s'.\n", move_char, move_string);
            return 0;
        }
        if (!can_play(game, col)) {
            fprintf(stderr, "Error: Full column %d for position '%s'.\n", col + 1, move_string);
            return 0;
        }
        make_move(game, col);
    }
    return 1;
}

int main(int argc, char *argv[]) {
    // Expect a single command-line argument: the move string
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <move_string>\n", argv[0]);
        return 1;
    }
    const char* move_string = argv[1];

    GameState game;
    if (!setup_board(&game, move_string)) {
        // Error message is printed inside setup_board
        return 1;
    }

    g_nodes_searched = 0;

    clock_t start = clock();
    int score = solve(&game);
    clock_t end = clock();

    double time_sec = ((double)(end - start) * 1e6) / CLOCKS_PER_SEC;
    long long time_microsec = (long long)(time_sec);

    // Output the results as a single, space-separated line
    // Format: score nodes_searched time_microseconds
    fprintf(stdout, "%d %llu %lld", score, (unsigned long long)g_nodes_searched, time_microsec);

    return 0;
}
