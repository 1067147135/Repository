#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <time.h>
#include <unistd.h>
#include <algorithm>
#include <omp.h>

#ifdef GUI
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif


#include "./headers/physics.h"


int size; // problem size
int n_omp_threads;

void initialize(float *data, bool* fire_area) {
    // intialize the temperature distribution
    int len = size * size;
    for (int i = 0; i < len; i++){
        if (fire_area[i]) data[i] = fire_temp;
        else data[i] = wall_temp;
    }
}


void generate_fire_area(bool *fire_area){
    // generate the fire area
    int len = size * size;
    for (int i = 0; i < len; i++) {
        fire_area[i] = 0;
    }

    float fire1_r2 = fire_size * fire_size;
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            int a = i - size / 2;
            int b = j - size / 2;
            int r2 = 0.5 * a * a + 0.8 * b * b - 0.5 * a * b;
            if (r2 < fire1_r2) fire_area[i * size + j] = 1;
        }
    }

    float fire2_r2 = (fire_size / 2) * (fire_size / 2);
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            int a = i - 1 * size / 3;
            int b = j - 1 * size / 3;
            int r2 = a * a + b * b;
            if (r2 < fire2_r2) fire_area[i * size + j] = 1;
        }
    }
}


void update(float *data, float *new_data, bool *fire_area) {
    // update the temperature of each point, and store the result in `new_data` to avoid data racing
    #pragma omp parallel for schedule(guided)
    for (int i = 1; i < (size - 1); i++){
        for (int j = 1; j < (size - 1); j++){
            int idx = i * size + j;
            if (fire_area[idx]) continue;
            float up = data[idx - size];
            float down = data[idx + size];
            float left = data[idx - 1];
            float right = data[idx + 1];
            float new_val = (up + down + left + right) / 4;
            new_data[idx] = new_val;
        }
    }
}

bool check_continue(float *data, float *new_data){
    // TODO: determine if we should stop (because the temperature distribution will finally converge)
    int len = size * size;
    for (int i = 0; i < len; i++){
        float gap = std::max(data[i] - new_data[i], new_data[i] - data[i]);
        // printf("index = %d, gap = %f\n", i, gap);
        if (gap > threshold) return true;
    }
    return false;
}

#ifdef GUI
void data2pixels(float *data, GLubyte *pixels){
    // convert rawdata (large, size^2) to pixels (small, resolution^2) for faster rendering speed
    float factor_data_pixel = (float) size / resolution;
    float factor_temp_color = (float) 255 / (fire_temp - wall_temp);
    for (int x = 0; x < resolution; x++){
        for (int y = 0; y < resolution; y++){
            int idx = x * resolution + y;
            int idx_pixel = idx * 3;
            int x_raw = x * factor_data_pixel;
            int y_raw = y * factor_data_pixel;
            int idx_raw = y_raw * size + x_raw;
            float temp = data[idx_raw];
            int color =  ((int) temp / 5 * 5 - wall_temp) * factor_temp_color;
            pixels[idx_pixel] = color;
            pixels[idx_pixel + 1] = 255 - color;
            pixels[idx_pixel + 2] = 255 - color;
        }
    }
}


void plot(GLubyte* pixels){
    // visualize temprature distribution
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels(resolution, resolution, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glutSwapBuffers();
}
#endif


void master(){

    float* data_odd = new float[size * size];
    float* data_even = new float[size * size];
    bool* fire_area = new bool[size * size];


    #ifdef GUI
    GLubyte* pixels = new GLubyte[resolution * resolution * 3];
    #endif


    generate_fire_area(fire_area);
    initialize(data_odd, fire_area);
    initialize(data_even, fire_area);

    // bool cont = true;
    int count = 1;
    double total_time = 0;

    omp_set_num_threads(n_omp_threads);

    // while (cont) {
    for (int c = 0; c < 300; c++) {
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

        if (count % 2 == 1) {
            update(data_odd, data_even, fire_area);
        } else {
            update(data_even, data_odd, fire_area);
        }
        
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        double this_time = std::chrono::duration<double>(t2 - t1).count();
        total_time += this_time;
        // printf("Iteration %d, elapsed time: %.6f\n", count, this_time);
        count++;

        // cont = check_continue(data_odd, data_even);

        #ifdef GUI
        if (count % 2 == 1) {
            data2pixels(data_even, pixels);
        } else {
            data2pixels(data_odd, pixels);
        }
        plot(pixels);
        #endif
        
    }

    printf("Converge after %d iterations, elapsed time: %.6f, average computation time: %.6f\n", count-1, total_time, (double) total_time / (count-1));

    delete[] data_odd;
    delete[] data_even;
    delete[] fire_area;

    #ifdef GUI
    delete[] pixels;
    #endif
  
}




int main(int argc, char* argv[]) {
    size = atoi(argv[1]);
    n_omp_threads = atoi(argv[2]);

    #ifdef GUI
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(resolution, resolution);
    glutCreateWindow("Heat Distribution Simulation OpenMP Implementation");
    gluOrtho2D(0, resolution, 0, resolution);
    #endif

    master();

    printf("size = %d, n_omp_threads = %d\n", size, n_omp_threads);
    printf("Student ID: 119010265\n"); // replace it with your student id
    printf("Name: SHI Wenlan\n"); // replace it with your name
    printf("Assignment 4: Heat Distribution OpenMP Implementation\n");

    return 0;
}
