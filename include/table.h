#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>

/**
 * @brief Allocates memory for the transposition table. Must be called once at startup.
 */
void init_table(void);

/**
 * @brief Clears all entries in the transposition table.
 */
void reset_table(void);

/**
 * @brief Frees the memory used by the transposition table. Must be called once at exit.
 */
void free_table(void);

/**
 * @brief Stores a value for a given key in the table.
 * @param key The 64-bit position key.
 * @param value The encoded score value. A value of 0 is reserved for "not found" and should not be stored.
 */
void table_put(uint64_t key, uint8_t value);

/**
 * @brief Retrieves a value for a given key from the table.
 * @param key The 64-bit position key.
 * @return The stored value, or 0 if the key is not found.
 */
uint8_t table_get(uint64_t key);

#endif // TABLE_H
