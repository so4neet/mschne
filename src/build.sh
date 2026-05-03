#!/bin/bash

debug=0
cTarget="mschne"
cObjs=$(find . -type f -name "*.c")
cFlags="-g -shared -fdeclspec -fPIC"
lFlags="-lraylib"

# Get launch flags

while getopts d flag
do
        case "${flag}" in
                d) debug=1;;
        esac
done

echo "Building MSCHNE"
if [ "$debug" -eq 1 ]; then
        echo "Debug Build Enabled"
fi

# Build stuff 
clang $cObjs $cFlags -o ../build/lib$cTarget.so $lFlags