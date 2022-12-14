# CSC4005 Project 3 
<br/>
<br/>

# Workflow & Logger & Reproduce

## Workflow

Simulation with a large amount of bodies is impossible on your own VM, so it is necessay to simulate on cluster and then transfer the data back to your VM for visualization. We have implemented 2 utilities to help you do that.

Also, for CUDA implementation, there is no CUDA on our VM, so you can use cluster to generate simulation results first, then you can transfer the results back to your VM. You can visualize the results on your VM.

## Logger

We provide a utility called `Logger` at `headers/checkpoint.h`, it can save `x,y` coordinates of multiple frames. Result will be stored in `./checkpoints/xxxxxxx/`. 

Here we give a sample use:

```c++
Logger l = Logger("cuda", 10000, 4000, 4000);
for (int i = 0; i < n_iterations; i++){
    // compute x, y
    l.save_frame(x, y);
}
```


## Copy Results Back

You can use `scp` command. 

Execute the following command on your VM.

```bash
scp -r $STUDENT_ID@10.26.200.21:/nfsmnt/$STUDENT_ID/xxxxx .
```

here `-r` represent recursively copying all files within a given directory.

You will find the target directory on cluster appears in your local working directory.


## Reproduce

We have implemented `video.cpp` to help you visualize simulation results generated by cluster.  

Use

```bash
g++ ./src/video.cpp -o video -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -DGUI -O2 -std=c++11
```

to compile `video` GUI application on your VM.

Once you get a checkpoint directory like `./checkpoints/sequential_1000_20221107025155` (from cluster or somewhere else), you can use

```bash
./video ./checkpoints/sequential_1000_20221107025155
```

to reproduce result with GUI on your VM. 

**So, you can visualize the output of your CUDA program!**

<br/>
<br/>
<br/>
<br/>


# Compile & Run

## Compile

Sequential (command line application):

```bash
g++ ./src/sequential.cpp -o seq -O2 -std=c++11
```

Sequential (GUI application):

```bash
g++ ./src/sequential.cpp -o seqg -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -DGUI -O2 -std=c++11
```

MPI (command line application):

```bash
mpic++ ./src/mpi.cpp -o mpi -std=c++11
```

MPI (GUI application):

```bash
mpic++ ./src/mpi.cpp -o mpig -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -DGUI -std=c++11
```

Pthread (command line application):

```bash
g++ ./src/pthread.cpp -o pthread -lpthread -O2 -std=c++11
```

Pthread (GUI application):

```bash
g++ ./src/pthread.cpp -o pthreadg -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -lpthread -DGUI -O2 -std=c++11
```

CUDA (command line application): notice that `nvcc` is not available on VM, please use cluster.

```bash
nvcc ./src/cuda.cu -o cuda -O2 --std=c++11
```

CUDA (GUI application): notice that `nvcc` is not available on VM, please use cluster.

```bash
nvcc ./src/cuda.cu -o cudag -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -O2 -DGUI --std=c++11
```


OpenMP (command line application):

```bash
g++ ./src/openmp.cpp -o openmp -fopenmp -O2 -std=c++11
```

OpenMP (GUI application):

```bash
g++ ./src/openmp.cpp -o openmpg -fopenmp -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -O2 -DGUI -std=c++11
```


MPI+OpenMP (command line application):

```bash
mpic++ ./src/mpi_openmp.cpp -o mpi_openmp -fopenmp -std=c++11
```

MPI+OpenMP (GUI application):

```bash
mpic++ ./src/mpi_openmp.cpp -o mpi_openmpg -fopenmp -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -DGUI -std=c++11
```

## Run

Sequential (command line mode):

```bash
./seq $n_body $n_iterations
```

Sequential (GUI mode): please run this on VM (with GUI desktop).

```bash
./seqg $n_body $n_iterations
```

MPI (command line mode):

```bash
mpirun -np $n_processes ./mpi $n_body $n_iterations
```

MPI (GUI mode): please run this on VM (with GUI desktop).

```bash
mpirun -np $n_processes ./mpig $n_body $n_iterations
```


Pthread (command line mode):

```bash
./pthread $n_body $n_iterations $n_threads
```

Pthread (GUI mode): please run this on VM (with GUI desktop).

```bash
./pthreadg $n_body $n_iterations $n_threads
```

CUDA (command line mode): for VM users, please run this on cluster.

```bash
./cuda $n_body $n_iterations
```

CUDA (GUI mode): if you have both nvcc and GUI desktop, you can try this.

```bash
./cuda $n_body $n_iterations
```


OpenMP (command line mode):

```bash
openmp $n_body $n_iterations $n_omp_threads
```

OpenMP (GUI mode):

```bash
openmpg $n_body $n_iterations $n_omp_threads
```


MPI+OpenMP (command line mode):

```bash
mpirun -np $n_processes ./mpi_openmp $n_body $n_iterations
```

MPI+OpenMP (GUI mode):

```bash
mpirun -np $n_processes ./mpi_openmpg $n_body $n_iterations
```

<br/>
<br/>
<br/>
