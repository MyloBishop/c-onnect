#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// --- Configuration ---
// Board dimensions
#define WIDTH 7
#define HEIGHT 6

// --- Derived Constants ---
#define PHEIGHT (HEIGHT + 1) // Padded height for bitboard calculations

// Compile-time check for bitboard size
#if (WIDTH * PHEIGHT > 64)
#error "Board dimensions are too large for a 64-bit integer."
#endif

// Score bounds for the solver
#define MIN_SCORE (-(WIDTH * HEIGHT) / 2 + 3)
#define MAX_SCORE (((WIDTH * HEIGHT + 1) / 2) - 3)

// --- Data Structures ---

// Represents the board state from the perspective of the current player.
typedef struct {
    uint64_t current_position; // Bitmask of the current player's pieces
    uint64_t mask;             // Bitmask of all occupied slots
    int moves;                 // Number of moves played in the game
} GameState;


// --- Public API ---

/**
 * @brief Initializes an empty game state.
 * @param state Pointer to the GameState object.
 */
void init_gamestate(GameState* state);

/**
 * @brief Plays a move in the given column for the current player.
 * Switches the perspective to the next player.
 * @param state Pointer to the GameState object.
 * @param col The 0-indexed column to play in.
 */
void play_move(GameState* state, int col);

/**
 * @brief Checks if a move can be made in the given column.
 * @param state Pointer to the GameState object.
 * @param col The 0-indexed column to check.
 * @return True if the column is not full, false otherwise.
 */
bool can_play(const GameState* state, int col);

/**
 * @brief Checks if playing in the given column is a winning move for the current player.
 * @param state Pointer to the GameState object.
 * @param col The 0-indexed column.
 * @return True if the move wins the game, false otherwise.
 */
bool is_winning_move(const GameState* state, int col);

/**
 * @brief Checks if the current player has any move that wins immediately.
 * @param state Pointer to the GameState object.
 * @return True if a winning move exists, false otherwise.
 */
bool can_win_next(const GameState* state);

/**
 * @brief Checks if the current game state is a draw.
 * @param state Pointer to the GameState object.
 * @return True if the state is a draw, false otherwise.
 */
bool is_draw(const GameState* state);

/**
 * @brief Computes a bitmask of all moves that do not result in an immediate loss.
 * An immediate loss is a position where the opponent can win on their next turn.
 * @param state Pointer to the GameState object.
 * @return A bitmask of non-losing moves.
 */
uint64_t possible_non_losing_moves(const GameState* state);

/**
 * @brief Calculates a heuristic score for a move.
 * The score is the number of new four-in-a-row threats created.
 * @param state Pointer to the GameState object.
 * @param move A bitmask representing the move to score.
 * @return The integer score of the move.
 */
int move_score(const GameState* state, uint64_t move);

/**
 * @brief Converts a mask representation of a move to the column index.
 * @param move A bitmask representing the move.
 * @return The integer that the stone would be dropped in for that move.
 */
int bitboard_to_col(uint64_t move);

/**
 * @brief Generates a unique 64-bit key for the current board position.
 * This key is used for the transposition table.
 * @param state Pointer to the GameState object.
 * @return The unique key.
 */
static inline uint64_t get_key(const GameState* state) {
    return state->current_position + state->mask;
}

/**
 * @brief Creates a bitmask for all cells in a given column.
 * @param col The 0-indexed column.
 * @return A bitmask for that column.
 */
static inline uint64_t column_mask(int col) {
    return ((1ULL << HEIGHT) - 1) << (col * PHEIGHT);
}

/**
 * @brief Generates a mask of the playable positions on the board.
 * @param state Pointer to the GameState object.
 * @return A bitmask of the possible positions.
 */
uint64_t possible(const GameState* state);

#endif // BITBOARD_H
