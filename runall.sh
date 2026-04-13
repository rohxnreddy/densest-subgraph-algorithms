#!/bin/bash

# Create folders
mkdir -p outputs
mkdir -p inputs

# -------- GENERATE INPUT --------
echo "Generating graph..."
python3 generator.py "$@"

INPUT_FILE="inputs/input.txt"
OUTPUT_FILE="outputs/output.txt"

# Clear previous output
> "$OUTPUT_FILE"

# -------- COMPILE --------
echo "Compiling..."

g++ 1_flow1.cpp -O2 -o flow1
g++ 2_flow4.cpp -O2 -o flow4
g++ 3_greedy.cpp -O2 -o greedy
g++ 4_greedypp.cpp -O2 -o greedypp

# -------- RUN ALL --------

echo "Running Flow Algo-1..."
echo "========== Flow Algo-1 ==========" >> "$OUTPUT_FILE"
./flow1 "$INPUT_FILE" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

echo "Running Flow Algo-4..."
echo "========== Flow Algo-4 ==========" >> "$OUTPUT_FILE"
./flow4 "$INPUT_FILE" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

echo "Running Greedy..."
echo "========== Greedy ==========" >> "$OUTPUT_FILE"
./greedy "$INPUT_FILE" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

echo "Running Greedy++..."
echo "========== Greedy++ ==========" >> "$OUTPUT_FILE"
./greedypp "$INPUT_FILE" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

echo "Done. Output saved to $OUTPUT_FILE"