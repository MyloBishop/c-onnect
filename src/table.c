#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include "bitboard.h"
#include "table.h"

uint32_t keys[TABLE_ENTRIES];
uint8_t vals[TABLE_ENTRIES];

uint64_t get_key(GameState* state) {
    return state->current_player + state->filled;
}

void reset_table(void) {
    // 0 means missing data
    memset(&keys, 0, TABLE_ENTRIES * sizeof(keys[0]));
    memset(&vals, 0, TABLE_ENTRIES * sizeof(vals[0]));
}

size_t table_index(uint64_t key) {
    return key % TABLE_ENTRIES;
}

// we shift our evaluation values so that 0 can
// represent a non-present entry instead of a draw
uint8_t map_val(int val) {
    assert(1 - MIN_SCORE + MAX_SCORE < 256);
    return (uint8_t)val - MIN_SCORE + 1;
}

int unmap_val(uint8_t val) {
    assert(1 - MIN_SCORE + MAX_SCORE < 256);
    return (int)val + MIN_SCORE - 1;
}

void put_table(uint64_t key, uint8_t val) {
    assert(key < (1ULL << 49));
    assert(val != 0); // 0 is reserved for empty entries

    size_t i = table_index(key);
    keys[i] = (uint32_t)key;
    vals[i] = val;
}

uint8_t get_table(uint64_t key) {
    size_t i = table_index(key);
    uint32_t table_key = keys[i];
    uint32_t table_val = vals[i];

    if (table_val == 0) {
        return 0;
    }

    if (table_key != (uint32_t)key) { 
        return 0;
    }

    return table_val;
}
