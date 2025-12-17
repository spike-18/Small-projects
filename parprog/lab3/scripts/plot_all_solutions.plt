#!/usr/bin/gnuplot
# Plot multiple solutions on one figure. Parameters passed via -e:
#   filelist  - space-separated data files (each with x,y)
#   blist     - space-separated b labels corresponding to filelist
#   outputfile- png output path (default plots/solutions_all.png)
#   plottitle - plot title

set terminal pngcairo size 1200,800 enhanced font 'Verdana,10'

if (!exists("filelist")) filelist = ""
if (!exists("blist")) blist = ""
if (!exists("outputfile")) outputfile = "plots/solutions_all.png"
if (!exists("plottitle")) plottitle = "Solutions for different b"

n = words(filelist)
if (n == 0) {
    print "No files to plot (filelist is empty)."
    exit
}

set output outputfile
set title plottitle
set xlabel "x"
set ylabel "y"
set grid
set key outside
set palette model RGB defined (0 "#1f77b4", 0.5 "#2ca02c", 1 "#d62728")

color(i) = (n > 1) ? ((i - 1.0) / (n - 1.0)) : 0.5

plot for [i=1:n] word(filelist, i) using 1:2 with lines lw 2 lc palette frac color(i) title sprintf("b=%s", word(blist, i))
