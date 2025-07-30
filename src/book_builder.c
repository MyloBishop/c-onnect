#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bitboard.h"
#include "engine.h"
#include "table.h"

#define DEFAULT_BOOK_DEPTH 1
#define BOOK_FILENAME "book.bin"

typedef struct {
    uint64_t* data;
    size_t count;
    size_t capacity;
} Book;

// Adds a new entry to the dynamic book array, resizing if necessary.
void book_add(Book* book, uint64_t entry) {
    if (book->count >= book->capacity) {
        book->capacity = (book->capacity == 0) ? 1024 : book->capacity * 2;
        book->data = (uint64_t*)realloc(book->data, book->capacity * sizeof(uint64_t));
        if (!book->data) {
            perror("Failed to reallocate memory for book");
            exit(EXIT_FAILURE);
        }
    }
    book->data[book->count++] = entry;
}

// Recursively traverses the game tree to find and store the best move for each position.
void build_book_recursive(GameState* state, int depth, Book* book) {
    if (depth < 0 || state->moves >= WIDTH * HEIGHT) {
        return;
    }

    int best_move_col = find_best_move(state);

    if (best_move_col != -1) {
        uint64_t key = get_key(state);
        // Pack the key and move into a single 64-bit integer.
        uint64_t book_entry = (key << 4) | (uint64_t)best_move_col;
        book_add(book, book_entry);
    }
    
    for (int col = 0; col < WIDTH; ++col) {
        if (can_play(state, col)) {
            // Prune branches that lead to an immediate win.
            if (is_winning_move(state, col)) continue;
            
            GameState next_state = *state;
            play_move(&next_state, col);
            build_book_recursive(&next_state, depth - 1, book);
        }
    }
}

int main(int argc, char* argv[]) {
    int max_depth = DEFAULT_BOOK_DEPTH;
    if (argc > 1) {
        max_depth = atoi(argv[1]);
        if (max_depth <= 0) {
            fprintf(stderr, "Invalid depth. Using default: %d\n", DEFAULT_BOOK_DEPTH);
            max_depth = DEFAULT_BOOK_DEPTH;
        }
    }

    printf("Initializing solver and transposition table...\n");
    init_solver();
    init_table();

    Book book = { .data = NULL, .count = 0, .capacity = 0 };
    GameState initial_state;
    init_gamestate(&initial_state);

    printf("Building opening book up to depth %d.\n", max_depth);
    clock_t start_time = clock();

    build_book_recursive(&initial_state, max_depth, &book);
    
    for (int col = 0; col < WIDTH; ++col) {
        int bar_width = 20;
        int completed = (int)(((float)(col + 1) / WIDTH) * bar_width);
        printf("\rProgress: [");
        for(int i = 0; i < bar_width; ++i) {
            if(i < completed) printf("#");
            else printf("-");
        }
        printf("] %d/%d", col + 1, WIDTH);
        fflush(stdout); // Force the line to print now.

        GameState next_state = initial_state;
        play_move(&next_state, col);
        build_book_recursive(&next_state, max_depth - 1, &book);
    }
    printf("\n"); // Newline after the progress bar is complete.

    clock_t end_time = clock();
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("Analysis complete in %.2f seconds.\n", time_spent);
    printf("Generated %zu book entries. Writing to %s...\n", book.count, BOOK_FILENAME);

    FILE* f_out = fopen(BOOK_FILENAME, "wb");
    if (!f_out) {
        perror("Failed to open book file for writing");
        exit(EXIT_FAILURE);
    }

    fwrite(book.data, sizeof(uint64_t), book.count, f_out);

    fclose(f_out);
    free(book.data);
    free_table();

    printf("Opening book created successfully.\n");

    return 0;
}