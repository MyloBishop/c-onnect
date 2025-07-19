#include "table.h"
#include "bitboard.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <stdio.h>

#define LIKELY(x) __builtin_expect(!!(x), 1)

// --- Configuration ---

// log2 of the table size.
#define LOG_SIZE 23
// Number of bits in the board key.
#define KEY_SIZE (WIDTH * PHEIGHT)
// Number of bits needed for the encoded score value.
#define VALUE_SIZE 7

// --- Type Definitions ---

// We only store a truncated portion of the key to save memory.
typedef uint32_t key_t;

// The encoded value must fit in this type.
typedef uint8_t value_t;

// Assert that key_t can hold the truncated key, which has (KEY_SIZE - LOG_SIZE) bits.
_Static_assert(sizeof(key_t) * CHAR_BIT >= (KEY_SIZE - LOG_SIZE),
               "key_t type is not large enough for the configured key size.");

// Assert that value_t can hold the encoded score, which has VALUE_SIZE bits.
_Static_assert(sizeof(value_t) * CHAR_BIT >= VALUE_SIZE,
               "value_t type is not large enough for the configured value size.");


// --- Static Data ---

static key_t* K_table;
static value_t* V_table;
static size_t table_size; // The size of the hash table, calculated at runtime.

// --- Primality Test Helper Functions ---

/**
 * @brief Checks if n is a prime number using trial division.
 * Optimized to only check odd divisors and factors of the form 6k +/- 1.
 * @param n The number to check.
 * @return True if n is prime, false otherwise.
 */
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

/**
 * @brief Finds the smallest prime number that is greater than or equal to n.
 * @param n The starting number for the search.
 * @return The next prime number.
 */
static uint64_t find_next_prime(uint64_t n) {
    // If n is even, start with the next odd number.
    if (n % 2 == 0) {
        n++;
    }
    // All primes > 2 are odd, so we can skip even numbers.
    while (true) {
        if (is_prime(n)) {
            return n;
        }
        n += 2;
    }
}

// --- Hashing Helper Function ---

static inline size_t get_index(uint64_t key) {
    return key % table_size;
}

// --- Public API Implementations ---

void init_table(void) {
    // Calculate the table size at runtime based on LOG_SIZE
    table_size = find_next_prime(1ULL << LOG_SIZE);

#if defined(__GNUC__) || defined(__clang__)
    // For GCC and Clang, use posix_memalign for cache-aligned memory.
    const size_t alignment = 64;
    if (posix_memalign((void**)&K_table, alignment, table_size * sizeof(key_t)) != 0) {
        fprintf(stderr, "Error: posix_memalign for K_table failed.\n");
        abort(); // Allocation failed
    }
    if (posix_memalign((void**)&V_table, alignment, table_size * sizeof(value_t)) != 0) {
        fprintf(stderr, "Error: posix_memalign for V_table failed.\n");
        free(K_table); // Clean up previously allocated memory
        abort(); // Allocation failed
    }
#else
    // For other compilers, fall back to standard malloc and check for failure.
    K_table = (key_t*)malloc(table_size * sizeof(key_t));
    if (K_table == NULL) {
        fprintf(stderr, "Error: malloc for K_table failed.\n");
        abort();
    }

    V_table = (value_t*)malloc(table_size * sizeof(value_t));
    if (V_table == NULL) {
        fprintf(stderr, "Error: malloc for V_table failed.\n");
        free(K_table); // Clean up the first allocation
        abort();
    }
#endif
    reset_table();
}

void reset_table(void) {
    assert(K_table != NULL && V_table != NULL && table_size > 0);
    memset(K_table, 0, table_size * sizeof(key_t));
    memset(V_table, 0, table_size * sizeof(value_t));
}

void free_table(void) {
    free(K_table);
    free(V_table);
    K_table = NULL;
    V_table = NULL;
    table_size = 0; // Reset size
}

void table_put(uint64_t key, value_t value) {
    assert(key >> KEY_SIZE == 0);
    assert(value != 0); // 0 is reserved for "not found"

    size_t pos = get_index(key);
    K_table[pos] = (key_t)key; // Key is truncated
    V_table[pos] = value;
}

value_t table_get(uint64_t key) {
    assert(key >> KEY_SIZE == 0);

    size_t pos = get_index(key);
    if (LIKELY(K_table[pos] == (key_t)key)) {
        return V_table[pos];
    }
    return 0; // Not found
}
