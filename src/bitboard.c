#include "bitboard.h"
#include <string.h>

// A mask representing the bottom row of the board.
static const uint64_t BOTTOM_MASK = ((1ULL << (WIDTH * PHEIGHT)) - 1) / ((1ULL << PHEIGHT) - 1);

// A mask representing all playable squares on the board.
static const uint64_t BOARD_MASK = BOTTOM_MASK * ((1ULL << HEIGHT) - 1);

// Returns a mask for the top-most cell of a column.
static uint64_t top_mask_for_col(int col) {
    return 1ULL << ((HEIGHT - 1) + col * PHEIGHT);
}

// Returns a mask for the bottom-most cell of a column.
static uint64_t bottom_mask_for_col(int col) {
    return 1ULL << (col * PHEIGHT);
}

// Counts the number of set bits in a bitmask (population count).
static unsigned int popcount(uint64_t m) {
    // Use a fast compiler intrinsic if available.
    #if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(m);
    #else
    unsigned int c = 0;
    for (c = 0; m; c++) {
        m &= m - 1; // Brian Kernighan's algorithm.
    }
    return c;
    #endif
}

// Computes a bitmask of all positions where the given player can win on the next move.
static uint64_t compute_winning_position(uint64_t position, uint64_t mask) {
    // Vertical check
    uint64_t r = (position << 1) & (position << 2) & (position << 3);
    
    // Horizontal check
    uint64_t p = (position << PHEIGHT) & (position << (2 * PHEIGHT));
    r |= p & (position << (3 * PHEIGHT));
    r |= p & (position >> PHEIGHT);
    p = (position >> PHEIGHT) & (position >> (2 * PHEIGHT));
    r |= p & (position << PHEIGHT);
    r |= p & (position >> (3 * PHEIGHT));
    
    // Diagonal (y = -x) check
    p = (position << (PHEIGHT - 1)) & (position << (2 * (PHEIGHT - 1)));
    r |= p & (position << (3 * (PHEIGHT - 1)));
    r |= p & (position >> (PHEIGHT - 1));
    p = (position >> (PHEIGHT - 1)) & (position >> (2 * (PHEIGHT - 1)));
    r |= p & (position << (PHEIGHT - 1));
    r |= p & (position >> (3 * (PHEIGHT - 1)));
    
    // Diagonal (y = x) check
    p = (position << (PHEIGHT + 1)) & (position << (2 * (PHEIGHT + 1)));
    r |= p & (position << (3 * (PHEIGHT + 1)));
    r |= p & (position >> (PHEIGHT + 1));
    p = (position >> (PHEIGHT + 1)) & (position >> (2 * (PHEIGHT + 1)));
    r |= p & (position << (PHEIGHT + 1));
    r |= p & (position >> (3 * (PHEIGHT + 1)));
    
    return r & (BOARD_MASK ^ mask); // Exclude spots that are already occupied.
}

// Computes the winning positions for the opponent.
static uint64_t opponent_winning_position(const GameState* state) {
    return compute_winning_position(state->current_position ^ state->mask, state->mask);
}

// Computes the winning positions for the current player.
static uint64_t winning_position(const GameState* state) {
    return compute_winning_position(state->current_position, state->mask);
}

// Gets the index of the least significant set bit (LSB).
static inline int count_trailing_zeros(uint64_t n) {
    // Use a fast compiler intrinsic if available.
    #if defined(__GNUC__) || defined(__clang__)
    return __builtin_ctzll(n);
    #else
    int count = 0;
    while ((n & 1) == 0) {
        n >>= 1;
        count++;
    }
    return count;
    #endif
}

// Initializes a GameState struct to an empty board.
void init_gamestate(GameState* state) {
    assert(state != NULL);
    state->current_position = 0;
    state->mask = 0;
    state->moves = 0;
}

// Updates the game state by playing a move in the specified column.
void play_move(GameState* state, int col) {
    assert(state != NULL);
    assert(col >= 0 && col < WIDTH);
    assert(can_play(state, col));
    
    uint64_t move = (state->mask + bottom_mask_for_col(col)) & column_mask(col);
    state->current_position ^= state->mask; // Pass turn to opponent.
    state->mask |= move;                    // Add the new stone to the board.
    state->moves++;
}

// Checks if a move can be legally played in a given column.
bool can_play(const GameState* state, int col) {
    assert(state != NULL);
    assert(col >= 0 && col < WIDTH);
    return (state->mask & top_mask_for_col(col)) == 0;
}

// Checks if playing a move in a column results in a win for the current player.
bool is_winning_move(const GameState* state, int col) {
    assert(state != NULL);
    assert(col >= 0 && col < WIDTH);
    
    // Temporarily play the move to check for a win.
    const uint64_t pos = state->current_position | ((state->mask + bottom_mask_for_col(col)) & column_mask(col));
    
    // Horizontal check
    uint64_t horizontal_win = pos & (pos >> PHEIGHT);
    if ((horizontal_win & (horizontal_win >> (2 * PHEIGHT))) != 0) return true;
    
    // Vertical check
    uint64_t vertical_win = pos & (pos >> 1);
    if ((vertical_win & (vertical_win >> 2)) != 0) return true;
    
    // Diagonal (y = x) check
    uint64_t diag1_win = pos & (pos >> (PHEIGHT + 1));
    if ((diag1_win & (diag1_win >> (2 * (PHEIGHT + 1)))) != 0) return true;
    
    // Diagonal (y = -x) check
    uint64_t diag2_win = pos & (pos >> (PHEIGHT - 1));
    if ((diag2_win & (diag2_win >> (2 * (PHEIGHT - 1)))) != 0) return true;
    
    return false;
}

// Checks if the current player can win on their next move.
bool can_win_next(const GameState* state) {
    assert(state != NULL);
    return (winning_position(state) & possible(state)) != 0;
}

// Checks if the game is a draw (board is full).
bool is_draw(const GameState* state) {
    assert(state != NULL);
    return state->moves >= WIDTH * HEIGHT;
}

// Returns a bitmask of moves that do not result in an immediate loss.
uint64_t possible_non_losing_moves(const GameState* state) {
    assert(state != NULL);
    assert(!can_win_next(state));
    uint64_t possible_mask = possible(state);
    uint64_t opponent_win = opponent_winning_position(state);
    uint64_t forced_moves = possible_mask & opponent_win;
    if (forced_moves) {
        if (forced_moves & (forced_moves - 1)) { // Opponent has more than one threat.
            return 0; // Loss is unavoidable.
        }
        possible_mask = forced_moves; // Must play the single blocking move.
    }
    // Avoid playing directly below an opponent's winning spot.
    return possible_mask & ~(opponent_win >> 1);
}

// Heuristically scores a move based on how many new winning threats it creates.
int move_score(const GameState* state, uint64_t move) {
    assert(state != NULL);
    uint64_t new_pos = state->current_position | move;
    uint64_t new_mask = state->mask | move;
    return popcount(compute_winning_position(new_pos, new_mask));
}

// Converts a bitboard move (a single set bit) to a column index.
int bitboard_to_col(uint64_t move) {
    return count_trailing_zeros(move) / PHEIGHT;
}

// Returns a bitmask of all possible moves.
uint64_t possible(const GameState* state) {
    return (state->mask + BOTTOM_MASK) & BOARD_MASK;
}