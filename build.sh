#!/bin/bash

MAIN="transform_mipi.c"
SRC="./src/*.c"
INC="./inc"
OBJ="transform_mipi"

gcc $MAIN $SRC -o $OBJ -I $INC
