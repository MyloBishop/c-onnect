#ifndef BITBOARD_H
#define BITBOARD_H

// Board dimensions
#define HEIGHT 6
#define WIDTH 7
#define PHEIGHT (HEIGHT + 1) // Padded height to prevent column wrap-around
#define POSITION_COUNT (WIDTH * HEIGHT)
#define PIECE_COUNT 2

// Heuristic score bounds for the solver
#define MIN_SCORE (-(WIDTH*HEIGHT)/2 + 3)
#define MAX_SCORE ((WIDTH*HEIGHT+1)/2 - 3)


#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

// Represents the board state using bitboards
typedef struct {
    uint64_t current_player; // Bitmask of the current player's pieces
    uint64_t filled;         // Bitmask of all occupied slots
    uint8_t moves;           // Number of moves played
} GameState;

// Mask for the top row of a column
static inline uint64_t top_mask(int col) {
    return (1ULL << (HEIGHT - 1)) << col * PHEIGHT;
}

// Mask for the bottom row of a column
static inline uint64_t bottom_mask(int col) {
    return 1ULL << col * PHEIGHT;
}

// Mask for all rows in a column
static inline uint64_t column_mask(int col) {
    return ((1ULL << HEIGHT) - 1) << col * PHEIGHT;
}

// Mask for the entire playable board area
#define BOARD_MASK (column_mask(0) | column_mask(1) | column_mask(2) | column_mask(3) | column_mask(4) | column_mask(5) | column_mask(6))

// Mask for the bottom row of the entire board
#define BOTTOM_MASK (bottom_mask(0) | bottom_mask(1) | bottom_mask(2) | bottom_mask(3) | bottom_mask(4) | bottom_mask(5) | bottom_mask(6))

// Checks if a bitboard has a four-in-a-row
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

// Checks if the board is full
static inline bool is_draw(const GameState* state) {
    return state->moves == HEIGHT * WIDTH;
}

// Checks if a column is not full
static inline bool can_play(const GameState* state, int col) {
    return (state->filled & top_mask(col)) == 0;
}

// Places a piece and updates the state for the next player
static inline void make_move(GameState* state, int col) {
    state->current_player ^= state->filled;
    state->filled |= state->filled + bottom_mask(col);
    state->moves++;
}

// Checks if playing a move in a column results in a win
static inline bool is_winning_move(const GameState* state, int col) {
    uint64_t pos = state->current_player;
    pos |= (state->filled + bottom_mask(col)) & column_mask(col);
    return is_win(pos);
}

// Computes a bitmask of all squares where a player can win on the next move
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

    return r & (BOARD_MASK ^ mask); // Return only empty squares
}

// Returns a bitmask of all possible moves
static inline uint64_t possible(const GameState* state) {
    return (state->filled + BOTTOM_MASK) & BOARD_MASK;
}

// Computes the opponent's winning positions
static inline uint64_t opponent_winning_position(const GameState* state) {
    uint64_t opponent_player = state->filled ^ state->current_player;
    return compute_winning_position(opponent_player, state->filled);
}

// Checks if the current player can win on their next turn
static inline bool can_win_next(const GameState* state) {
    uint64_t winning_moves = compute_winning_position(state->current_player, state->filled);
    return (possible(state) & winning_moves) != 0;
}

// Returns a bitmask of moves that don't lead to an immediate loss
static inline uint64_t possible_non_losing_moves(const GameState* state) {
    assert(!can_win_next(state));
    uint64_t possible_mask = possible(state);
    uint64_t opponent_win = opponent_winning_position(state);
    uint64_t forced_moves = possible_mask & opponent_win;
    if (forced_moves) {
        if (forced_moves & (forced_moves - 1)) { // If opponent has multiple threats
            return 0; // Loss is unavoidable
        }
        possible_mask = forced_moves; // Must play the blocking move
    }
    return possible_mask & ~(opponent_win >> 1); // Avoid playing under an opponent's winning spot
}


#endif

#ifdef DEBUG

// Prints a raw bitboard for debugging
void print_bitboard(uint64_t board);

// Prints the game board in a human-readable format
void print_board(const GameState* state);

#endif