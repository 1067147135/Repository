#!/bin/bash

#SBATCH --job-name MPI_OpenMP       # Job name
#SBATCH --output mpi_openmp.out
#SBATCH --nodes=1                   # Run all processes on a single node	
#SBATCH --ntasks=10                 # number of processes
#SBATCH --cpus-per-task=4           # Number of CPU cores allocated to each process
#SBATCH --partition=Project         # Partition name: Project or Debug (Debug is default)
#SBATCH --priority 10

# cd /nfsmnt/119010265/project4/
mpic++ ./src/mpi_openmp.cpp -o mpi_openmp -fopenmp -std=c++11
for size in 200 1000 5000
do
    mpirun -np 2 ./mpi_openmp $size 1
    mpirun -np 2 ./mpi_openmp $size 2
    mpirun -np 3 ./mpi_openmp $size 2
    mpirun -np 3 ./mpi_openmp $size 4
    mpirun -np 4 ./mpi_openmp $size 4
    mpirun -np 5 ./mpi_openmp $size 4
    mpirun -np 6 ./mpi_openmp $size 4
    mpirun -np 7 ./mpi_openmp $size 4
    mpirun -np 8 ./mpi_openmp $size 4
    mpirun -np 9 ./mpi_openmp $size 4
    mpirun -np 10 ./mpi_openmp $size 4
done
# mpirun -np 2 ./mpi_openmp 1000 1