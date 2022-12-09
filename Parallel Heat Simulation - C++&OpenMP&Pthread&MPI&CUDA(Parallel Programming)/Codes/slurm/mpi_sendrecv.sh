#!/bin/bash

#SBATCH --job-name MPI              # Job name
#SBATCH --output mpi.out
#SBATCH --nodes=1                   # Run all processes on a single node	
#SBATCH --ntasks=37                 # number of processes
#SBATCH --cpus-per-task=1           # Number of CPU cores allocated to each process (please use 1 here, in comparison with pthread)
#SBATCH --partition=Project         # Partition name: Project or Debug (Debug is default)
#SBATCH --priority 10

# cd /nfsmnt/119010265/project4/
mpic++ ./src/mpi_sendrecv.cpp -o mpi_sendrecv -std=c++11
for size in 200 1000 5000
do
    for cores in 2 3 5 9 13 17 21 25 29 33 37
    do
        mpirun -np $cores ./mpi_sendrecv $size 
    done
done