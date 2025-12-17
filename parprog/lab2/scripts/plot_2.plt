#!/usr/bin/gnuplot

# Setup
reset session
set terminal pngcairo size 1200,800 enhanced font 'Verdana,10'
set output "plots/task_2.png"

datafile = "results/task_2.csv"

set multiplot layout 2,2 title "Parallel Program Performance Analysis" font ",14"

set title "Execution Time vs Number of Threads"
set xlabel "Number of Threads/Processes"
set ylabel "Execution Time (seconds)"
set grid
set logscale y
set key top right

plot datafile using 1:2 with linespoints lw 2 pt 7 ps 1.2 lc rgb "red" title "Execution Time"

set title "Speedup (Acceleration)"
set xlabel "Number of Threads/Processes"
set ylabel "Speedup"
set grid
set yrange [0:*]
unset logscale y

serial_time = 0
stats datafile using 2 nooutput
do for [i=1:STATS_records] {
    stats datafile every ::i-1::i-1 using 1:2 nooutput
    if (STATS_min_x == 1) {
        serial_time = STATS_min_y
        break
    }
}

plot datafile using 1:(serial_time/$2) with linespoints lw 2 pt 7 ps 1.2 lc rgb "blue" title "Speedup", \
     x with lines dt 2 lc rgb "black" title "Linear Speedup (Ideal)"

set title "Parallel Efficiency"
set xlabel "Number of Threads/Processes"
set ylabel "Efficiency (%)"
set grid
set yrange [0:110]

plot datafile using 1:(serial_time/$2/$1*100) with linespoints lw 2 pt 7 ps 1.2 lc rgb "green" title "Efficiency", \
     100 with lines dt 2 lc rgb "black" title "Ideal Efficiency"
