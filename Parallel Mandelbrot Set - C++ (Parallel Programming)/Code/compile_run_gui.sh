#! /bin/bash

# salloc -n20 -p Project # allocate cpu for your task

g++ -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm sequential.cpp -o seqg -DGUI -O2 -std=c++11
./seqg 800 800 100

mpic++ -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm mpi_ScatterGather.cpp -o mpi_ScatterGatherg -DGUI -std=c++11
mpirun -np 8 ./mpi_ScatterGatherg 800 800 100

mpic++ -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm mpi_SendRecv.cpp -o mpi_SendRecvg -DGUI -std=c++11
mpirun -np 8 ./mpi_SendRecvg 800 800 100

g++ -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -lpthread pthread.cpp -o pthreadg -DGUI -O2 -std=c++11
./pthreadg 800 800 100 8


# /bin/bash compile_run_gui.sh