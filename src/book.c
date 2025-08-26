#include "book.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// --- Book Data Structure ---

typedef struct {
    uint128_t key;
    uint8_t move;
} __attribute__((packed)) BookEntry;

static BookEntry* g_book_entries = NULL;
static size_t g_book_size = 0;

// --- Public API Implementations ---

void init_book(void) {
    const char* book_filename = "book.bin";
    FILE* file = fopen(book_filename, "rb");
    if (!file) {
        fprintf(stderr, "Info: Opening book '%s' not found. Continuing without it.\n", book_filename);
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size == 0) {
        fclose(file);
        return;
    }
    
    assert(file_size % sizeof(BookEntry) == 0);

    g_book_size = file_size / sizeof(BookEntry);
    g_book_entries = (BookEntry*)malloc(file_size);
    if (!g_book_entries) {
        fprintf(stderr, "Error: Failed to allocate memory for the opening book.\n");
        fclose(file);
        g_book_size = 0;
        return;
    }

    size_t items_read = fread(g_book_entries, sizeof(BookEntry), g_book_size, file);
    if (items_read != g_book_size) {
        fprintf(stderr, "Error: Failed to read the opening book file correctly.\n");
        free(g_book_entries);
        g_book_entries = NULL;
        g_book_size = 0;
    } else {
        #ifdef DEBUG
        fprintf(stderr, "DEBUG: Opening book loaded successfully with %zu entries.\n", g_book_size);
        fprintf(stderr, "DEBUG: ---- Verifying first 10 book entries ----\n");
        size_t limit = g_book_size < 10 ? g_book_size : 10;
        for (size_t i = 0; i < limit; ++i) {
            uint128_t key = g_book_entries[i].key;
            uint64_t key_high = (uint64_t)(key >> 64);
            uint64_t key_low = (uint64_t)key;
            fprintf(stderr, "DEBUG: Entry %zu -> Key (Mask/Pos): %-10llu / %-10llu | Move: %u\n", 
                   i, key_high, key_low, g_book_entries[i].move);
        }
        fprintf(stderr, "DEBUG: ----------------------------------------\n");
        #endif
    }

    fclose(file);
}

void free_book(void) {
    if (g_book_entries) {
        free(g_book_entries);
        g_book_entries = NULL;
        g_book_size = 0;
    }
}

uint128_t book_compute_key(const GameState* state) {
    return ((uint128_t)state->mask << 64) | state->current_position;
}

bool book_get_move(uint128_t key, int* move) {
    assert(move != NULL);

    if (!g_book_entries || g_book_size == 0) {
        return false;
    }
    
    // Binary search for the key in the sorted book entries.
    int low = 0;
    int high = g_book_size - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        uint128_t mid_key = g_book_entries[mid].key;

        if (mid_key < key) {
            low = mid + 1;
        } else if (mid_key > key) {
            high = mid - 1;
        } else {
            *move = g_book_entries[mid].move;
            #ifdef DEBUG
            fprintf(stderr, "DEBUG: Book hit! Found move: %d\n", *move);
            #endif
            return true;
        }
    }
    
    #ifdef DEBUG
    fprintf(stderr, "DEBUG: Book miss.\n");
    #endif

    return false;
}