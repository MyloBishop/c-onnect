#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include "bitboard.h"
#include "table.h"

Entry table[TABLE_ENTRIES];

uint64_t get_key(GameState* state) {
    return state->current_player + state->filled;
}

void reset_table(void) {
    memset(&table, 0, TABLE_SIZE_BYTES); // 0 means missing data
}

size_t table_index(uint64_t key) {
    return key % TABLE_ENTRIES;
}

// we shift our evaluation values so that 0 can
// represent a non-present entry instead of a draw
uint8_t map_val(int val) {
    return (uint8_t)val - MIN_SCORE + 1;
}

int unmap_val(uint8_t val) {
    return (int)val + MIN_SCORE - 1;
}

void put_table(uint64_t key, uint8_t val) {
    assert(key < (1ULL << 56));

    size_t i = table_index(key);
    table[i].key = key;
    table[i].val = val;
}

uint8_t get_table(u_int64_t key) {
    Entry entry = table[table_index(key)];

    if (entry.val == 0) {
        return 0;
    }

    if (entry.key != key) { 
        return 0;
    }

    return entry.val;
}
