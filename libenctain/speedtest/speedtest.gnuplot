#!/usr/bin/env gnuplot

set terminal pdf solid size 5.0, 3.5
set output 'speedtest.pdf'

set xrange [10:1600000]
set format x "%.0f"

### Plot ###

set title "Serpent Ciphers: Absolute Time by Data Length with Standard Deviation"
set xlabel "Data Length in Bytes"
set ylabel "Seconds"
set logscale x
set logscale y
set key below

plot "gcrypt-serpent-ecb.txt" using 1:2:3 title "Libgcrypt ECB" with errorlines pointtype 0, \
     "gcrypt-serpent-cbc.txt" using 1:2:3 title "Libgcrypt CBC" with errorlines pointtype 0, \
     "custom-serpent-ecb.txt" using 1:2:3 title "Custom ECB" with errorlines pointtype 0, \
     "custom-serpent-cbc.txt" using 1:2:3 title "Custom CBC" with errorlines pointtype 0

### Plot ###

set title "Serpent Ciphers: Speed by Data Length"
set xlabel "Data Length in Bytes"
set ylabel "Megabyte / Second"
set logscale x
unset logscale y
set key below

plot "gcrypt-serpent-ecb.txt" using 1:($1 / $2) / 1048576 title "Libgcrypt ECB" with lines, \
     "gcrypt-serpent-cbc.txt" using 1:($1 / $2) / 1048576 title "Libgcrypt CBC" with lines, \
     "custom-serpent-ecb.txt" using 1:($1 / $2) / 1048576 title "Custom ECB" with lines, \
     "custom-serpent-cbc.txt" using 1:($1 / $2) / 1048576 title "Custom CBC" with lines
