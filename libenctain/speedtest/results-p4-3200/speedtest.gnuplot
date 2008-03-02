#!/usr/bin/env gnuplot

set terminal pdf solid size 5.0, 3.5
set output 'speedtest.pdf'

set xrange [10:1600000]
set format x "%.0f"
set label "p4-3200" at screen 0.0,1.0 offset 2.5,-1.5

### Plot ###

set title "Serpent Ciphers: Absolute Time by Data Length with Standard Deviation"
set xlabel "Data Length in Bytes"
set ylabel "Seconds"
set logscale x
set logscale y
set key below

plot "custom-serpent-ecb.txt" using 1:2:3 title "Custom ECB" with errorlines pointtype 0, \
     "custom-serpent-cbc.txt" using 1:2:3 title "Custom CBC" with errorlines pointtype 0, \
     "gcrypt1-serpent-ecb.txt" using 1:2:3 title "Libgcrypt ECB Gentoo" with errorlines pointtype 0, \
     "gcrypt1-serpent-cbc.txt" using 1:2:3 title "Libgcrypt CBC Gentoo" with errorlines pointtype 0, \
     "gcrypt2-serpent-ecb.txt" using 1:2:3 title "Libgcrypt ECB -O3 -fomit-frame-pointer" with errorlines pointtype 0, \
     "gcrypt2-serpent-cbc.txt" using 1:2:3 title "Libgcrypt CBC -O3 -fomit-frame-pointer" with errorlines pointtype 0

### Plot ###

set title "Serpent Ciphers: Speed by Data Length"
set xlabel "Data Length in Bytes"
set ylabel "Megabyte / Second"
set logscale x
unset logscale y
set key below

plot "custom-serpent-ecb.txt" using 1:($1 / $2) / 1048576 title "Custom ECB" with lines, \
     "custom-serpent-cbc.txt" using 1:($1 / $2) / 1048576 title "Custom CBC" with lines, \
     "gcrypt1-serpent-ecb.txt" using 1:($1 / $2) / 1048576 title "Libgcrypt ECB Gentoo" with lines, \
     "gcrypt1-serpent-cbc.txt" using 1:($1 / $2) / 1048576 title "Libgcrypt CBC Gentoo" with lines, \
     "gcrypt2-serpent-ecb.txt" using 1:($1 / $2) / 1048576 title "Libgcrypt ECB -O3 -fomit-frame-pointer" with lines, \
     "gcrypt2-serpent-cbc.txt" using 1:($1 / $2) / 1048576 title "Libgcrypt CBC -O3 -fomit-frame-pointer" with lines
