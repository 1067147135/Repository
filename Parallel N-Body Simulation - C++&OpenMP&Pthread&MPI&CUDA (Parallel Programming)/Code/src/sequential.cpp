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

#include "./headers/physics.h"
#include "./headers/checkpoint.h"

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

double collision(double v1, double v2, double m1, double m2)
{
    return ((m1 - m2) * v1 + 2 * m2 * v2) / (m1 + m2);
}

void update_position(double *x, double *y, double *vx, double *vy, double *new_vx, double *new_vy, int n)
{
    // TODO: update position
    for (int i = 0; i < n; i++)
    {
        vx[i] = new_vx[i];
        vy[i] = new_vy[i];

        x[i] += vx[i] * dt;
        y[i] += vy[i] * dt;
        // printf("x[%d] = %f, y[%d] = %f\n", i, x[i], i, y[i]);
    }
}


void update_velocity(double *m, double *x, double *y, double *vx, double *vy, double *new_vx, double *new_vy, int n)
{
    // TODO: calculate acceleration, update velocity
    //  Compute Accelerations
    for (int i = 0; i < n; i++)
    {
        double ax = 0;
        double ay = 0;
        new_vx[i] = vx[i];
        new_vy[i] = vy[i];
        for (int j = 0; j < n_body; j++)
        {
            if (i == j)
                continue;
            double dx = x[j] - x[i];
            double dy = y[j] - y[i];
            double r = sqrt(dx * dx + dy * dy);
            if (r < radius1)
            {
                double dax = gravity_const * m[j] * dx / pow(r + error, 3);
                double day = gravity_const * m[j] * dy / pow(r + error, 3);
                ax += dax;
                ay += day;
            }
            else
            {
                double dax = gravity_const * m[j] * dx / pow(r, 3);
                double day = gravity_const * m[j] * dy / pow(r, 3);
                ax += dax;
                ay += day;
            }
            if (r < radius2)
            { // collision
                double v1x = new_vx[i];
                double v1y = new_vy[i];
                double v2x = vx[j];
                double v2y = vy[j];
                new_vx[i] = collision(v1x, v2x, m[i], m[j]);
                new_vy[i] = collision(v1y, v2y, m[i], m[j]);
            }
            // calculate collisions with borders to get the final velocity
            if (x[i] <= 0 || x[i] >= bound_x)
            {
                new_vx[i] = -new_vx[i];
            }
            new_vx[i] += ax * dt;
            // Set the extreme seep to make the ball's motion visible.
            if (new_vx[i] > 20000000.0f)
                new_vx[i] = 20000000.0f;
            else if (new_vx[i] < -20000000.0f)
                new_vx[i] = -20000000.0f;

            if (y[i] <= 0 || y[i] >= bound_y)
            {
                new_vy[i] = -new_vy[i];
            }
            new_vy[i] += ay * dt;
            // Set the extreme seep to make the ball's motion visible.
            if (new_vy[i] > 20000000.0f)
                new_vy[i] = 20000000.0f;
            else if (new_vy[i] < -20000000.0f)
                new_vy[i] = -20000000.0f;
        }
    }
}

void master()
{
    double *m = new double[n_body];
    double *x = new double[n_body];
    double *y = new double[n_body];
    double *vx = new double[n_body];
    double *vy = new double[n_body];

    double *new_vx = new double[n_body];
    double *new_vy = new double[n_body];

    generate_data(m, x, y, vx, vy, n_body);

    Logger l = Logger("sequential", n_body, bound_x, bound_y);

    for (int i = 0; i < n_iteration; i++)
    {
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

        update_velocity(m, x, y, vx, vy, new_vx, new_vy, n_body);

        update_position(x, y, vx, vy, new_vx, new_vy, n_body);

        l.save_frame(x, y);

        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = t2 - t1;

        // printf("Iteration %d, elapsed time: %.3f\n", i, time_span);
        total_time += time_span;

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
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(0, 0);

    glutCreateWindow("N Body Simulation Sequential Implementation");
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gluOrtho2D(0, bound_x, 0, bound_y);
#endif
    master();

    printf("Student ID: 119010265\n"); // replace it with your student id
    printf("Name: SHI Wenlan\n");      // replace it with your name
    printf("Assignment 3: N Body Simulation Sequential Implementation\n");
    printf("Used time: %.3f\n", total_time);

    return 0;
}
