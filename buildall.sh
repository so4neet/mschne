#!/bin/bash
if [ "$1" == "-d" ]; then
        cd src
        ./build.sh -d
        cd ..
else
        cd src
        ./build.sh
        cd ..
fi
cd test
./build.sh
cd ..