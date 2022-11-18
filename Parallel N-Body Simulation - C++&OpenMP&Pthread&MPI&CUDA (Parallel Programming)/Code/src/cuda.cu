#include <cuda.h>
#include <cuda_runtime.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <vector>
#include <utility>
#ifdef GUI
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "./headers/physics_cuda.h"
#include "./headers/checkpoint.h"

int block_size = 512;

int n_body;
int n_iteration;

std::chrono::duration<double> total_time;

void generate_data(double *m, double *x, double *y, double *vx, double *vy, int n)
{
    // TODO: Generate proper initial position and mass for better visualization
    for (int i = 0; i < n; i++)
    {
        m[i] = rand() % max_mass + 1.0f;
        // Leave marginal space to make the starting position of the ball closer,
        // so as to facilitate observation of the ball movement.
        x[i] = rand() % (bound_x / 2) + (bound_x / 4);
        y[i] = rand() % (bound_y / 2) + (bound_y / 4);
        vx[i] = 0.0f;
        vy[i] = 0.0f;
    }
}

__device__ double collision(double v1, double v2, double m1, double m2)
{
    return ((m1 - m2) * v1 + 2 * m2 * v2) / (m1 + m2);
}

__global__ void update_position(double *x, double *y, double *vx, double *vy, double* new_vx, double* new_vy, int n)
{
    // TODO: update position
    int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i < n)
    {
        vx[i] = new_vx[i];
        vy[i] = new_vy[i];
        x[i] += vx[i] * dt_d;
        y[i] += vy[i] * dt_d;
    }
}



__global__ void update_velocity(double *m, double *x, double *y, double *vx, double *vy, double* new_vx, double* new_vy, int n)
{
    // TODO: calculate force and acceleration, update velocity
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    // printf("blockDim.x = %d, blockIdx.x = %d, threadIdx.x = %d\n", blockDim.x, blockIdx.x, threadIdx.x);

    if (i < n)
    {
        // printf("Thread %d get x[%d] = %f\n", threadIdx.x, i, x[threadIdx.x]);
        double ax = 0;
        double ay = 0;
        new_vx[i] = vx[i];
        new_vy[i] = vy[i];
        for (int j = 0; j < n; j++)
        {
            if (i == j)
                continue;
            double dx = x[j] - x[i];
            double dy = y[j] - y[i];
            double r = sqrt(dx * dx + dy * dy);
            // if (i == 0) printf("Device %d: calculate distance from %d to %d: dx = %f, dy = %f, r = %f\n", threadIdx.x, i, j, dx, dy, r);

            if (r < radius1_d)
            {
                double dax = gravity_const_d * m[j] * dx / ((r + error_d) * (r + error_d) * (r + error_d));
                double day = gravity_const_d * m[j] * dy / ((r + error_d) * (r + error_d) * (r + error_d));
                ax += dax;
                ay += day;
                // if (i == 0) printf("Device %d: calculate acc_x = %f * %f * %f / (%f + %f)^3 = %f\n", threadIdx.x, gravity_const_d, m[j], dx, r, error_d, dax);
                // if (i == 0) printf("Device %d: calculate acc_y = %f * %f * %f / (%f + %f)^3 = %f\n", threadIdx.x, gravity_const_d, m[j], dy, r, error_d, day);
            }
            else
            {
                double dax = gravity_const_d * m[j] * dx / (r * r * r);
                double day = gravity_const_d * m[j] * dy / (r * r * r);
                ax += dax;
                ay += day;
                // if (i == 0) printf("Device %d: calculate acc from %d to %d: dax = %f, day = %f\n", threadIdx.x, i, j, dax, day);
            }

            if (r < radius2_d)
            { // collision
                double v1x = new_vx[i];
                double v1y = new_vy[i];
                double v2x = vx[j];
                double v2y = vy[j];
                new_vx[i] = collision(v1x, v2x, m[i], m[j]);
                new_vy[i] = collision(v1y, v2y, m[i], m[j]);
                // if (i == 0) printf("Device %d: collision from %d to %d: v1y = %f, v11y = %f\n", threadIdx.x, i, j, v1y, new_vy[threadIdx.x]);
            }
        }
        // if (i == 0) printf("Work item %d calculated ax = %f, ay = %f\n", i, ax[threadIdx.x], ay[threadIdx.x]);

        if (x[i] <= 0 || x[i] >= bound_x_d)
        {
            new_vx[i] = -new_vx[i];
        }
        new_vx[i] += ax * dt_d;
        // Set the extreme seep to make the ball's motion visible.
        if (new_vx[i] > 20000000.0f)
            new_vx[i] = 20000000.0f;
        else if (new_vx[i] < -20000000.0f)
            new_vx[i] = -20000000.0f;

        if (y[i] <= 0 || y[i] >= bound_y_d)
        {
            new_vy[i] = -new_vy[i];
        }
        new_vy[i] += ay * dt_d;
        // Set the extreme seep to make the ball's motion visible.
        if (new_vy[i] > 20000000.0f)
            new_vy[i] = 20000000.0f;
        else if (new_vy[i] < -20000000.0f)
            new_vy[i] = -20000000.0f;
        // if (i == 0) printf("Work item %d calculated new_vx = %f, new_vy = %f\n", i, new_vx[threadIdx.x], new_vy[threadIdx.x]);
    }
}

