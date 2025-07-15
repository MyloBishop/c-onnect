#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include "bitboard.h"

#define TABLE_ENTRIES ((1 << 23) + 9)
#define UPPER_BOUND_THRESHOLD (MAX_SCORE - MIN_SCORE + 1)
#define LOWER_BOUND_OFFSET (MAX_SCORE - 2 * MIN_SCORE + 2)

extern uint32_t keys[TABLE_ENTRIES];
extern uint8_t vals[TABLE_ENTRIES];

void reset_table(void);

static inline uint64_t get_key(GameState* state) {
    return state->current_player + state->filled;
}

static inline size_t table_index(uint64_t key) {
    return key % TABLE_ENTRIES;
}

// we shift our evaluation values so that 0 can
// represent a non-present entry instead of a draw
static inline uint8_t map_val(int val) {
    assert(val >= MIN_SCORE && val <= MAX_SCORE);
    assert(1 - MIN_SCORE + MAX_SCORE < 256);
    return (uint8_t)val - MIN_SCORE + 1;
}

static inline int unmap_val(uint8_t val) {
    assert(val != 0);
    assert(1 - MIN_SCORE + MAX_SCORE < 256);
    return (int)val + MIN_SCORE - 1;
}

static inline void put_table(uint64_t key, uint8_t val) {
    assert(key < (1ULL << 49));
    assert(val != 0); // 0 is reserved for empty entries

    size_t i = table_index(key);
    keys[i] = (uint32_t)key;
    vals[i] = val;
}

static inline uint8_t get_table(uint64_t key) {
    size_t i = table_index(key);
    uint32_t table_key = keys[i];
    uint8_t table_val = vals[i];

    if (table_val == 0) {
        return 0;
    }

    if (table_key != (uint32_t)key) {
        return 0;
    }

    return table_val;
}

#endif
