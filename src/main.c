#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "engine.h"
#include "bitboard.h"

#define MAX_LINE_LEN 256

extern uint64_t g_nodes_searched;

void init_gamestate(GameState* state) {
    memset(state, 0, sizeof(GameState));
    for (int col = 0; col < WIDTH; ++col) {
        state->bottom |= (1ULL << (col * PHEIGHT));
    }
}

int setup_board(GameState* game, const char* move_string) {
    init_gamestate(game);
    for (size_t i = 0; i < strlen(move_string); ++i) {
        char move_char = move_string[i];
        if (!isdigit(move_char)) {
            fprintf(stderr, "Error: Invalid char '%c' in position '%s'.\n", move_char, move_string);
            return 0;
        }
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
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <path_to_test_file>\n", argv[0]);
        return 1;
    }
    const char* test_file_path = argv[1];
    FILE *file = fopen(test_file_path, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open test file '%s'.\n", test_file_path);
        return 1;
    }

    char line[MAX_LINE_LEN];
    int line_num = 0;
    uint64_t total_nodes = 0;
    double total_time_sec = 0.0;
    int positions_processed = 0;

    while (fgets(line, sizeof(line), file)) {
        line_num++;
        if (line[0] == '\n' || line[0] == '\0') continue;

        char move_string[MAX_LINE_LEN];
        int expected_score;
        if (sscanf(line, "%s %d", move_string, &expected_score) != 2) {
            fprintf(stderr, "Error: Malformed line #%d in file '%s'.\n", line_num, test_file_path);
            fclose(file);
            return 1;
        }

        GameState game;
        if (!setup_board(&game, move_string)) {
            fclose(file);
            return 1;
        }

        g_nodes_searched = 0;
        const int alpha = -WIDTH * HEIGHT / 2;
        const int beta = WIDTH * HEIGHT / 2;

        clock_t start = clock();
        int actual_score = negamax(&game, alpha, beta);
        clock_t end = clock();

        if (actual_score != expected_score) {
            fprintf(stderr, "FAIL [Line %d]: Position '%s'\n", line_num, move_string);
            fprintf(stderr, "  -> Expected: %d, Got: %d\n", expected_score, actual_score);
            fclose(file);
            return 1;
        }

        total_nodes += g_nodes_searched;
        total_time_sec += (double)(end - start) / CLOCKS_PER_SEC;
        positions_processed++;
    }

    fclose(file);

    // Convert total time to microseconds for high-resolution integer output
    long long total_microseconds = (long long)(total_time_sec * 1e6);

    // Print raw data: passed_count, total_nodes, total_microseconds
    fprintf(stdout, "%d %llu %lld", positions_processed, total_nodes, total_microseconds);

    return 0;
}
