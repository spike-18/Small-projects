#!/bin/bash

# --- Defaults ---
DEFAULT_BINARY="main_base"
DEFAULT_B=0.5
DEFAULT_POINTS=50
DATA_FILE="solution.csv"

# --- Argument Parsing ---
# Use provided arguments or fall back to defaults
BINARY="${1:-$DEFAULT_BINARY}"
B_VAL="${2:-$DEFAULT_B}"
POINTS="${3:-$DEFAULT_POINTS}"

# --- Validation ---
if [ ! -f "bin/$BINARY" ]; then
    echo "Error: Binary executable 'bin/$BINARY' not found."
    echo "Usage: $0 [binary in bin/] [b_value] [total_points]"
    echo "Example: $0 main_base 0.5 100"
    exit 1
fi

if ! command -v gnuplot &> /dev/null; then
    echo "Error: gnuplot is not installed."
    exit 1
fi

# --- Execution ---
echo "Running: $BINARY with b=$B_VAL, N=$POINTS"

# Run the binary. 
# We ignore stdout (timing info) by sending it to /dev/null if you don't need it, 
# or keep it displayed. Here we capture stderr (data) to the file.
"./bin/$BINARY" "$B_VAL" "$POINTS" 2> "$DATA_FILE"

if [ $? -ne 0 ]; then
    echo "Simulation failed (non-zero exit code)."
    exit 1
fi

echo "Data written to $DATA_FILE"

# --- Plotting ---
echo "Plotting with Gnuplot..."

gnuplot -p -e "
    set datafile separator ',';
    set title 'Solution (b=$B_VAL, N=$POINTS)';
    set xlabel 'x';
    set ylabel 'y';
    set grid;
    set key bottom right;
    plot '$DATA_FILE' using 1:2 with lines pt 20 ps 0.5 lw 1.5 lc rgb 'blue' title 'y(x)';
"