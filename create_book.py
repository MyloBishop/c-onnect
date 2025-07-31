import os
import subprocess
from multiprocessing import Pool, cpu_count
import struct
from tqdm import tqdm

# --- Configuration ---
DEPTH = 5               # How many moves deep to build the book.
WIDTH = 7
HEIGHT = 6
SOLVER_PATH = "./bin/solver"  # Path to your game solver executable.
BOOK_OUTPUT_PATH = "book.bin" # Final output file for the C engine.
TIMEOUT_SECONDS = 300

# --- Helper Functions ---

def get_mirror_sequence(move_string):
    """Calculates the mirror image of a sequence of moves (e.g., "123" -> "765")."""
    return "".join(str(WIDTH + 1 - int(c)) for c in move_string)

def call_solver(move_string):
    """Calls the external solver and returns the position key and score."""
    # Your solver might require a specific input for the empty board, e.g., "0".
    # An empty list as an argument can cause errors.
    solver_arg = move_string if move_string else "0"
    
    try:
        result = subprocess.run(
            [SOLVER_PATH, solver_arg],
            capture_output=True,
            text=True,
            check=True,
            timeout=TIMEOUT_SECONDS,
        )
        parts = result.stdout.strip().split()
        key = int(parts[0])
        score = int(parts[1])
        return key, score
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired, IndexError, ValueError):
        return None, None

def generate_legal_positions(max_depth):
    """
    Generates all unique, legal move sequences up to the specified depth.
    This is far more efficient than generating every possible permutation.
    """
    all_positions = {""}
    current_depth_positions = {""}

    for _ in range(max_depth):
        next_depth_positions = set()
        for pos in current_depth_positions:
            column_heights = [pos.count(str(i + 1)) for i in range(WIDTH)]
            for col in range(WIDTH):
                if column_heights[col] < HEIGHT:
                    next_depth_positions.add(pos + str(col + 1))
        
        if not next_depth_positions:
            break # No more legal moves can be made
            
        all_positions.update(next_depth_positions)
        current_depth_positions = next_depth_positions
            
    return list(all_positions)

# --- Core Logic ---

def find_book_entry_for(pos):
    """
    For a given position, finds its canonical key and the best next move.
    A position's key is compared to its mirror image, and only the one with the
    smaller key (the canonical one) is stored. This halves the book size.
    """
    # 1. Get the key for the current position.
    position_key, _ = call_solver(pos)
    if position_key is None:
        return None

    # 2. Check for symmetry (canonicalization).
    if pos: # The empty board has no mirror.
        mirror_pos = get_mirror_sequence(pos)
        mirror_key, _ = call_solver(mirror_pos)
        # If the mirror version has a smaller key, we skip this one.
        if mirror_key is not None and mirror_key < position_key:
            return None 

    # 3. Find the best move using 1-ply minimax.
    best_move = -1
    min_opponent_score = float('inf')
    column_heights = [pos.count(str(i + 1)) for i in range(WIDTH)]

    for move_col in range(WIDTH):
        if column_heights[move_col] < HEIGHT:
            next_pos = pos + str(move_col + 1)
            _, score = call_solver(next_pos)
            
            if score is not None and score < min_opponent_score:
                min_opponent_score = score
                best_move = move_col # Store as 0-indexed column

    # 4. If a valid move was found, pack the data for C.
    if best_move != -1:
        # Pack the key and move into a single 64-bit integer.
        # Your C code can unpack this with:
        # uint64_t key = packed_value >> 4;
        # int move = packed_value & 0xF;
        packed_value = (position_key << 4) | best_move
        return packed_value
        
    return None

def main():
    """Main function to generate, analyze, and write the opening book."""
    if not os.path.exists(SOLVER_PATH):
        print(f"❌ Error: Solver executable not found at '{SOLVER_PATH}'")
        return

    # 1. Generate all legal positions up to the desired depth.
    print(f"Generating legal positions up to depth {DEPTH}...")
    positions = generate_legal_positions(DEPTH)
    print(f"Found {len(positions)} unique legal positions to analyze.")

    # 2. Analyze positions in parallel to find the best move for each.
    book_entries = []
    with Pool(processes=cpu_count()) as pool:
        results_iterator = pool.imap_unordered(find_book_entry_for, positions)
        
        desc = "Analyzing Positions (incl. symmetry reduction)"
        for packed_entry in tqdm(results_iterator, total=len(positions), desc=desc):
            if packed_entry is not None:
                book_entries.append(packed_entry)

    # 3. Sort the entries by key. THIS IS THE CRITICAL STEP for C.
    print(f"\nSorting {len(book_entries)} canonical book entries...")
    book_entries.sort()

    # 4. Write the sorted, packed entries to the binary file.
    print(f"Writing book to '{BOOK_OUTPUT_PATH}'...")
    with open(BOOK_OUTPUT_PATH, 'wb') as f:
        for entry in book_entries:
            # '<Q' packs as a little-endian unsigned 64-bit integer (uint64_t in C).
            f.write(struct.pack('<Q', entry))
            
    print("\n✅ Book creation complete!")
    print(f"Wrote {len(book_entries)} sorted entries to '{BOOK_OUTPUT_PATH}'.")
    print("Your C engine can now load this file and use binary search (bsearch).")

if __name__ == "__main__":
    main()