#!/bin/bash
gcc -c 1lab5.c
gcc -c 2lab5.c
gcc -o  1lab5 1lab5.o -lpthread 
gcc -o  2lab5 2lab5.o -lpthread
