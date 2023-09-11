#!/usr/bin/env bash

# No negative values should be seen
# Reflected: 1-8
# Transmitted: 11-18

A="  1  0.0  2  0.0   3 0.0  4 0.0   0.00000E+00 0.0\n \
 5  0.0  6  0.0   7 0.0  8 0.0   0.00000E+00 0.0\n \
 -1  0.0  -2 0.0  -3 0.0  -4 0.0  0.00000E+00 0.0\n \
 -5 0.0  -6 0.0  -7 0.0  -8.1 0.0  0.00000E+00 0.0\n \
 -11 0.0  -12 0.0  -13 0.0 -14 0.0  0.00000E+00 0.0\n \
 -15 0.0  -16 0.0  -17 0.0  -18.1 0.0  0.00000E+00 0.0\n \
 11 0.0  12 0.0  13 0.0  14 0.0  0.00000E+00 0.0\n \
 15 0.0  16 0.0  17 0.0  18 0.0  0.00000E+00 0.0 0.00000E+00 0.0"
cat ../../../test/bins/mctal.template | sed "s;^VALUES;$A;" > mctal

rm -fr case*
mctal2root mctal
pstudy -i test.pstudy -setup
for dir in case*; do
    cp mctal.root $dir
done
