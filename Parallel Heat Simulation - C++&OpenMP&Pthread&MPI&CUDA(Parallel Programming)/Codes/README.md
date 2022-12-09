# CSC4005 Project 4 

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

MPI_Broadcast (command line application):

```bash
mpic++ ./src/mpi_broadcast.cpp -o mpi_broadcast -std=c++11
```

MPI_Broadcast (GUI application):

```bash
mpic++ ./src/mpi_broadcast.cpp -o mpi_broadcastg -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -DGUI -std=c++11
```

MPI_SendRecv (command line application):

```bash
mpic++ ./src/mpi_sendrecv.cpp -o mpi_sendrecv -std=c++11
```

MPI_SendRecv (GUI application):

```bash
mpic++ ./src/mpi_sendrecv.cpp -o mpi_sendrecvg -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -DGUI -std=c++11
```

Pthread (command line application):

```bash
g++ ./src/pthread.cpp -o pthread -lpthread -O2 -std=c++11
```

Pthread (GUI application):

```bash
g++ ./src/pthread.cpp -o pthreadg -I/usr/include -L/usr/local/lib -L/usr/lib -lglut -lGLU -lGL -lm -lpthread -DGUI -O2 -std=c++11
```

CUDA (command line application):

```bash
nvcc ./src/cuda.cu -o cuda -O2 --std=c++11
```

CUDA (GUI application): 

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
./seq $problem_size
```

Sequential (GUI mode): please run this on VM (with GUI desktop).

```bash
./seqg $problem_size
```

MPI_Broadcast (command line mode):

```bash
mpirun -np $n_processes ./mpi_broadcast $problem_size
```

MPI_Broadcast (GUI mode): please run this on VM (with GUI desktop).

```bash
mpirun -np $n_processes ./mpi_broadcastg $problem_size
```

MPI_SendRecv (command line mode):

```bash
mpirun -np $n_processes ./mpi_sendrecv $problem_size
```

MPI_SendRecv (GUI mode): please run this on VM (with GUI desktop).

```bash
mpirun -np $n_processes ./mpi_sendrecvg $problem_size
```

Pthread (command line mode):

```bash
./pthread $problem_size $n_threads
```

Pthread (GUI mode): please run this on VM (with GUI desktop).

```bash
./pthreadg $problem_size $n_threads
```

CUDA (command line mode): for VM users, please run this on cluster.

```bash
./cuda $problem_size
```

CUDA (GUI mode): if you have both nvcc and GUI desktop, you can try this.

```bash
./cuda $problem_size
```


OpenMP (command line mode):

```bash
./openmp $problem_size $n_omp_threads
```

OpenMP (GUI mode):

```bash
./openmpg $problem_size $n_omp_threads
```

MPI+OpenMP (command line mode):

```bash
mpirun -np $n_processes ./mpi_openmp $problem_size $n_omp_threads
```

MPI+OpenMP (GUI mode):

```bash
mpirun -np $n_processes ./mpi_openmpg $problem_size $n_omp_threads
```
<br/>
<br/>
<br/>

