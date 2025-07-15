#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include "bitboard.h"

#define TABLE_ENTRIES ((1 << 23) + 9)

// Separates upper-bound values from lower-bound values in the table.
#define BOUND_SEPARATOR (MAX_SCORE - MIN_SCORE + 1)

// Encodes a score as a lower-bound table value.
#define VAL_LOWER_BOUND(score) ((uint8_t)((score) + MAX_SCORE - 2 * MIN_SCORE + 2))

// Encodes a score as an upper-bound table value.
#define VAL_UPPER_BOUND(score) ((uint8_t)((score) - MIN_SCORE + 1))

// Decodes a lower-bound table value back to a score.
#define DECODE_LOWER_BOUND(val) ((int)(val) + 2 * MIN_SCORE - MAX_SCORE - 2)

// Decodes an upper-bound table value back to a score.
#define DECODE_UPPER_BOUND(val) ((int)(val) + MIN_SCORE - 1)

extern uint32_t keys[TABLE_ENTRIES];
extern uint8_t vals[TABLE_ENTRIES];

void reset_table(void);

static inline uint64_t get_key(GameState* state) {
    return state->current_player + state->filled;
}

static inline void put_table(uint64_t key, uint8_t val) {
    assert(key < (1ULL << (WIDTH * PHEIGHT)));
    assert(val != 0); // 0 is reserved for empty entries

    size_t i = key % TABLE_ENTRIES;
    keys[i] = (uint32_t)key;
    vals[i] = val;
}

static inline uint8_t get_table(uint64_t key) {
    size_t i = key % TABLE_ENTRIES;
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
