#include <cuda.h>
#include <cuda_runtime.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <algorithm>

#ifdef GUI
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "./headers/physics.h"


int block_size = 512; // cuda thread block size
int size; // problem size

std::chrono::duration<double> total_time;

void initialize(float* data, bool* fire_area) {
    // TODO: intialize the temperature distribution (in parallelized way)
    int len = size * size;
    for (int i = 0; i < len; i++) {
        if (fire_area[i]) data[i] = fire_temp;
        else data[i] = wall_temp;
    }
}

void generate_fire_area(bool* fire_area) {
    // TODO: generate the fire area (in parallelized way)
    int len = size * size;
    for (int i = 0; i < len; i++) {
        fire_area[i] = 0;
    }

    float fire1_r2 = fire_size * fire_size;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int a = i - size / 2;
            int b = j - size / 2;
            int r2 = 0.5 * a * a + 0.8 * b * b - 0.5 * a * b;
            if (r2 < fire1_r2) fire_area[i * size + j] = 1;
        }
    }

    float fire2_r2 = (fire_size / 2) * (fire_size / 2);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int a = i - 1 * size / 3;
            int b = j - 1 * size / 3;
            int r2 = a * a + b * b;
            if (r2 < fire2_r2) fire_area[i * size + j] = 1;
        }
    }
}

__device__ bool shouldCal(int size, int i, bool* fire_area) {
    if (i >= (size * size - size)) return false;
    if (i < size) return false;
    if (i % size == 0) return false;
    if ((i + 1) % size == 0) return false;
    if (fire_area[i]) return false;
    return true;
}

__global__ void update(float* data, float* new_data, bool* fire_area, int size) {
    // TODO: update temperature for each point  (in parallelized way)
    int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (shouldCal(size, i, fire_area)) {
        float up = data[i - size];
        float down = data[i + size];
        float left = data[i - 1];
        float right = data[i + 1];
        float new_val = (up + down + left + right) / 4;
        new_data[i] = new_val;
    }
}

bool check_continue(float* data, float* new_data) {
    // TODO: determine if we should stop (because the temperature distribution will finally converge)
    int len = size * size;
    for (int i = 0; i < len; i++) {
        float gap = std::max(data[i] - new_data[i], new_data[i] - data[i]);
        //printf("index = %d, new_data = %d, gap = %f\n", i, new_data[i], gap);
        if (gap > threshold) return true;
    }
    return false;
}

#ifdef GUI
void data2pixels(float* data, GLubyte* pixels) {
    // convert rawdata (large, size^2) to pixels (small, resolution^2) for faster rendering speed
    float factor_data_pixel = (float)size / resolution;
    float factor_temp_color = (float)255 / (fire_temp - wall_temp);
    for (int x = 0; x < resolution; x++) {
        for (int y = 0; y < resolution; y++) {
            int idx = x * resolution + y;
            int idx_pixel = idx * 3;
            int x_raw = x * factor_data_pixel;
            int y_raw = y * factor_data_pixel;
            int idx_raw = y_raw * size + x_raw;
            float temp = data[idx_raw];
            int color = ((int)temp / 5 * 5 - wall_temp) * factor_temp_color;
            pixels[idx_pixel] = color;
            pixels[idx_pixel + 1] = 255 - color;
            pixels[idx_pixel + 2] = 255 - color;
        }
    }
}


void plot(GLubyte* pixels) {
    // visualize temprature distribution
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels(resolution, resolution, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glutSwapBuffers();
}
#endif


void master() {
    float* data_odd_host = new float[size * size];
    float* data_even_host = new float[size * size];
    bool* fire_area_host = new bool[size * size];

    float* data_odd;
    float* data_even;
    bool* fire_area;

    cudaMalloc(&data_odd, size * size * sizeof(float));
    cudaMalloc(&data_even, size * size * sizeof(float));
    cudaMalloc(&fire_area, size * size * sizeof(bool));

    #ifdef GUI
    GLubyte* pixels = new GLubyte[resolution * resolution * 3];
    #endif

    generate_fire_area(fire_area_host);
    initialize(data_odd_host, fire_area_host);
    initialize(data_even_host, fire_area_host);
    //printf("After initialize, data_odd_host[1500] = %f, data_even_host[1500] = %f\n", data_odd_host[1500], data_even_host[1500]);

    cudaMemcpy(data_odd, data_odd_host, size * size * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(data_even, data_even_host, size * size * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(fire_area, fire_area_host, size * size * sizeof(bool), cudaMemcpyHostToDevice);

    // bool cont = true;
    int count = 1;
    double total_time = 0;

    int n_block_size = size * size / block_size + 1;

    // while (cont) {
    for (int c = 0; c < 300; c++){
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

        if (count % 2 == 1) {
            update << <n_block_size, block_size >> > (data_odd, data_even, fire_area, size);
        }
        else {
            update << <n_block_size, block_size >> > (data_even, data_odd, fire_area, size);
        }

        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        double this_time = std::chrono::duration<double>(t2 - t1).count();
        total_time += this_time;
        // printf("Iteration %d, elapsed time: %.6f\n", count, this_time);     

        if (count % 2 == 1) {
            cudaMemcpy(data_even_host, data_even, size * size * sizeof(float), cudaMemcpyDeviceToHost);
            //printf("After iteration %d, data_odd_host[1500] = %f, data_even_host[1500] = %f\n", count, data_odd_host[1500], data_even_host[1500]);
            #ifdef GUI
            data2pixels(data_even_host, pixels);
            #endif
        }
        else {
            cudaMemcpy(data_odd_host, data_odd, size * size * sizeof(float), cudaMemcpyDeviceToHost);
            //printf("After iteration %d, data_odd_host[1500] = %f, data_even_host[1500] = %f\n", count, data_odd_host[1500], data_even_host[1500]);
            #ifdef GUI
            data2pixels(data_odd_host, pixels);
            #endif
        }

        // cont = check_continue(data_odd_host, data_even_host);
        #ifdef GUI
        plot(pixels);
        #endif

        count++;
    }

    printf("Converge after %d iterations, elapsed time: %.6f, average computation time: %.6f\n", count - 1, total_time, (double)total_time / (count - 1));

    delete data_odd_host;
    delete data_even_host;
    delete fire_area_host;
    cudaFree(data_odd);
    cudaFree(data_even);
    cudaFree(fire_area);

    #ifdef GUI
    cudaFree(pixels);
    #endif
    
}


int main(int argc, char *argv[]){
    
    size = atoi(argv[1]);

    #ifdef GUI
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(resolution, resolution);
    glutCreateWindow("Heat Distribution Simulation CUDA Implementation");
    gluOrtho2D(0, resolution, 0, resolution);
    #endif

    master();

    printf("size = %d\n", size);
    printf("Student ID: 119010265\n"); // replace it with your student id
    printf("Name: SHI Wenlan\n"); // replace it with your name
    printf("Assignment 4: Heat Distribution CUDA Implementation\n");
    
    return 0;

}


