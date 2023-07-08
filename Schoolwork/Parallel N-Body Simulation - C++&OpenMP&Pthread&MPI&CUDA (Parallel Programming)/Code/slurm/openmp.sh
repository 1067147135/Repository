#!/bin/bash

#SBATCH --job-name=OpenMP           # Job name
#SBATCH --output openmp.out
#SBATCH --nodes=1                       # Run all processes on a single node	
#SBATCH --ntasks=1                      # number of processes = 1 
#SBATCH --cpus-per-task=40              # Number of CPU cores allocated to each process
#SBATCH --partition=Project             # Partition name: Project or Debug (Debug is default)
#SBATCH --priority 10

# cd /nfsmnt/119010265/project3/
g++ ./src/openmp.cpp -o openmp -fopenmp -O2 -std=c++11
for size in 200 1000 2000
do
    for threads in 1 2 4 8 16 24 32 40
    do
        ./openmp $size 100 $threads
    done
done