#ifndef PLAYER_H
#define PLAYER_H

#include "bitboard.h"
#include "engine.h"

// Define the types of players available
typedef enum {
    PLAYER_TYPE_HUMAN,
    PLAYER_TYPE_AI
} PlayerType;

// Represents a player, holding its type and the current turn symbol
typedef struct {
    PlayerType type;
    char symbol; 
} Player;

/**
 * @brief Gets a move from the specified player.
 * * @param player A pointer to the player whose turn it is.
 * @param game The current game state.
 * @return The column (0-indexed) of the chosen move.
 */
int get_player_move(const Player* player, const GameState* game);

#endif // PLAYER_H