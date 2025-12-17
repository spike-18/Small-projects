#!/usr/bin/gnuplot
# Generic plotting script. Pass parameters via -e:
# gnuplot -e "datafile='results/solution.dat'; outputfile='plots/solution.png'; plottitle='Solution'" scripts/plot.plt

set terminal pngcairo size 1200,800 enhanced font 'Verdana,10'

if (!exists("datafile")) datafile = "results/solution.dat"
if (!exists("outputfile")) outputfile = "plots/solution.png"
if (!exists("plottitle")) plottitle = sprintf("Solution from %s", datafile)

set output outputfile
set title plottitle
set xlabel "x"
set ylabel "y"
set grid

plot datafile using 1:2 with lines lw 2 lc rgb "#d62728" notitle
