import os
import subprocess
from multiprocessing import Pool, cpu_count
import struct
from itertools import product
from tqdm import tqdm

# --- Configuration ---
DEPTH = 6
WIDTH = 7
SOLVER_PATH = "./bin/solver"
BOOK_OUTPUT_PATH = "book.bin"
TIMEOUT_SECONDS = 300

def call_solver(move_string):
    try:
        result = subprocess.run(
            [SOLVER_PATH, move_string],
            capture_output=True,
            text=True,
            check=True,
            timeout=TIMEOUT_SECONDS,
        )
        parts = result.stdout.strip().split()
        key = int(parts[0])
        score = int(parts[1])
        # print(move_string, score)
        return key, score
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired, IndexError, ValueError):
        return None, None

def find_best_move_for(pos):
    position_key, _ = call_solver(pos)
    if position_key is None:
        return None

    best_move = -1
    min_opponent_score = float('inf')

    # Try all possible next moves; call_solver will return None for illegal ones.
    for move_col in range(WIDTH):
        next_pos = pos + str(move_col + 1)
        _, score = call_solver(next_pos)
        
        if score is not None and score < min_opponent_score:
            min_opponent_score = score
            best_move = move_col

    if best_move != -1:
        # Pack the key and move into a single 64-bit integer.
        packed_value = (position_key << 4) | best_move
        return packed_value
    return None

def main():
    if not os.path.exists(SOLVER_PATH):
        print(f"Error: Solver executable not found at '{SOLVER_PATH}'")
        return

    # Generate all possible move sequences (legal and illegal) from depth 0 to DEPTH
    positions = []
    for d in range(DEPTH + 1):
        for p in product("1234567", repeat=d):
            positions.append("".join(p))
    
    print(f"Analyzing {len(positions)} potential positions up to depth {DEPTH}...")

    with Pool(processes=cpu_count()) as pool:
        # Use tqdm to create a progress bar for the map operation.
        # imap_unordered is used for better progress bar updates.
        results = list(tqdm(pool.imap_unordered(find_best_move_for, positions), total=len(positions), desc="Analyzing"))

    print("\nWriting book...")
    book_entries = 0
    with open(BOOK_OUTPUT_PATH, 'wb') as f:
        for packed_entry in results:
            if packed_entry is not None:
                f.write(struct.pack('<Q', packed_entry))
                book_entries += 1
    
    print(f"Book creation complete. Wrote {book_entries} valid entries to '{BOOK_OUTPUT_PATH}'.")

if __name__ == "__main__":
    main()