void master()
{
    double *m = new double[n_body];
    double *x = new double[n_body];
    double *y = new double[n_body];
    double *vx = new double[n_body];
    double *vy = new double[n_body];

    generate_data(m, x, y, vx, vy, n_body);

    Logger l = Logger("cuda", n_body, bound_x, bound_y);

    double *device_m;
    double *device_x;
    double *device_y;
    double *device_vx;
    double *device_vy;

    double *device_new_vx;
    double *device_new_vy;

    cudaMalloc(&device_m, n_body * sizeof(double));
    cudaMalloc(&device_x, n_body * sizeof(double));
    cudaMalloc(&device_y, n_body * sizeof(double));
    cudaMalloc(&device_vx, n_body * sizeof(double));
    cudaMalloc(&device_vy, n_body * sizeof(double));
    cudaMalloc(&device_new_vx, n_body * sizeof(double));
    cudaMalloc(&device_new_vy, n_body * sizeof(double));

    cudaMemcpy(device_m, m, n_body * sizeof(double), cudaMemcpyHostToDevice);
    cudaMemcpy(device_x, x, n_body * sizeof(double), cudaMemcpyHostToDevice);
    cudaMemcpy(device_y, y, n_body * sizeof(double), cudaMemcpyHostToDevice);
    cudaMemcpy(device_vx, vx, n_body * sizeof(double), cudaMemcpyHostToDevice);
    cudaMemcpy(device_vy, vy, n_body * sizeof(double), cudaMemcpyHostToDevice);

    int n_block = n_body / block_size + 1;

    for (int i = 0; i < n_iteration; i++)
    {
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

        update_velocity<<<n_block, block_size>>>(device_m, device_x, device_y, device_vx, device_vy, device_new_vx, device_new_vy, n_body);
        update_position<<<n_block, block_size>>>(device_x, device_y, device_vx, device_vy, device_new_vx, device_new_vy, n_body);

        cudaMemcpy(x, device_x, n_body * sizeof(double), cudaMemcpyDeviceToHost);
        cudaMemcpy(y, device_y, n_body * sizeof(double), cudaMemcpyDeviceToHost);

        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = t2 - t1;
        total_time += time_span;

        l.save_frame(x, y);

        // printf("Iteration %d, elapsed time: %.3f\n", i, time_span);

#ifdef GUI
        glClear(GL_COLOR_BUFFER_BIT);
        glColor3f(1.0f, 0.0f, 0.0f);
        glPointSize(2.0f);
        glBegin(GL_POINTS);
        double xi;
        double yi;
        for (int i = 0; i < n_body; i++)
        {
            xi = x[i];
            yi = y[i];
            glVertex2f(xi, yi);
        }
        glEnd();
        glFlush();
        glutSwapBuffers();
#else

#endif
    }

    cudaFree(device_m);
    cudaFree(device_x);
    cudaFree(device_y);
    cudaFree(device_vx);
    cudaFree(device_vy);
    cudaFree(device_new_vx);
    cudaFree(device_new_vy);

    delete m;
    delete x;
    delete y;
    delete vx;
    delete vy;
}

int main(int argc, char *argv[])
{

    n_body = atoi(argv[1]);
    n_iteration = atoi(argv[2]);

#ifdef GUI
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(800, 800);
    glutCreateWindow("N Body Simulation CUDA Implementation");
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gluOrtho2D(0, bound_x, 0, bound_y);
#endif

    master();

    printf("Student ID: 119010265\n"); // replace it with your student id
    printf("Name: SHI Wenlan\n");      // replace it with your name
    printf("Assignment 3: N Body Simulation CUDA Implementation\n");
    printf("n_body = %d, n_iteration = %d\n", n_body, n_iteration);
    printf("Used time: %.3f\n", total_time);

    return 0;
}
