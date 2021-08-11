#!/bin/sh
# tst.sh - script to build tgen

gcc -Wall -o tgen nstm.c tgen.c
if [ $? -ne 0 ]; then exit 1; fi

time ./tgen "$@"
