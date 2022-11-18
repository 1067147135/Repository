#!/bin/bash

#SBATCH --job-name=MPI_OpenMP       # Job name
#SBATCH --output mpi_openmp.out
#SBATCH --nodes=1                   # Run all processes on a single node	
#SBATCH --ntasks=17                 # number of processes
#SBATCH --cpus-per-task=2           # Number of CPU cores allocated to each process
#SBATCH --partition=Project         # Partition name: Project or Debug (Debug is default)
#SBATCH --priority 10

# cd /nfsmnt/119010265/project3/
mpic++ ./src/mpi_openmp.cpp -o mpi_openmp -fopenmp -std=c++11
for size in 200 1000 2000
do
    for cores in 2 3 5 9 13 17
    do
        mpirun -np $cores ./mpi_openmp $size 100
    done
done