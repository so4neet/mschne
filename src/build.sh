#!/bin/bash

suppress=0
cTarget="mschne"
cObjs=$(find . -type f -name "*.c")
cFlags="-g -shared -fdeclspec -fPIC"
lFlags="-lSDL3"

# Get launch flags

while getopts d flag
do
        case "${flag}" in
                d) suppress=1;;
        esac
done 

echo "Building MSCHNE"
if [ "$suppress" -eq 1 ]; then
        echo "Suppressing debug messages"
        clang $cObjs $cFlags -o ../build/lib$cTarget.so $lFlags -DMSUPPRESS
else
        clang $cObjs $cFlags -o ../build/lib$cTarget.so $lFlags
fi