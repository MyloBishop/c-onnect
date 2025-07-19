#include "ordering.h"
#include <assert.h>
#include <stdlib.h>

void sorter_init(MoveSorter* sorter) {
    assert(sorter != NULL);
    sorter->size = 0;
}

void sorter_add(MoveSorter* sorter, uint64_t move, int score) {
    assert(sorter != NULL);
    assert(sorter->size < WIDTH);
    
    int pos = sorter->size++;
    // Insertion sort: elements are shifted to make room for the new move
    for (; pos > 0 && sorter->entries[pos - 1].score > score; --pos) {
        sorter->entries[pos] = sorter->entries[pos - 1];
    }
    sorter->entries[pos].move = move;
    sorter->entries[pos].score = score;
}

uint64_t sorter_get_next(MoveSorter* sorter) {
    assert(sorter != NULL);
    if (sorter->size > 0) {
        // Returns the entry with the highest score, which is at the end
        return sorter->entries[--sorter->size].move;
    }
    return 0; // No moves left
}
