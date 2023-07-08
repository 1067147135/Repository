#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <time.h>
#include <unistd.h>
#include <algorithm>
#include <pthread.h>
#include <vector>

#ifdef GUI
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif


#include "./headers/physics.h"


int size; // problem size
int n_thd; // number of threads

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


void update(float *data, float *new_data, bool *fire_area, int start, int end) {
    // update the temperature of each point, and store the result in `new_data` to avoid data racing
    for (int i = start; i < end; i++){
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

typedef struct
{
    // TODO: specify your arguments for threads
    float *data_old;
    float *data_new;
    bool *fire;

    std::vector<std::pair<int, int>> *workitems;
    int *progress;
    pthread_mutex_t *mutex;

    int rank;
    // TODO END
} Args;

void *worker(void *args)
{
    // TODO: procedure in each threads

    Args *my_arg = (Args *)args;
    // printf("Pthread %d start...\n", my_arg->rank);
    float *data_old = my_arg->data_old;
    float *data_new = my_arg->data_new;
    bool *fire = my_arg->fire;

    std::vector<std::pair<int, int>> *workitems = my_arg->workitems;

    while (1)
    {
        // Acquire work item
        // printf("Pthread %d begin to acquire work item...\n", my_arg->rank);
        pthread_mutex_lock(my_arg->mutex);
        // printf("progress = %d vs. workitem.size() = %d\n", *(my_arg->progress), (*workitems).size());
        if (*(my_arg->progress) >= (*workitems).size())
        {
            // printf("Pthread %d: Task finished\n", my_arg->rank);
            pthread_mutex_unlock(my_arg->mutex);
            pthread_exit(NULL);
        }
        int start = (*workitems)[*(my_arg->progress)].first;
        int end = (*workitems)[*(my_arg->progress)].second;
        *(my_arg->progress) = *(my_arg->progress) + 1;
        pthread_mutex_unlock(my_arg->mutex);
        // printf("Pthread %d get workitems from %d to %d.\n", my_arg->rank, start, end);

        // Excute work item
        update(data_old, data_new, fire, start, end);
    }
    pthread_exit(NULL);

    // TODO END
}

void calculate_work_items(std::vector<std::pair<int, int>> &workitems){
    int start = 1;
    int end = 1;
    int row = size - 2;
    if (row < 100 * n_thd){    // static allocation
        int q = row / n_thd;
        int r = row % n_thd;
        int i = 0;
        for (; i < r; i++)
        {   
            start = end;
            end += q + 1;
            workitems.push_back(std::make_pair(start, end));
        }
        for (; i < n_thd; i++)
        {
            start = end;
            end += q;
            workitems.push_back(std::make_pair(start, end));
        }
    }
    else{   // dynamic allocation
        while (end < size-1){
            start = end;
            end += 100;
            if (end > size-1) end = size-1;
            workitems.push_back(std::make_pair(start, end));
        }
    }
}

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

    pthread_t thds[n_thd];
    Args args[n_thd]; // arguments for all threads
    std::vector<std::pair<int, int>> workitems;
    calculate_work_items(workitems);
    pthread_mutex_t mutex; // mutex lock of getting workitem
    pthread_mutex_init(&mutex, NULL);
    for (int rank = 0; rank < n_thd; ++rank){
        args[rank].fire = fire_area;
        args[rank].rank = rank;
        args[rank].workitems = &workitems;
        args[rank].mutex = &mutex;
    }

    // bool cont = true;
    int count = 1;
    double total_time = 0;

    // while (cont) {
    for (int c = 0; c < 300; c++) {
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

        int progress = 0;
        for (int rank = 0; rank < n_thd; ++rank)
        {
            if (count % 2 == 1) {
                args[rank].data_old = data_odd;
                args[rank].data_new = data_even;
            }
            else {
                args[rank].data_old = data_even;
                args[rank].data_new = data_odd;
            }
            args[rank].progress = &progress;
            // Send it to each rank
            pthread_create(&thds[rank], NULL, worker, &args[rank]);
            // printf("Create 1 pthread %d.\n", rank);
        }
        for (int rank = 0; rank < n_thd; rank++)
        {
            pthread_join(thds[rank], NULL);
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

    pthread_mutex_destroy(&mutex);
    delete[] data_odd;
    delete[] data_even;
    delete[] fire_area;

    #ifdef GUI
    delete[] pixels;
    #endif
  
}




int main(int argc, char* argv[]) {
    size = atoi(argv[1]);
    n_thd = atoi(argv[2]);

    #ifdef GUI
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(resolution, resolution);
    glutCreateWindow("Heat Distribution Simulation Pthread Implementation");
    gluOrtho2D(0, resolution, 0, resolution);
    #endif

    master();

    printf("size = %d, n_thd = %d\n", size, n_thd);
    printf("Student ID: 119010265\n"); // replace it with your student id
    printf("Name: SHI Wenlan\n"); // replace it with your name
    printf("Assignment 4: Heat Distribution Pthread Implementation\n");

    return 0;
}
