#!/usr/bin/gnuplot

reset session
set terminal pngcairo size 1200,800 enhanced font 'Verdana,10'
set output "plots/time.png"

set xlabel "time"
set ylabel "N"
set grid

plot "results/base.csv" using (20 / $1):2 with lines lw 2 lc rgb "red" title "single thread", \
     "results/omp.csv" using (20 / $1):2 with lines lw 2 lc rgb "blue" title "parallel",
