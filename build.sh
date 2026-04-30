#!/bin/bash

debug=0

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

# Build stuff here once i have actual code lmao