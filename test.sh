#!/bin/sh

gcc -Wall -ansi -o ass2 ass2.c

if [ "$2" == "debug" ]; then
    ./ass2 < test${1}.txt 2>&1 >/dev/null | less
else
    ./ass2 < test${1}.txt > ass2-test${1}.txt
    nvim -d ass2-test${1}.txt test${1}-out-mac.txt
fi
