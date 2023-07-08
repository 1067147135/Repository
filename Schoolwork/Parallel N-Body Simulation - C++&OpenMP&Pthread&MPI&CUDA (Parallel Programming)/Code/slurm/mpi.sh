#!/bin/bash

#SBATCH --job-name=MPI.out          # Job name
#SBATCH --output mpi.out
#SBATCH --nodes=1                   # Run all processes on a single node	
#SBATCH --ntasks=33                 # number of processes
#SBATCH --cpus-per-task=1           # Number of CPU cores allocated to each process (please use 1 here, in comparison with pthread)
#SBATCH --partition=Project         # Partition name: Project or Debug (Debug is default)
#SBATCH --priority 10

# cd /nfsmnt/119010265/project3/
mpic++ ./src/mpi.cpp -o mpi -std=c++11
for size in 200 1000 2000
do
    for cores in 2 3 5 9 17 25 33
    do
        mpirun -np $cores ./mpi $size 100
    done
done