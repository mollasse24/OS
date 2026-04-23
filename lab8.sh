#!/bin/bash
gcc -c s.c
gcc -c client.c
gcc -o s s.c -lpthread 
gcc -o client client.c -lpthread 
