#!/bin/bash

# compile program
CC="g++"
CFLAGS="-Wall -O3"
LDFLAGS="-lSDL2"
${CC} main.cpp ${CFLAGS} ${LDFLAGS} -o audio

# start program
./audio
