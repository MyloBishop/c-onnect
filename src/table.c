#include "table.h"
#include "bitboard.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <stdio.h>

// A hint to the compiler that a condition is likely to be true, for optimization.
#define LIKELY(x) __builtin_expect(!!(x), 1)

// Log2 of the table size.
#define LOG_SIZE 23
// Number of bits in the board key.
#define KEY_SIZE (WIDTH * PHEIGHT)
// Number of bits needed for the encoded score value.
#define VALUE_SIZE 7

// The type for the truncated key stored in the table.
typedef uint32_t board_key_t;
// The type for the encoded score value.
typedef uint8_t board_value_t;

// Assert that board_key_t can hold the truncated key.
_Static_assert(sizeof(board_key_t) * CHAR_BIT >= (KEY_SIZE - LOG_SIZE),
               "board_key_t type is not large enough for the configured key size.");
// Assert that board_value_t can hold the encoded score.
_Static_assert(sizeof(board_value_t) * CHAR_BIT >= VALUE_SIZE,
               "board_value_t type is not large enough for the configured value size.");


static board_key_t* K_table;
static board_value_t* V_table;
static size_t table_size;

// Checks if a number is prime using an optimized trial division.
static bool is_prime(uint64_t n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;

    for (uint64_t i = 5; i * i <= n; i = i + 6) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return false;
        }
    }
    return true;
}

// Finds the smallest prime number greater than or equal to n.
static uint64_t find_next_prime(uint64_t n) {
    if (n % 2 == 0) n++; // Start with an odd number.
    while (true) {
        if (is_prime(n)) return n;
        n += 2; // Check next odd number.
    }
}

// Computes the table index for a given key.
static inline size_t get_index(uint64_t key) {
    return key % table_size;
}

// Initializes the transposition table.
void init_table(void) {
    // Using a prime size helps reduce collisions.
    table_size = find_next_prime(1ULL << LOG_SIZE);

#if defined(__GNUC__) || defined(__clang__)
    // Use posix_memalign for cache-aligned memory to improve performance.
    const size_t alignment = 64;
    if (posix_memalign((void**)&K_table, alignment, table_size * sizeof(board_key_t)) != 0) {
        fprintf(stderr, "Error: posix_memalign for K_table failed.\n");
        abort();
    }
    if (posix_memalign((void**)&V_table, alignment, table_size * sizeof(board_value_t)) != 0) {
        fprintf(stderr, "Error: posix_memalign for V_table failed.\n");
        free(K_table);
        abort();
    }
#else
    // Fall back to standard malloc for other compilers.
    K_table = (board_key_t*)malloc(table_size * sizeof(board_key_t));
    if (K_table == NULL) {
        fprintf(stderr, "Error: malloc for K_table failed.\n");
        abort();
    }
    V_table = (board_value_t*)malloc(table_size * sizeof(board_value_t));
    if (V_table == NULL) {
        fprintf(stderr, "Error: malloc for V_table failed.\n");
        free(K_table);
        abort();
    }
#endif
    reset_table();
}

// Clears all entries in the transposition table.
void reset_table(void) {
    assert(K_table != NULL && V_table != NULL && table_size > 0);
    memset(K_table, 0, table_size * sizeof(board_key_t));
    memset(V_table, 0, table_size * sizeof(board_value_t));
}

// Frees the memory used by the transposition table.
void free_table(void) {
    free(K_table);
    free(V_table);
    K_table = NULL;
    V_table = NULL;
    table_size = 0;
}

// Stores a key-value pair in the table, overwriting any existing entry at the index.
void table_put(uint64_t key, board_value_t value) {
    assert(key >> KEY_SIZE == 0);
    assert(value != 0); // 0 is reserved for "not found".

    size_t pos = get_index(key);
    K_table[pos] = (board_key_t)key; // Store truncated key to detect some collisions.
    V_table[pos] = value;
}

// Retrieves a value from the table for a given key.
board_value_t table_get(uint64_t key) {
    assert(key >> KEY_SIZE == 0);

    size_t pos = get_index(key);
    // Check if the stored truncated key matches the current key.
    if (LIKELY(K_table[pos] == (board_key_t)key)) {
        return V_table[pos];
    }
    return 0; // Return 0 if not found or if a collision occurred.
}