#!/bin/bash

#SBATCH --job-name CUDA             ## Job name
#SBATCH --gres=gpu:1                ## Number of GPUs required for job execution.
#SBATCH --output cuda.out           ## filename of the output
#SBATCH --partition=Project         ## the partitions to run in (Debug or Project)
#SBATCH --ntasks=1                  ## number of tasks (analyses) to run
#SBATCH --gpus-per-task=1           ## number of gpus per task
#SBATCH --priority 10

## Compile the cuda script using the nvcc compiler
## You can compile your codes out of the script and simply srun the executable file.
# cd /nfsmnt/119010265/project3/
## Run the script
nvcc ./src/cuda.cu -o cuda -O2 --std=c++11
./cuda 200 100
./cuda 1000 100
./cuda 2000 100
