#! /bin/bash
# rm -r build
mkdir build
cd build
cmake ../
make

./meshedit ../bzc/myCurve.bzc
./meshedit ../bez/teapot.bez
./meshedit ../dae/teapot.dae
./meshedit ../dae/beetle.dae
./meshedit ../dae/cube.dae

# /bin/bash compile_run.sh