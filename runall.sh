#!/usr/bin/env bash

set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Restore original input logic:
# If args are provided, use them. Otherwise, use the default list.
INPUT_FILES=()
if [[ "$#" -gt 0 ]]; then
    INPUT_FILES=("$@")
else
    INPUT_FILES=(
        "testcases/wiki-Vote.txt"
        "testcases/email-Enron.txt"
        "testcases/as-skitter.txt"
    )
fi

# Basic check for input files
for f in "${INPUT_FILES[@]}"; do
    if [[ ! -f "$f" ]]; then
        echo "Error: input file not found: $f" >&2
        exit 1
    fi
done

echo -e "Compiling... \n"
make -s all
mkdir -p "outputs"

run_one() {
    local algo_name="$1"
    local exe="$2"
    local input_file="$3"
    local output_file="$4"

    echo "Running $algo_name on $input_file..."

    # The Python block starts here with a double quote
    python3 -c "
import subprocess, time, resource, os

start = time.perf_counter()

with open('$output_file', 'a') as f:
    process = subprocess.Popen(['./$exe', '$input_file'], stdout=f)
    process.wait()

end = time.perf_counter()
elapsed = end - start

usage = resource.getrusage(resource.RUSAGE_CHILDREN)
peak_mem_mb = usage.ru_maxrss / 1024.0

print(f'Completed in {elapsed:.4f}s (Peak Memory: {peak_mem_mb:.2f} MB)')

with open('$output_file', 'a') as f:
    f.write(f'Time: {elapsed:.4f}s\n')
    f.write(f'Peak Memory Usage: {peak_mem_mb:.2f} MB\n\n')
" # The Python block MUST end with this quote on its own line

    # Add your echo here, OUTSIDE the python3 -c "..." block
    echo ""
}

for input_file in "${INPUT_FILES[@]}"; do
    # Capture start for total testcase time calculation
    testcase_start_val=$(python3 -c "import time; print(time.perf_counter())")

    base_name="$(basename "$input_file")"
    base_name="${base_name%.txt}"
    output_file="outputs/${base_name}_output.txt"

    # Clear output file for new run
    >"$output_file"
    echo "Testcase: $input_file" >>"$output_file"
    echo "" >>"$output_file"

    run_one "Flow Algo-1" "flow1" "$input_file" "$output_file"
    run_one "Flow Algo-4" "flow4" "$input_file" "$output_file"
    run_one "Greedy" "greedy" "$input_file" "$output_file"
    run_one "Greedy++" "greedypp" "$input_file" "$output_file"

    # Calculate total time for this file
    testcase_elapsed=$(python3 -c "import time; print(time.perf_counter() - $testcase_start_val)")

    printf "\nTotal time for %s: %.4fs\n" "$input_file" "$testcase_elapsed"
    echo -e "Output file at $output_file \n\n"
done

echo "Done."
