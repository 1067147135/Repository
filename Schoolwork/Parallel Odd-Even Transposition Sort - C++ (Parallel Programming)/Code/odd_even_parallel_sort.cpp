#include <mpi.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <chrono>

void separate(int num_numbers, int num_threads, int res[])
{
    int q = num_numbers / num_threads;
    int r = num_numbers % num_threads;
    int i = 0;
    for (; i < r; i++)
    {
        res[i] = q + 1;
    }
    for (; i < num_threads; i++)
    {
        res[i] = q;
    }
}

int main(int argc, char **argv)
{

    MPI_Init(&argc, &argv); // initialize parallel programming environment

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // get the rank of this process

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // get the number of processes

    int num_elements; // number of elements to be sorted

    num_elements = atoi(argv[1]); // convert command line argument to num_elements

    int elements[num_elements];        // store elements
    int sorted_elements[num_elements]; // store sorted elements

    if (rank == 0)
    { // read inputs from file (master process)
        std::ifstream input(argv[2]);
        int element;
        int i = 0;
        while (input >> element)
        {
            elements[i] = element;
            i++;
        }
        std::cout << "actual number of elements:" << i << std::endl;
    }

    /* TODO BEGIN
        Implement parallel odd even transposition sort
        Code in this block is not a necessary.
        Replace it with your own code.
        Useful MPI documentation: https://rookiehpc.github.io/mpi/docs
    */

    std::chrono::high_resolution_clock::time_point t1;
    std::chrono::high_resolution_clock::time_point t2;
    std::chrono::duration<double> time_span;
    if (rank == 0)
    {
        t1 = std::chrono::high_resolution_clock::now(); // record time
    }

    int sep[world_size];
    int disp[world_size];
    separate(num_elements, world_size, sep);
    int sum = 0;
    for (int i = 0; i < world_size; i++)
    {
        disp[i] = sum;
        sum += sep[i];
    }

    int num_my_element = sep[rank]; // number of elements allocated to each process
    int my_element[num_my_element];

    MPI_Scatterv(elements, sep, disp, MPI_INT, my_element, num_my_element, MPI_INT, 0, MPI_COMM_WORLD); // distribute elements to each process

    int start = 0; // section start point
    for (int i = 0; i < rank; i++)
    {
        start += sep[i];
    }
    int s_odd = 0;
    int s_even = 0;
    if (start % 2 != 0)
        s_odd = 1;
    else
        s_even = 1; // store elements of each process

    // printf("MPI process %d has start = %d, s_odd = %d, s_even = %d\n", rank, start, s_odd, s_even);

    for (int i = 0; i < num_elements; i++)
    {
        // printf("MPI process %d in iteration %d\n", rank, i);
        if (i % 2)
        {
            bool send_e = false;
            MPI_Request request_send_s;
            MPI_Request request_send_e;
            if (s_odd == 1 && rank > 0)
            {
                MPI_Isend(&my_element, 1, MPI_INT, rank - 1, i * 2, MPI_COMM_WORLD, &request_send_s);
                // printf("MPI process %d sends value %d to MPI process %d with tag %d.\n", rank, my_element[0], rank - 1, i * 2);
            }
            for (int j = s_odd; j < num_my_element; j += 2)
            {
                if (j + 1 >= num_my_element && rank >= world_size - 1)
                    break;
                else if (j + 1 >= num_my_element && rank < world_size - 1)
                {
                    int received;
                    MPI_Recv(&received, 1, MPI_INT, rank + 1, i * 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    // printf("MPI process %d received value %d from MPI process %d with tag %d.\n", rank, received, rank + 1, i * 2);
                    if (received >= my_element[j])
                    {
                        MPI_Isend(&received, 1, MPI_INT, rank + 1, i * 2 + 1, MPI_COMM_WORLD, &request_send_e);
                        // printf("MPI process %d sends value %d to MPI process %d with tag %d.\n", rank, received, rank + 1, i * 2 + 1);
                    }
                    else
                    {
                        MPI_Isend(&my_element[j], 1, MPI_INT, rank + 1, i * 2 + 1, MPI_COMM_WORLD, &request_send_e);
                        // printf("MPI process %d sends value %d to MPI process %d with tag %d.\n", rank, my_element[j], rank + 1, i * 2 + 1);
                        my_element[j] = received;
                    }
                    send_e = true;
                }
                else if (my_element[j] > my_element[j + 1])
                {
                    int tmp = my_element[j];
                    my_element[j] = my_element[j + 1];
                    my_element[j + 1] = tmp;
                }
            }
            if (send_e)
            {
                MPI_Wait(&request_send_e, MPI_STATUS_IGNORE);
            }
            if (s_odd == 1 && rank > 0)
            {
                MPI_Wait(&request_send_s, MPI_STATUS_IGNORE);
                int received;
                MPI_Recv(&received, 1, MPI_INT, rank - 1, i * 2 + 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                // printf("MPI process %d received value %d from MPI process %d with tag %d.\n", rank, received, rank - 1, i * 2 + 1);
                my_element[0] = received;
            }
        }
        else
        {
            bool send_e = false;
            MPI_Request request_send_s;
            MPI_Request request_send_e;
            if (s_even == 1 && rank > 0)
            {
                MPI_Isend(&my_element, 1, MPI_INT, rank - 1, i * 2, MPI_COMM_WORLD, &request_send_s);
                // printf("MPI process %d sends value %d to MPI process %d with tag %d.\n", rank, my_element[0], rank - 1, i * 2);
            }
            for (int j = s_even; j < num_my_element; j += 2)
            {
                if (j + 1 >= num_my_element && rank >= world_size - 1)
                    break;
                else if (j + 1 >= num_my_element && rank < world_size - 1)
                {
                    int received;
                    MPI_Recv(&received, 1, MPI_INT, rank + 1, i * 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    // printf("MPI process %d received value %d from MPI process %d with tag %d.\n", rank, received, rank + 1, i * 21);
                    if (received >= my_element[j])
                    {
                        MPI_Isend(&received, 1, MPI_INT, rank + 1, i * 2 + 1, MPI_COMM_WORLD, &request_send_e);
                        // printf("MPI process %d sends value %d to MPI process %d with tag %d.\n", rank, received, rank + 1, i * 2 + 1);
                    }
                    else
                    {
                        MPI_Isend(&my_element[j], 1, MPI_INT, rank + 1, i * 2 + 1, MPI_COMM_WORLD, &request_send_e);
                        // printf("MPI process %d sends value %d to MPI process %d with tag %d.\n", rank, my_element[j], rank + 1, i * 2 + 1);
                        my_element[j] = received;
                    }
                    send_e = true;
                }
                if (my_element[j] > my_element[j + 1])
                {
                    int tmp = my_element[j];
                    my_element[j] = my_element[j + 1];
                    my_element[j + 1] = tmp;
                }
            }
            if (send_e)
            {
                MPI_Wait(&request_send_e, MPI_STATUS_IGNORE);
            }
            if (s_even == 1 && rank > 0)
            {
                MPI_Wait(&request_send_s, MPI_STATUS_IGNORE);
                int received;
                MPI_Recv(&received, 1, MPI_INT, rank - 1, i * 2 + 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                // printf("MPI process %d received value %d from MPI process %d with tag %d.\n", rank, received, rank - 1, i * 2 + 1);
                my_element[0] = received;
            }
        }
    }

    MPI_Gatherv(my_element, num_my_element, MPI_INT, sorted_elements, sep, disp, MPI_INT, 0, MPI_COMM_WORLD); // collect result from each process

    /* TODO END */

    if (rank == 0)
    { // record time (only executed in master process)
        t2 = std::chrono::high_resolution_clock::now();
        time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        std::cout << "Student ID: "
                  << "119010265" << std::endl; // replace it with your student id
        std::cout << "Name: "
                  << "Shi Wenlan" << std::endl; // replace it with your name
        std::cout << "Assignment 1, Parallel version, MPI implementation" << std::endl;
        std::cout << "Run Time: " << time_span.count() << " seconds" << std::endl;
        std::cout << "Input Size: " << num_elements << std::endl;
        std::cout << "Process Number: " << world_size << std::endl;
    }

    if (rank == 0)
    { // write result to file (only executed in master process)
        std::ofstream output(argv[2] + std::string(".parallel.out"), std::ios_base::out);
        for (int i = 0; i < num_elements; i++)
        {
            output << sorted_elements[i] << std::endl;
        }
    }

    MPI_Finalize(); // exit MPI system, only sequential code can run rank 0, no more MPI function

    return 0;
}
