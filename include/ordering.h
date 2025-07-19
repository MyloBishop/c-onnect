#ifndef ORDERING_H
#define ORDERING_H

#include "bitboard.h"
#include <stdint.h>

typedef struct {
    uint64_t move;
    int score;
} MoveEntry;

typedef struct {
    MoveEntry entries[WIDTH];
    int size;
} MoveSorter;

/**
 * @brief Initializes or resets a move sorter to be empty.
 */
void sorter_init(MoveSorter* sorter);

/**
 * @brief Adds a move and its score to the sorter, maintaining sorted order.
 * Uses insertion sort.
 */
void sorter_add(MoveSorter* sorter, uint64_t move, int score);

/**
 * @brief Retrieves the best move (highest score) from the sorter and removes it.
 * @return The bitmask of the best move, or 0 if the sorter is empty.
 */
uint64_t sorter_get_next(MoveSorter* sorter);

#endif // ORDERING_H
