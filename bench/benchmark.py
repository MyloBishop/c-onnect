import sys
import subprocess
from pathlib import Path

TIMOUT = 1000 # 1s per test case

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
    """Formats microseconds into a human-readable string (s, ms, or us)."""
    us = float(us)
    if us >= 1_000_000:
        return f"{us / 1_000_000:.3f} s"
    if us >= 1_000:
        return f"{us / 1_000:.3f} ms"
    return f"{int(us)} us" if us >= 1 else f"{us:.2f} us"

def run_all_tests(executable_path):
    tests_dir = Path(__file__).parent / "tests"
    print(f"{colors.HEADER}--- Running Benchmarks on '{executable_path}' ---{colors.ENDC}")

    for test_file_name in TEST_ORDER:
        test_file_path = tests_dir / test_file_name
        print(f"\n{colors.BOLD}Running Test Set: {test_file_name}{colors.ENDC}")

        if not test_file_path.is_file():
            print(f"  {colors.FAIL}-> ERROR: Test file not found.{colors.ENDC}")
            print(f"\n{colors.FAIL}--- Halting due to missing test file. ---{colors.ENDC}")
            return

        try:
            result = subprocess.run(
                [executable_path, str(test_file_path)],
                capture_output=True, text=True, timeout=TIMOUT, check=True
            )

            passed, nodes, time_us = map(int, result.stdout.strip().split())

            if passed > 0:
                # Calculate performance metrics.
                time_sec = time_us / 1_000_000.0
                avg_time_per_pos = time_us / passed
                avg_nodes_per_pos = nodes / passed
                pos_per_sec = passed / time_sec if time_sec > 0 else 0
                kilo_nodes_per_sec = (nodes / 1000.0) / time_sec if time_sec > 0 else 0

                # Print the condensed, colored summary.
                print(f"  {colors.OKGREEN}-> PASS{colors.ENDC} ({passed} cases)")
                print(f"     Total Time: {colors.BOLD}{format_time(time_us)}{colors.ENDC}")
                print(f"     Avg: {colors.BOLD}{format_time(avg_time_per_pos)}{colors.ENDC}/pos, {colors.BOLD}{avg_nodes_per_pos:,.0f}{colors.ENDC} nodes/pos")
                print(f"     Perf: {colors.BOLD}{pos_per_sec:.2f}{colors.ENDC} pos/s, {colors.BOLD}{kilo_nodes_per_sec:.2f}{colors.ENDC} kn/s")
            else:
                print(f"  {colors.OKGREEN}-> PASS{colors.ENDC} (0 test cases in file)")

        except subprocess.TimeoutExpired:
            print(f"  {colors.FAIL}-> TIMEOUT{colors.ENDC} (Exceeded {str(TIMEOUT)})")
            print(f"\n{colors.FAIL}--- Halting due to failure in suite: {test_file_name}. ---{colors.ENDC}")
            return
        except subprocess.CalledProcessError as e:
            print(f"  {colors.FAIL}-> C-PROGRAM FAILED (Code: {e.returncode}){colors.ENDC}")
            print(f"     {colors.BOLD}C-program stderr:{colors.ENDC}\n{e.stderr.strip()}")
            print(f"\n{colors.FAIL}--- Halting due to failure in suite: {test_file_name}. ---{colors.ENDC}")
            return
        except Exception as e:
            print(f"  {colors.FAIL}-> SCRIPT ERROR:{colors.ENDC} {e}")
            print(f"\n{colors.FAIL}--- Halting due to script error. ---{colors.ENDC}")
            return

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: python3 {Path(__file__).name} <path_to_c_executable>")
        sys.exit(1)

    executable = sys.argv[1]
    if not Path(executable).is_file():
        print(f"{colors.FAIL}Error: Executable not found at '{executable}'{colors.ENDC}")
        sys.exit(1)

    run_all_tests(executable)