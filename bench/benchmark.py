import sys
import subprocess
from pathlib import Path

TIMEOUT_SECONDS = 300.0

# ANSI colors for formatted output.
class colors:
    HEADER = '\033[95m'
    OKGREEN = '\033[92m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'

# A hardcoded list of test files to run in a specific order.
TEST_ORDER = [
    "Test_L3_R1.txt", "Test_L2_R1.txt", "Test_L2_R2.txt",
    "Test_L1_R1.txt", "Test_L1_R2.txt", "Test_L1_R3.txt",
]

def format_time(us):
    us = float(us)
    if us >= 1_000_000:
        return f"{us / 1_000_000:.3f} s"
    if us >= 1_000:
        return f"{us / 1_000:.3f} ms"
    return f"{int(us)} us" if us >= 1 else f"{us:.2f} us"

def run_test_file(executable_path, test_file_path):
    print(f"\n{colors.BOLD}Running Test Set: {test_file_path.name}{colors.ENDC}")

    if not test_file_path.is_file():
        print(f"  {colors.FAIL}-> ERROR: Test file not found.{colors.ENDC}")
        return False

    total_passed = 0
    total_nodes = 0
    total_time_us = 0

    try:
        with open(test_file_path, 'r') as f:
            for line_num, line in enumerate(f, 1):
                if line.startswith('#') or not line.strip():
                    continue

                parts = line.strip().split()
                if len(parts) != 2:
                    print(f"  {colors.FAIL}-> PARSE ERROR (Line {line_num}){colors.ENDC}")
                    print(f"     Malformed line: '{line.strip()}'")
                    return False
                
                move_string, expected_score_str = parts
                expected_score = int(expected_score_str)

                # Run the C executable for the single position
                result = subprocess.run(
                    [executable_path, move_string],
                    capture_output=True, text=True, check=True, timeout=TIMEOUT_SECONDS
                )

                # Parse the output: score nodes time_us
                actual_score, nodes, time_us = map(int, result.stdout.strip().split())

                if actual_score != expected_score:
                    print(f"  {colors.FAIL}-> FAIL [Line {line_num}]{colors.ENDC}")
                    print(f"     Position: '{move_string}'")
                    print(f"     Expected: {expected_score}, Got: {actual_score}")
                    return False

                total_passed += 1
                total_nodes += nodes
                total_time_us += time_us

    except subprocess.TimeoutExpired:
        print(f"  {colors.FAIL}-> TIMEOUT [Line {line_num}]{colors.ENDC} (Position '{move_string}' exceeded {TIMEOUT_SECONDS:.1f}s)")
        return False
    except subprocess.CalledProcessError as e:
        print(f"  {colors.FAIL}-> C-PROGRAM FAILED (Code: {e.returncode}){colors.ENDC}")
        print(f"     Position: '{move_string}'")
        print(f"     {colors.BOLD}C-program stderr:{colors.ENDC}\n{e.stderr.strip()}")
        return False
    except FileNotFoundError:   
        print(f"  {colors.FAIL}-> ERROR: Test file not found at '{test_file_path}'.{colors.ENDC}")
        return False


    # All tests in the file passed. Calculate and print summary statistics.
    time_sec = total_time_us / 1_000_000.0
    avg_time_per_pos = total_time_us / total_passed if total_passed > 0 else 0
    avg_nodes_per_pos = total_nodes / total_passed if total_passed > 0 else 0
    pos_per_sec = total_passed / time_sec if time_sec > 0 else 0
    kilo_nodes_per_sec = (total_nodes / 1000.0) / time_sec if time_sec > 0 else 0

    print(f"  {colors.OKGREEN}-> PASS{colors.ENDC} ({total_passed} cases)")
    print(f"      Total Time: {colors.BOLD}{format_time(total_time_us)}{colors.ENDC}")
    print(f"      Avg: {colors.BOLD}{format_time(avg_time_per_pos)}{colors.ENDC}/pos, {colors.BOLD}{avg_nodes_per_pos:,.0f}{colors.ENDC} nodes/pos")
    print(f"      Perf: {colors.BOLD}{pos_per_sec:.2f}{colors.ENDC} pos/s, {colors.BOLD}{kilo_nodes_per_sec:.2f}{colors.ENDC} kn/s")
    return True


def run_all_benchmarks(executable_path):
    tests_dir = Path(__file__).parent / "tests"
    print(f"{colors.HEADER}--- Running Benchmarks on '{executable_path}' ---{colors.ENDC}")

    for test_file_name in TEST_ORDER:
        test_file_path = tests_dir / test_file_name
        if not run_test_file(executable_path, test_file_path):
            print(f"\n{colors.FAIL}--- Halting due to failure in suite: {test_file_name} ---{colors.ENDC}")
            return

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: python3 {Path(__file__).name} <path_to_c_executable>")
        sys.exit(1)

    executable = sys.argv[1]
    if not Path(executable).is_file():
        print(f"{colors.FAIL}Error: Executable not found at '{executable}'{colors.ENDC}")
        sys.exit(1)

    run_all_benchmarks(executable)