#ifndef BITBOARD_H
#define BITBOARD_H

// NB (HEIGHT+1)*WIDTH must be less than 64
#define HEIGHT 6
#define PHEIGHT (HEIGHT + 1)
#define WIDTH 7

#include <stdint.h>
#include <stdbool.h>


typedef struct {
    uint64_t current_player; // 1 in current player filled positions
    uint64_t filled; // 1 in filled positions
    uint64_t bottom; // where the next stone will be placed
    uint8_t moves; // max number of moves is 42
} GameState;

bool is_win(const uint64_t pos);

bool is_draw(const GameState* state);

bool can_play(const GameState*, int col);

// Populates move_list with all legal moves and returns the count
int get_legal_moves(const GameState* state, int* move_list);

void make_move(GameState* const state, int col);

void undo_move(GameState* const state, int col);

bool is_winning_move(GameState* const state, int col);

void print_board(const GameState* state);


#endif
