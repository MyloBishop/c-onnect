#ifndef BITBOARD_H
#define BITBOARD_H

// NB (HEIGHT+1)*WIDTH must be less than 64
#define HEIGHT 6
#define PHEIGHT (HEIGHT + 1)
#define WIDTH 7

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint64_t current_player; // 1s in current player's filled positions
    uint64_t filled;         // 1s in all filled positions
    uint8_t moves;           // Number of moves played, max is HEIGHT*WIDTH (42)
} GameState;

static inline uint64_t top_mask(int col) {
    return (1ULL << (HEIGHT - 1)) << col * PHEIGHT;
}

static inline uint64_t bottom_mask(int col) {
    return 1ULL << col * PHEIGHT;
}

static inline uint64_t column_mask(int col) {
    return ((1ULL << HEIGHT) - 1) << col * PHEIGHT;
}

static inline uint64_t highest_bit(uint64_t board, int col) {
    uint64_t mask = column_mask(col) & board;
    if (mask == 0) {return 0;}
    return 1ULL << (63 - __builtin_clzll(mask));
}

static inline bool is_win(const uint64_t pos) {
    uint64_t m;

    // Horizontal check
    m = pos & (pos >> PHEIGHT);
    if (m & (m >> (2 * PHEIGHT))) return true;

    // Vertical check
    m = pos & (pos >> 1);
    if (m & (m >> 2)) return true;

    // Diagonal (y = x) check
    m = pos & (pos >> (PHEIGHT + 1));
    if (m & (m >> (2 * (PHEIGHT + 1)))) return true;

    // Diagonal (y = -x) check
    m = pos & (pos >> (PHEIGHT - 1));
    if (m & (m >> (2 * (PHEIGHT - 1)))) return true;

    return false;
}

static inline bool is_draw(const GameState* state) {
    return state->moves == HEIGHT * WIDTH;
}

static inline bool can_play(const GameState* state, int col) {
    return (state->filled & top_mask(col)) == 0;
}

static inline void make_move(GameState* state, int col) {
    state->current_player ^= state->filled;
    state->filled |= state->filled + bottom_mask(col);
    state->moves++;
}

static inline bool is_winning_move(const GameState* state, int col) {
    uint64_t pos = state->current_player;
    pos |= (state->filled + bottom_mask(col)) & column_mask(col);
    return is_win(pos);
}


#endif

#ifdef DEBUG

void print_bitboard(uint64_t board);

void print_board(const GameState* state);

#endif
