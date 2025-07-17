#include "bitboard.h"

typedef struct {
    int move;
    int score;
} ScoredMove;

void insertion_sort_scored_move(ScoredMove* arr, int n)
{
    int i, j;
    ScoredMove key;

    for (i = 1; i < n; ++i) {
        key = arr[i];
        j = i - 1;

        while (j >= 0 && arr[j].score < key.score) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

int sort_moves(const GameState* state, int* arr, uint64_t moves_mask) {
    ScoredMove scored_moves[WIDTH];
    int num_moves = 0;

    const int base_order[WIDTH] = {3,4,2,5,1,6,0};

    for (int i = 0; i < WIDTH; i++) {
        int col = base_order[i];
        if ((moves_mask & column_mask(col))) {
            scored_moves[num_moves].move = col;
            scored_moves[num_moves].score = move_score(state, col);
            num_moves++;
        }
    }

    insertion_sort_scored_move(scored_moves, num_moves);

    for (int i = 0; i < num_moves; i++) {
        arr[i] = scored_moves[i].move;
    }
    
    assert((unsigned int)num_moves == popcount(moves_mask));

    return num_moves;
}
