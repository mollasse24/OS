#!/bin/bash
gcc -c reader.c
gcc -c writer.c
gcc -o reader reader.c -lpthread 
gcc -o writer writer.c -lpthread 
