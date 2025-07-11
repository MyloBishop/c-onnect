#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>
#include <stddef.h>
#include "bitboard.h"

#define TABLE_ENTRIES 8388593 // closest prime to 64MB table

typedef struct {
    uint64_t key: 56;
    uint8_t val;
} Entry;

extern Entry table[TABLE_ENTRIES];

uint64_t get_key(GameState* state);

void reset_table(void);

size_t table_index(uint64_t key);

uint8_t map_val(int val);

int unmap_val(uint8_t val);

void put_table(uint64_t key, uint8_t val);

uint8_t get_table(uint64_t key);

#endif
