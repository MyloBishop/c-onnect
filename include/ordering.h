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

        while (j >= 0 && arr[j].score > key.score) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

int sort_moves(const GameState* state, int* arr) {
    ScoredMove scored_moves[WIDTH];
    int num_moves = 0;

    const int base_order[7] = {3,2,4,1,5,0,6};

    for (int i = 0; i < WIDTH; i++) {
        int col = base_order[i];
        if (can_play(state, col)) {
            scored_moves[num_moves].move = col;
            scored_moves[num_moves].score = move_score(state, col);
            num_moves++;
        }
    }

    insertion_sort_scored_move(scored_moves, num_moves);

    for (int i = 0; i < num_moves; i++) {
        arr[i] = scored_moves[i].move;
    }
    
    return num_moves;
}
