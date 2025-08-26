import subprocess
import os
import struct
from multiprocessing import Pool, cpu_count
from collections import deque
import sys
from tqdm import tqdm

# --- Configuration ---
SOLVER_PATH = os.path.join("bin", "solver")
BOOK_FILE_PATH = "book.bin"
MAX_DEPTH = 2
WIDTH = 7
HEIGHT = 6
SOLVER_TIMEOUT = 300

def get_board_state_from_sequence(move_sequence):
    if not move_sequence:
        return 0, 0
    
    try:
        result = subprocess.run(
            [SOLVER_PATH, move_sequence],
            capture_output=True, text=True, check=True, timeout=SOLVER_TIMEOUT
        )
        parts = result.stdout.strip().split()
        return int(parts[0]), int(parts[1])
    except (IndexError, ValueError):
        print(f"FATAL: Could not parse solver output for '{move_sequence}'. Halting.", file=sys.stderr)
        raise
    except subprocess.TimeoutExpired:
        print(f"Warning: Timeout getting board state for '{move_sequence}'. Skipping position.", file=sys.stderr)
        return None, None
    except subprocess.CalledProcessError:
        return None, None

def find_best_move_for_sequence(move_sequence):
    col_counts = {str(i): 0 for i in range(1, WIDTH + 1)}
    for move in move_sequence:
        col_counts[move] += 1

    possible_moves = [str(col) for col in range(1, WIDTH + 1) if col_counts[str(col)] < HEIGHT]
    if not possible_moves:
        return None, None

    scores = {}
    for move in possible_moves:
        next_sequence = move_sequence + move
        try:
            result = subprocess.run(
                [SOLVER_PATH, next_sequence],
                capture_output=True, text=True, check=True, timeout=SOLVER_TIMEOUT
            )
            scores[move] = int(result.stdout.strip().split()[2])
        except (IndexError, ValueError):
            print(f"FATAL: Could not parse solver output for '{next_sequence}'. Worker will terminate.", file=sys.stderr)
            raise
        except subprocess.TimeoutExpired:
            print(f"Warning: Solver timed out for sequence '{next_sequence}'. Skipping this move.", file=sys.stderr)
        except subprocess.CalledProcessError:
            pass

    if not scores:
        return None, None

    best_move_str = min(scores, key=scores.get)
    best_move = int(best_move_str) - 1

    current_pos, mask = get_board_state_from_sequence(move_sequence)
    if current_pos is None:
        return None, None

    key = (mask << 64) | current_pos
    return key, best_move

def main():
    if not os.path.exists(SOLVER_PATH):
        print(f"Error: Solver executable not found at '{SOLVER_PATH}'.", file=sys.stderr)
        sys.exit(1)

    print(f"Generating opening book up to depth {MAX_DEPTH} using {cpu_count()} cores.")
    
    sequences_to_analyze = set()
    q = deque([""])
    visited_sequences = {""}

    while q:
        seq = q.popleft()
        sequences_to_analyze.add(seq)

        if len(seq) < MAX_DEPTH - 1:
            col_counts = {str(i): 0 for i in range(1, WIDTH + 1)}
            for m in seq:
                col_counts[m] += 1
            
            for col in range(1, WIDTH + 1):
                if col_counts[str(col)] < HEIGHT:
                    next_seq = seq + str(col)
                    if next_seq not in visited_sequences:
                        visited_sequences.add(next_seq)
                        q.append(next_seq)
    
    print(f"Found {len(sequences_to_analyze)} unique positions to analyze.")
    
    book_entries = {}
    with Pool(cpu_count()) as pool:
        sequences_list = list(sequences_to_analyze)
        with tqdm(total=len(sequences_list), desc="Analyzing positions") as pbar:
            for result in pool.imap_unordered(find_best_move_for_sequence, sequences_list):
                if result and result[0] is not None:
                    key, move = result
                    book_entries[key] = move
                pbar.update()

    print(f"\nFound {len(book_entries)} book entries.")
    print(f"Sorting and writing to '{BOOK_FILE_PATH}'...")

    sorted_entries = sorted(book_entries.items())

    with open(BOOK_FILE_PATH, "wb") as f:
        for key, move in sorted_entries:
            mask = key >> 64
            current_pos = key & ((1 << 64) - 1)
            f.write(struct.pack("<QQB", current_pos, mask, move))

    print("Done.")

if __name__ == "__main__":
    main()