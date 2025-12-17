#!/bin/bash

# Compare runtime of base vs omp solvers versus grid size (N points).
# Usage: ./scripts/plot_runtime.sh
#   N_VALUES        - space-separated list of total grid points (default: "400 800 1200 1600 2000 2400 2800 3200 3600 4000")
#   B_VALUE         - boundary value b for y(1)=b (default: 0.5)
#   OMP_NUM_THREADS - env var for omp threads (default: 4)
# Outputs:
#   results/runtime.csv   (N,base,omp)
#   plots/runtime.png

set -euo pipefail

cd "$(dirname "$0")/.."

b_value="${B_VALUE:-0.3}"
n_values=${N_VALUES:-"400 1000 2000 4000 8000 16000"}
export OMP_NUM_THREADS="${OMP_NUM_THREADS:-4}"

mkdir -p results plots

datafile="results/runtime.csv"
outfile="plots/runtime.png"

echo "#N,base,omp" > "${datafile}"

for n in ${n_values}; do
  base_time=$(bin/main_base "${b_value}" "${n}" 2>/dev/null)
  omp_time=$(bin/main_omp "${b_value}" "${n}" 2>/dev/null)
  echo "${n},${base_time},${omp_time}" >> "${datafile}"
done

gnuplot -e "datafile='${datafile}'; outputfile='${outfile}'" scripts/plot_runtime.plt

echo "Done."
echo "Data: ${datafile}"
echo "Plot: ${outfile}"
