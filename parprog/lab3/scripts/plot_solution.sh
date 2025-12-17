#!/bin/bash

# Generate solution data for y'' - e^y = 0 on [0,1] and plot it with gnuplot.
# Usage: ./scripts/plot_solution.sh [b] [solver]
#   b       - right boundary value y(1)=b (default: 0.5)
#   solver  - base|omp (default: base)
#   POINTS  - env var for total grid points (default: 4000)
# Example: ./scripts/plot_solution.sh 0.3 omp

set -euo pipefail

cd "$(dirname "$0")/.."

b="${1:-0.5}"
solver="${2:-base}"
points="${POINTS:-4000}"

if [[ "${solver}" != "base" && "${solver}" != "omp" ]]; then
  echo "solver must be 'base' or 'omp'" >&2
  exit 1
fi

mkdir -p results plots

b_label="${b/./p}"
datafile="results/solution_${solver}_b${b_label}.dat"
outfile="plots/solution_${solver}_b${b_label}.png"

echo "Running solver: bin/main_${solver} b=${b}, points=${points}"
bin/main_"${solver}" "${b}" "${points}" 1>/dev/null 2> "${datafile}"

echo "Plotting to ${outfile}"
gnuplot -e "datafile='${datafile}'; outputfile='${outfile}'; plottitle='y(x) with b=${b} (solver=${solver})'" scripts/plot.plt

echo "Done. Output data: ${datafile}"
echo "      Output plot: ${outfile}"
