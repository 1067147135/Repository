#! /bin/bash

# salloc -n20 -p Project # allocate cpu for your task

g++ sequential.cpp -o seq -O2 -std=c++11
mpic++ mpi_ScatterGather.cpp -o mpi_ScatterGather -std=c++11
mpic++ mpi_SendRecv.cpp -o mpi_SendRecv -std=c++11
g++ pthread.cpp -lpthread -o pthread -O2 -std=c++11

for cores in 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
do 
    ./pthread 100 100 100 $cores
done

for RESN in 1000 5000 10000
do 
    ./seq $RESN $RESN 100
    for cores in 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
    do 
        mpirun -np $cores ./mpi_ScatterGather $RESN $RESN 100
        mpirun -np $cores ./mpi_SendRecv $RESN $RESN 100
        ./pthread $RESN $RESN 100 $cores
    done
done

# /bin/bash compile_run.sh