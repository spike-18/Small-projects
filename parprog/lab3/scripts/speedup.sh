#!/bin/bash

# --- Configuration ---
RESULTS_FILE="benchmark_results.csv"

# --- Argument Parsing ---
if [ "$#" -lt 3 ]; then
    echo "Usage: $0 <b_value> <start_N> <end_N> [step_N]"
    exit 1
fi

BIN1="bin/main_base"
BIN2="bin/main_omp"
B_VAL="$1"
START_N="$2"
END_N="$3"
STEP_N="${4:-100}" # Default step is 100 if not provided

# --- Validation ---
if [[ ! -x "$BIN1" ]] || [[ ! -x "$BIN2" ]]; then
    echo "Error: One or both binaries are not executable or found."
    exit 1
fi

if ! command -v gnuplot &> /dev/null; then
    echo "Error: gnuplot is not installed."
    exit 1
fi

if ! command -v bc &> /dev/null; then
    echo "Error: 'bc' calculator is not installed (needed for floating point math)."
    exit 1
fi

# --- Data Collection ---
echo "Benchmarking..."
echo "N,Time1,Time2,Speedup" > "$RESULTS_FILE"

# Progress bar function
show_progress() {
    local width=50
    local percent=$1
    local num_chars=$(echo "$percent * $width / 100" | bc)
    printf "\r["
    for ((i=0; i<num_chars; i++)); do printf "#"; done
    for ((i=num_chars; i<width; i++)); do printf " "; done
    printf "] %d%% (N=%d)" "$percent" "$2"
}

total_steps=$(( (END_N - START_N) / STEP_N + 1 ))
current_step=0

for (( n=START_N; n<=END_N; n+=STEP_N )); do
    # Run Binary 1
    # We grab the LAST line of stdout, assuming it is the time
    out1=$("$BIN1" "$B_VAL" "$n" 2>/dev/null)
    time1=$(echo "$out1" | tail -n 1)

    # Run Binary 2
    out2=$("$BIN2" "$B_VAL" "$n" 2>/dev/null)
    time2=$(echo "$out2" | tail -n 1)

    # Calculate Speedup (Time1 / Time2) using awk for safety with floats
    if (( $(echo "$time2 == 0" | bc -l) )); then
        speedup=0
    else
        speedup=$(awk "BEGIN {print $time1 / $time2}")
    fi

    echo "$n,$time1,$time2,$speedup" >> "$RESULTS_FILE"

    # Update progress
    ((current_step++))
    percent=$(( current_step * 100 / total_steps ))
    show_progress $percent $n
done

echo -e "\nDone! Results saved to $RESULTS_FILE"

# --- Plotting ---
echo "Generating plot image..."

# Check if data file exists and has content (more than just header)
if [ ! -s "$RESULTS_FILE" ] || [ $(wc -l < "$RESULTS_FILE") -le 1 ]; then
    echo "Error: CSV file is empty or contains only headers. Cannot plot."
    exit 1
fi

PLOT_IMG="benchmark_plot.png"

# We use a Here-Doc (<<EOF) to pass commands cleanly to gnuplot
gnuplot <<EOF
    # Set output to PNG image
    set terminal pngcairo size 1000,800 enhanced font 'Verdana,10'
    set output '$PLOT_IMG'

    # CSV Setup
    set datafile separator ','
    set decimalsign '.'       
    set grid
    
    # Layout Setup
    set multiplot layout 2,1 title 'Performance Comparison: $BIN1 vs $BIN2'
    
    # Plot 1: Execution Time
    set title 'Execution Time vs Total Points (N)'
    set xlabel 'N'
    set ylabel 'Time (seconds)'
    set key top left box opaque
    
    plot '$RESULTS_FILE' skip 1 using 1:2 with linespoints lw 2 pt 7 ps 1.5 title 'Base', \
         '$RESULTS_FILE' skip 1 using 1:3 with linespoints lw 2 pt 5 ps 1.5 title 'Reduction (OMP)'

    # Plot 2: Speedup
    set title 'Speedup (T_{base} / T_{OMP})'
    set xlabel 'N'
    set ylabel 'Factor'
    set key top left box opaque
    
    # Reference line at 1.0
    set arrow from graph 0, first 1 to graph 1, first 1 nohead lc rgb 'black' dt 2
    
    plot '$RESULTS_FILE' skip 1 using 1:4 with linespoints lw 2 lc rgb 'forest-green' pt 7 ps 1.5 title 'Speedup'
    
    unset multiplot
EOF

if [ $? -eq 0 ]; then
    echo "Success! Plot saved to: $PLOT_IMG"
else
    echo "Error: Gnuplot failed to generate the image."
fi