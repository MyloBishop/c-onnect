# Connect Four Solver

A high-performance Connect Four solver written in C. This project uses a bitboard representation, a negamax search algorithm with alpha-beta pruning, a transposition table, and an opening book to solve game positions quickly and accurately.

It includes a playable game executable (`game`), a powerful standalone solver (`solver`), and a comprehensive benchmark suite.

## Features

-   **Bitboard Representation**: A fast and memory-efficient way to represent the game state and check for wins.
-   **Negamax Search**: A highly optimized search algorithm with alpha-beta pruning to reduce the search space.
-   **Transposition Table**: Caches previously computed game states to avoid redundant calculations.
-   **Move Ordering**: Heuristically orders moves to maximize the effectiveness of alpha-beta pruning.
-   **Opening Book**: Provides optimal moves for the first few turns of the game, loaded from `book.bin`.
-   **Dual Executables**: Comes with a playable game (`game`) and a command-line solver (`solver`).

---
## Building the Project

You will need `gcc`, `make`, and `python3` installed.

-   **Build for Debugging**:
    `make` or `make all`
    This compiles the `game` and `solver` executables with debug symbols.

-   **Build for Release**:
    `make release`
    This cleans the project and compiles highly optimized executables in the `bin/` directory. This is recommended for benchmarking and general use.

-   **Generate the Opening Book**:
    `make book`
    This first builds the release version of the solver, then runs the `generate_book.py` script to create the `book.bin` file. The book contains optimal moves for the first 7 plies by default.

-   **Run Benchmarks**:
    `make bench`
    This builds the release version of the solver and then runs the benchmark suite to verify correctness and measure performance.

-   **Clean the Project**:
    `make clean`
    This removes all compiled binaries and object files.

---
## Usage

All executables are placed in the `bin/` directory.

### Playable Game

Run the interactive game from the command line. You can specify whether each player is human or AI.

`./bin/game [player1_type] [player2_type]`

-   `player_type` can be `human` or `ai`.
-   If no arguments are provided, it defaults to `human` vs `ai`.

Example:
```
# Start a game with a human player (O) against the AI (X)
./bin/game human ai
```

```
# Start a game where the AI plays against itself
./bin/game ai ai
```

### Standalone Solver

The solver takes a single argument: a string of moves representing a game position. The moves are 1-indexed columns (1-7).

`./bin/solver <move_string>`

Example:
```
# Solve the position after the moves 4, 4, 5, 3
./bin/solver 4453
```

#### Understanding the Solver Output

The solver outputs a single line containing the position's bitboards, its score, the number of nodes searched, and the time taken in microseconds.

##### Bitboards Explained
In this solver, **bitboards** are a highly efficient method for representing the game board using numbers. Instead of using a traditional 2D array, the state of the board is stored in two **64-bit unsigned integers** (`uint64_t`). Each bit within these integers corresponds to a specific square on the 7x6 Connect Four grid.

The solver uses two separate bitboards to capture the entire game state, as defined in the `GameState` structure:

* **`current_position`**: This is the first number in the output. It's a bitmask where a '1' bit represents a piece belonging to the **player whose turn it is**.
* **`mask`**: This is the second number in the output. It's a bitmask where a '1' bit represents **any occupied square**, regardless of which player's piece is there.

The primary advantages of using bitboards are **speed and efficiency**. Complex operations like checking for a four-in-a-row can be performed with a few simple bitwise operations, which is significantly faster than iterating through an array.

##### The Score Explained
The score reflects the outcome of the game assuming both players play perfectly. The goal is to win as soon as possible or lose as late as possible.

* **A positive score** means the current player has a winning strategy. The score indicates how quickly they can win. A higher score means a faster victory. The score can be calculated as `22 - (number of moves played to win)`.
* **A score of 0** means the game will end in a draw if both players make optimal moves.
* **A negative score** means the current player will lose, even with perfect play. The score indicates how long they can delay the loss. A score closer to zero (e.g., -1) means a longer-lasting game. The score can be calculated as `(number of moves played by the winner) - 22`.

### Benchmarking

The easiest way to run the benchmark suite is with the Makefile command. It will automatically build the optimized solver first.

`make bench`

This runs [Pascal Pons' benchmarking suite](http://blog.gamesolver.org/solving-connect-four/02-test-protocol), [compare the results](https://github.com/PascalPons/connect4)!

---
## Core API Components

The project is modular, with functionality separated into several key components defined in the `include/` and `src/` directories.

-   `bitboard`: Manages the `GameState` struct. It handles the board representation, move execution, and win detection.
-   `engine`: Contains the core solving logic, including the `negamax` search function and the public `solve` and `find_best_move` functions.
-   `table`: Implements the transposition table, a hash map used to store the scores of previously evaluated positions.
-   `book`: Handles loading and querying the opening book from `book.bin`.
-   `ordering`: Implements a move sorter that is used by the engine to prioritize promising moves, which significantly improves alpha-beta pruning efficiency.
-   `game`: Contains the main loop and logic for the interactive playable game.
-   `solver`: A lightweight wrapper that parses a command-line position and calls the engine to solve it.
