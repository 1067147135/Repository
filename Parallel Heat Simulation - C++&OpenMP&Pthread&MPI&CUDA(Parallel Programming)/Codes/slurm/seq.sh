#!/bin/bash

#SBATCH --job-name Sequential           # Job name
#SBATCH --output seq.out
#SBATCH --nodes=1                       # Run all processes on a single node	
#SBATCH --ntasks=1                      # number of processes = 1 
#SBATCH --cpus-per-task=1               # Number of CPU cores allocated to each process
#SBATCH --partition=Project             # Partition name: Project or Debug (Debug is default)
#SBATCH --priority 10

# cd /nfsmnt/119010265/project4/
g++ ./src/sequential.cpp -o seq -O2 -std=c++11
for size in 200 1000 5000
do
    ./seq $size
done