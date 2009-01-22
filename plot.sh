#!/bin/sh
echo '
set terminal png size 1440,900
set xlabel "Packet id"
set ylabel "Delay in milliseconds"
set autoscale
set grid
set title "Delay Mobile to Fixed"
set output "delayfixedpoints.png"
plot "delayfixed.txt" with points
set output "delayfixedlines.png"
plot "delayfixed.txt" with lines
set title "Delay Fixed to Mobile"
set output "delaymobilepoints.png"
plot "delaymobile.txt" with points
set output "delaymobilelines.png"
plot "delaymobile.txt" with lines
' | gnuplot
