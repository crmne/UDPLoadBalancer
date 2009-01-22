#!/bin/sh
echo '
set terminal png size 1440,900
set xlabel "Packet id"
set ylabel "Delay in milliseconds"
set autoscale
set title "Delay Mobile to Fixed"
set output "delayfixed.png"
plot "delayfixed.txt" using 1:2 index 0 with points
set title "Delay Fixed to Mobile"
set output "delaymobile.png"
plot "delaymobile.txt" using 1:2 index 0 with points
' | gnuplot
