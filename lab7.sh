#!/bin/bash
gcc -c sender.c
gcc -c receiver.c
gcc -o sender sender.c -lpthread 
gcc -o receiver receiver.c -lpthread 
