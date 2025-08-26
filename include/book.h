#ifndef BOOK_H
#define BOOK_H

#include "bitboard.h"
#include <stdbool.h>

// Use GCC/Clang extension for 128-bit integers.
typedef __uint128_t uint128_t;

// The book will be used for positions with fewer moves than this value.
#define MAX_BOOK_DEPTH 7

/**
 * @brief Initializes the opening book by loading it from a file.
 * Must be called once at startup.
 */
void init_book(void);

/**
 * @brief Frees memory allocated for the opening book.
 */
void free_book(void);

/**
 * @brief Computes the unique 128-bit key for a given game state.
 * The key is a concatenation of the mask and the current player's position.
 * @param state Pointer to the GameState object.
 * @return The unique 128-bit key.
 */
uint128_t book_compute_key(const GameState* state);

/**
 * @brief Retrieves a move from the opening book for a given key.
 * Uses binary search on the loaded book data.
 * @param key The 128-bit position key.
 * @param move A pointer to an integer where the move will be stored.
 * @return True if a move was found for the key, false otherwise.
 */
bool book_get_move(uint128_t key, int* move);

#endif // BOOK_H