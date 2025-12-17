#!/bin/bash

# Plot all solutions for b in [0.0, 1.0] step 0.1 on a single figure.
# Usage: ./scripts/plot_all_solutions.sh [solver]
#   solver - base|omp (default: base)
#   POINTS - env var for total grid points (default: 4000)
# Outputs:
#   results/solutions_<solver>_bX.dat for each b
#   plots/solutions_<solver>_all.png

set -euo pipefail

cd "$(dirname "$0")/.."

solver="${1:-base}"
points="${POINTS:-4000}"
if [[ "${solver}" != "base" && "${solver}" != "omp" ]]; then
  echo "solver must be 'base' or 'omp'" >&2
  exit 1
fi

mkdir -p results plots

b_values=(0.0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0)
filelist=()

for b in "${b_values[@]}"; do
  b_label="${b/./p}"
  datafile="results/solutions_${solver}_b${b_label}.dat"
  echo "Running solver ${solver} for b=${b}, points=${points}"
  bin/main_"${solver}" "${b}" "${points}" 1>/dev/null 2> "${datafile}"
  filelist+=("${datafile}")
done

# Build space-separated strings for gnuplot
filelist_str=""
for f in "${filelist[@]}"; do
  filelist_str+="${f} "
done

outfile="plots/solutions_${solver}_all.png"
plottitle="Solutions for b in [0.0, 1.0] (solver=${solver}, points=${points})"

gnuplot -e "filelist='${filelist_str% }'; blist='${b_values[*]}'; outputfile='${outfile}'; plottitle='${plottitle}'" scripts/plot_all_solutions.plt

echo "Done."
echo "Data files: ${filelist[*]}"
echo "Plot: ${outfile}"
