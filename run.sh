#!/usr/bin/env bash

pgrep=$(python3 -m pip list | grep pgrep)

if [[ ! $pgrep ]];
then
    python3 -m pip install pgrep
fi
gcc -o flush -I. flush.c LinkedList.c
python3 test.py
