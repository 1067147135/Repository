#! /bin/bash
mkdir Build
cd Build
cmake ../Source
make

./main

# rm -r ../Build

# /bin/bash compile_run.sh