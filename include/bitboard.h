#ifndef BITBOARD_H
#define BITBOARD_H

#define HEIGHT 6
#define WIDTH 7
#define PHEIGHT (HEIGHT + 1)
#define POSITION_COUNT (WIDTH * HEIGHT)
#define PIECE_COUNT 2

#define MIN_SCORE (-(WIDTH*HEIGHT)/2 + 3)
#define MAX_SCORE ((WIDTH*HEIGHT+1)/2 - 3)


#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

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

#define BOARD_MASK (column_mask(0) | column_mask(1) | column_mask(2) | column_mask(3) | column_mask(4) | column_mask(5) | column_mask(6))

#define BOTTOM_MASK (bottom_mask(0) | bottom_mask(1) | bottom_mask(2) | bottom_mask(3) | bottom_mask(4) | bottom_mask(5) | bottom_mask(6))

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

static inline uint64_t compute_winning_position(uint64_t position, uint64_t mask) {
    // Vertical
    uint64_t r = (position << 1) & (position << 2) & (position << 3);

    // Horizontal
    uint64_t p = (position << PHEIGHT) & (position << (2 * PHEIGHT));
    r |= p & (position << (3 * PHEIGHT));
    r |= p & (position >> PHEIGHT);
    p = (position >> PHEIGHT) & (position >> (2 * PHEIGHT));
    r |= p & (position << PHEIGHT);
    r |= p & (position >> (3 * PHEIGHT));

    // Diagonal (y = -x)
    p = (position << (PHEIGHT - 1)) & (position << (2 * (PHEIGHT - 1)));
    r |= p & (position << (3 * (PHEIGHT - 1)));
    r |= p & (position >> (PHEIGHT - 1));
    p = (position >> (PHEIGHT - 1)) & (position >> (2 * (PHEIGHT - 1)));
    r |= p & (position << (PHEIGHT - 1));
    r |= p & (position >> (3 * (PHEIGHT - 1)));

    // Diagonal (y = x)
    p = (position << (PHEIGHT + 1)) & (position << (2 * (PHEIGHT + 1)));
    r |= p & (position << (3 * (PHEIGHT + 1)));
    r |= p & (position >> (PHEIGHT + 1));
    p = (position >> (PHEIGHT + 1)) & (position >> (2 * (PHEIGHT + 1)));
    r |= p & (position << (PHEIGHT + 1));
    r |= p & (position >> (3 * (PHEIGHT + 1)));

    return r & (BOARD_MASK ^ mask);
}

static inline uint64_t possible(const GameState* state) {
    return (state->filled + BOTTOM_MASK) & BOARD_MASK;
}

static inline uint64_t opponent_winning_position(const GameState* state) {
    uint64_t opponent_player = state->filled ^ state->current_player;
    return compute_winning_position(opponent_player, state->filled);
}

static inline bool can_win_next(const GameState* state) {
    uint64_t winning_moves = compute_winning_position(state->current_player, state->filled);
    return (possible(state) & winning_moves) != 0;
}

static inline uint64_t possible_non_losing_moves(const GameState* state) {
    assert(!can_win_next(state));
    uint64_t possible_mask = possible(state);
    uint64_t opponent_win = opponent_winning_position(state);
    uint64_t forced_moves = possible_mask & opponent_win;
    if (forced_moves) {
        if (forced_moves & (forced_moves - 1)) { // check if there is more than one forced move
            return 0; // the opponent has two winning moves and you cannot stop them
        }
        possible_mask = forced_moves; // enforce to play the single forced move
    }
    return possible_mask & ~(opponent_win >> 1); // avoid to play below an opponent winning spot
}


#endif

#ifdef DEBUG

void print_bitboard(uint64_t board);

void print_board(const GameState* state);

#endif
