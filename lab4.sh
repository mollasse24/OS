#!/bin/bash
gcc -c dlab4.c
gcc -c plab4.c
gcc -o  dlab4 dlab4.o -lpthread 
gcc -o  plab4 plab4.o -lpthread
