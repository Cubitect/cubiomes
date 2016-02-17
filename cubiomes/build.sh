#!/bin/bash
gcc -c -g -O3 *.c
gcc -O3 -o "cubiomes"  ./*.o -lm
rm ./*.o 

