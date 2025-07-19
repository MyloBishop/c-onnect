#include "bitboard.h"
#include <string.h>

// --- Pre-computed Masks ---

// A mask representing the bottom row of the entire board.
static const uint64_t BOTTOM_MASK = ((1ULL << (WIDTH * PHEIGHT)) - 1) / ((1ULL << PHEIGHT) - 1);

// A mask representing all playable squares on the board.
static const uint64_t BOARD_MASK = BOTTOM_MASK * ((1ULL << HEIGHT) - 1);

// --- Static Helper Functions ---

static uint64_t top_mask_for_col(int col) {
    return 1ULL << ((HEIGHT - 1) + col * PHEIGHT);
}

static uint64_t bottom_mask_for_col(int col) {
    return 1ULL << (col * PHEIGHT);
}

static uint64_t possible(const GameState* state) {
    return (state->mask + BOTTOM_MASK) & BOARD_MASK;
}

static unsigned int popcount(uint64_t m) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(m);
#else
    unsigned int c = 0;
    for (c = 0; m; c++) {
        m &= m - 1;
    }
    return c;
#endif
}

static uint64_t compute_winning_position(uint64_t position, uint64_t mask) {
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

static uint64_t opponent_winning_position(const GameState* state) {
    return compute_winning_position(state->current_position ^ state->mask, state->mask);
}

static uint64_t winning_position(const GameState* state) {
    return compute_winning_position(state->current_position, state->mask);
}

// Get the index of the least significant set bit
static inline int count_trailing_zeros(uint64_t n) {
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

// --- Public API Implementations ---

void init_gamestate(GameState* state) {
    assert(state != NULL);
    state->current_position = 0;
    state->mask = 0;
    state->moves = 0;
}

void play_move(GameState* state, int col) {
    assert(state != NULL);
    assert(col >= 0 && col < WIDTH);
    assert(can_play(state, col));

    uint64_t move = (state->mask + bottom_mask_for_col(col)) & column_mask(col);
    state->current_position ^= state->mask; // Switch current player's stones to opponent
    state->mask |= move;                    // Add the new stone to the mask
    state->moves++;
}

bool can_play(const GameState* state, int col) {
    assert(state != NULL);
    assert(col >= 0 && col < WIDTH);
    return (state->mask & top_mask_for_col(col)) == 0;
}

bool is_winning_move(const GameState* state, int col) {
    assert(state != NULL);
    assert(col >= 0 && col < WIDTH);
    
    // The new piece is placed on the board for checking
    const uint64_t pos = state->current_position | ((state->mask + bottom_mask_for_col(col)) & column_mask(col));

    // Check all four directions for a four-in-a-row.
    // Each check looks for a pattern of 4 adjacent bits.
    
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

bool can_win_next(const GameState* state) {
    assert(state != NULL);
    return (winning_position(state) & possible(state)) != 0;
}

bool is_draw(const GameState* state) {
    assert(state != NULL);
    return state->moves >= WIDTH * HEIGHT;
}

uint64_t possible_non_losing_moves(const GameState* state) {
    assert(state != NULL);
    assert(!can_win_next(state));
    uint64_t possible_mask = possible(state);
    uint64_t opponent_win = opponent_winning_position(state);
    uint64_t forced_moves = possible_mask & opponent_win;
    if (forced_moves) {
        if (forced_moves & (forced_moves - 1)) { // Opponent has more than one threat
            return 0; // Loss is unavoidable
        }
        possible_mask = forced_moves; // Must play the single blocking move
    }
    return possible_mask & ~(opponent_win >> 1); // Avoid playing under an opponent's winning spot
}

int move_score(const GameState* state, uint64_t move) {
    assert(state != NULL);
    uint64_t new_pos = state->current_position | move;
    uint64_t new_mask = state->mask | move; // Assume move is on the mask
    return popcount(compute_winning_position(new_pos, new_mask));
}

int bitboard_to_col(uint64_t move) {
    return count_trailing_zeros(move) / PHEIGHT;
}
