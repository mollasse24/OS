#!/bin/bash
gcc -c parent.c
gcc -c child.c
gcc -o parent parent.c -lpthread -lseccomp
gcc -o child child.c -lpthread 
