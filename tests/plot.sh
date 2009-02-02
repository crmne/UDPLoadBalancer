#!/bin/sh
tests/packloss delayfixed.txt lossfixed.txt
tests/packloss delaymobile.txt lossmobile.txt
echo '
set terminal png size 1280,750
set xlabel "Packet id"
set ylabel "Delay in milliseconds"
set autoscale
set grid

set title "Delay and Packet Loss Mobile to Fixed"
set output "fixed.png"
plot "delayfixed.txt" with linespoints, "lossfixed.txt" using 1:($2*10) with linespoints

set title "Delay and Packet Loss Fixed to Mobile"
set output "mobile.png"
plot "delaymobile.txt" with linespoints, "lossmobile.txt" using 1:($2*10) with linespoints 
' | gnuplot
#FIT_LIMIT=1e-100
#FIT_START_LAMBDA=1
#f1(x) = a1+abs(b1*sin((x+c1)/d1))
#a1=50.2879;b1=43.5836;c1=333.519;d1=190.958
#fit f1(x) "delayfixed.txt" using 1:2 via a1,b1,c1,d1
#
#f2(x) = a2+abs(b2*sin((x+c2)/d2))
#a2=49.652;b2=44.4597;c2=536.779;d2=190.976
#fit f2(x) "delaymobile.txt" using 1:2 via a2,b2,c2,d2
 
