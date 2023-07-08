#!/bin/bash

g++ ./src/sequential.cpp -o seqg -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -DGUI -O2 -std=c++11
./seqg 1000

mpic++ ./src/mpi_sendrecv.cpp -o mpi_sendrecvg -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -DGUI -std=c++11
mpirun -np 4 ./mpi_sendrecvg 1000

mpic++ ./src/mpi_broadcast.cpp -o mpi_broadcastg -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -DGUI -std=c++11
mpirun -np 4 ./mpi_broadcastg 1000

g++ ./src/pthread.cpp -o pthreadg -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -lpthread -DGUI -O2 -std=c++11
./pthreadg 1000 4

g++ ./src/openmp.cpp -o openmpg -fopenmp -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -O2 -DGUI -std=c++11
./openmpg 1000 4

mpic++ ./src/mpi_openmp.cpp -o mpi_openmpg -fopenmp -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -DGUI -std=c++11
mpirun -np 2 ./mpi_openmpg 1000 2

nvcc ./src/cuda.cu -o cudag -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -O2 -DGUI --std=c++11
./cudag 1000