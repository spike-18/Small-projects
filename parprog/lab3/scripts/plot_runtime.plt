#!/usr/bin/gnuplot
# Runtime comparison plotter (time vs N points)
# Expects CSV with columns: N,base,omp
# Customize via -e: gnuplot -e "datafile='results/runtime.csv'; outputfile='plots/runtime.png'" scripts/plot_runtime.plt

set datafile separator ","
set terminal pngcairo size 1200,800 enhanced font 'Verdana,10'

if (!exists("datafile")) datafile = "results/runtime.csv"
if (!exists("outputfile")) outputfile = "plots/runtime.png"

set output outputfile
set title sprintf("Runtime comparison (%s)", datafile)
set xlabel "N (grid points)"
set ylabel "Time, s"
set grid
set key top left

plot datafile using 1:2 with linespoints lw 2 lc rgb "#1f77b4" title "base", \
     datafile using 1:3 with linespoints lw 2 lc rgb "#d62728" title "omp"
