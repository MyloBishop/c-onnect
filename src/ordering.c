#include "ordering.h"
#include <assert.h>
#include <stdlib.h>

// Initializes a move sorter.
void sorter_init(MoveSorter* sorter) {
    assert(sorter != NULL);
    sorter->size = 0;
}

// Adds a move and its score to the sorter, maintaining sorted order.
void sorter_add(MoveSorter* sorter, uint64_t move, int score) {
    assert(sorter != NULL);
    assert(sorter->size < WIDTH);
    
    int pos = sorter->size++;
    // Use insertion sort to place the new move based on its score.
    for (; pos > 0 && sorter->entries[pos - 1].score > score; --pos) {
        sorter->entries[pos] = sorter->entries[pos - 1];
    }
    sorter->entries[pos].move = move;
    sorter->entries[pos].score = score;
}

// Retrieves the move with the highest score from the sorter.
uint64_t sorter_get_next(MoveSorter* sorter) {
    assert(sorter != NULL);
    if (sorter->size > 0) {
        // Highest-scored moves are at the end of the array.
        return sorter->entries[--sorter->size].move;
    }
    return 0; // Indicates no moves are left.
}