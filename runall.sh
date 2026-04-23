#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# If args are provided, treat them as input files.
# Otherwise, run on the announced testcases in testcases/.
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
	local elapsed
	local start

	echo "Running $algo_name on $input_file..."
	start=$SECONDS
	"./$exe" "$input_file" >> "$output_file"
	elapsed=$((SECONDS - start))
	echo -e "    Completed in ${elapsed}s \n"
	echo -e "Time: ${elapsed}s \n" >> 	"$output_file"
	echo "" >> "$output_file"
}

for input_file in "${INPUT_FILES[@]}"; do
	testcase_start="$SECONDS"
	base_name="$(basename "$input_file")"
	base_name="${base_name%.txt}"
	output_file="outputs/${base_name}_output.txt"

	> "$output_file"
	echo "Testcase: $input_file" >> "$output_file"
	echo "" >> "$output_file"

	run_one "Flow Algo-1" "flow1" "$input_file" "$output_file"
	run_one "Flow Algo-4" "flow4" "$input_file" "$output_file"
	run_one "Greedy" "greedy" "$input_file" "$output_file"
	run_one "Greedy++" "greedypp" "$input_file" "$output_file"

	testcase_elapsed=$((SECONDS - testcase_start))

	echo -e "\nTotal time for $input_file: ${testcase_elapsed}s"

	echo -e "Output file at $output_file \n\n"
done

echo "Done."