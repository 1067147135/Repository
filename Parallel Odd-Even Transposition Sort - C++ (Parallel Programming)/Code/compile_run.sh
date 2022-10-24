#! /bin/bash

# salloc -n20 -p Project # allocate cpu for your task

g++ -std=c++11 test_data_generator.cpp -o gen
g++ -O2 -std=c++11 odd_even_sequential_sort.cpp -o ssort
g++ -std=c++11 check_sorted.cpp -o check
mpic++ -std=c++11 odd_even_parallel_sort.cpp -o psort

./gen 500000 ./my_test/500000a.in
./ssort 500000 ./my_test/500000a.in
./check 500000 ./my_test/500000a.in.seq.out

for arg in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
do 
    mpirun -np $arg ./psort 500000 ./my_test/500000a.in
    ./check 500000 ./my_test/500000a.in.parallel.out
done

# /bin/bash compile_run.sh