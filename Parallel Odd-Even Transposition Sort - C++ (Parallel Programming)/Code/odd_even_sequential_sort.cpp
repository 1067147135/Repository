#include <cstdlib>
#include <fstream>
#include <iostream>
#include <chrono>

int main(int argc, char **argv)
{

    int num_elements;             // number of elements to be sorted
    num_elements = atoi(argv[1]); // convert command line argument to num_elements

    int elements[num_elements];        // store elements
    int sorted_elements[num_elements]; // store sorted elements

    std::ifstream input(argv[2]);
    int element;
    int i = 0;
    while (input >> element)
    {
        elements[i] = element;
        i++;
    }
    std::cout << "actual number of elements:" << i << std::endl;

    std::chrono::high_resolution_clock::time_point t1;
    std::chrono::high_resolution_clock::time_point t2;
    std::chrono::duration<double> time_span;
    t1 = std::chrono::high_resolution_clock::now(); // record time

    for (int i = 0; i < num_elements; i++)
    {
        if (i % 2)
        {
            for (int j = 0; j < num_elements; j += 2)
            {
                if (j + 1 >= num_elements)
                    break;
                if (elements[j] > elements[j + 1])
                {
                    int tmp = elements[j];
                    elements[j] = elements[j + 1];
                    elements[j + 1] = tmp;
                }
            }
        }
        else
        {
            for (int j = 1; j < num_elements; j += 2)
            {
                if (j + 1 >= num_elements)
                    break;
                if (elements[j] > elements[j + 1])
                {
                    int tmp = elements[j];
                    elements[j] = elements[j + 1];
                    elements[j + 1] = tmp;
                }
            }
        }
    }

    for (int i = 0; i < num_elements; i++)
    {
        sorted_elements[i] = elements[i];
    }
    /* TODO END */

    t2 = std::chrono::high_resolution_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    std::cout << "Student ID: "
              << "119010265" << std::endl; // replace it with your student id
    std::cout << "Name: "
              << "Shi Wenlan" << std::endl; // replace it with your name
    std::cout << "Assignment 1, Sequential version" << std::endl;
    std::cout << "Run Time: " << time_span.count() << " seconds" << std::endl;
    std::cout << "Input Size: " << num_elements << std::endl;
    std::cout << "Process Number: " << 1 << std::endl;

    std::ofstream output(argv[2] + std::string(".seq.out"), std::ios_base::out);
    for (int i = 0; i < num_elements; i++)
    {
        output << sorted_elements[i] << std::endl;
    }

    return 0;
}