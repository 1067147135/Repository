#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <pthread.h>
#include <vector>
#include <utility>
#ifdef GUI
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "./headers/physics.h"
#include "./headers/checkpoint.h"

int n_thd; // number of threads

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

void update_position(double *x, double *y, double *vx, double *vy, double *new_vx, double *new_vy, int i)
{
    // TODO: update position
    //  copy velocity
    vx[i] = new_vx[i];
    vy[i] = new_vy[i];

    // calculate positions
    x[i] += vx[i] * dt;
    y[i] += vy[i] * dt;
}

void update_velocity(double *m, double *x, double *y, double *vx, double *vy, double *new_vx, double *new_vy, int i)
{
    // TODO: calculate force and acceleration, update velocity
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

typedef struct
{
    // TODO: specify your arguments for threads
    double *m;
    double *x;
    double *y;
    double *vx;
    double *vy;

    double *new_vx;
    double *new_vy;

    int flag; // 1 -> calculate velocity
              // 2 -> calculate position

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
    double *m = my_arg->m;
    double *x = my_arg->x;
    double *y = my_arg->y;
    double *vx = my_arg->vx;
    double *vy = my_arg->vy;

    double *new_vx = my_arg->new_vx;
    double *new_vy = my_arg->new_vy;

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
        if (my_arg->flag == 1)
        {
            for (int i = start; i < end; i++)
            {
                update_velocity(m, x, y, vx, vy, new_vx, new_vy, i);
            }
        }
        else if (my_arg->flag == 2)
        {
            for (int i = start; i < end; i++)
            {
                update_position(x, y, vx, vy, new_vx, new_vy, i);
            }
        }
        else
        {
            printf("Illegal args.\n");
        }
    }
    pthread_exit(NULL);

    // TODO END
}

void calculate_work_items(std::vector<std::pair<int, int>> &workitems)
{
    int start = 0;
    int end = 0;
    // 1 master + (world_size-1) slaves
    if (n_body < 100 * n_thd)
    { // static allocation
        int q = n_body / n_thd;
        int r = n_body % n_thd;
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
    else
    { // dynamic allocation
        while (end < n_body)
        {
            start = end;
            end += 100;
            if (end > n_body)
                end = n_body;
            workitems.push_back(std::make_pair(start, end));
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

    Logger l = Logger("pthread", n_body, bound_x, bound_y);
    pthread_t thds[n_thd];
    Args args[n_thd]; // arguments for all threads
    std::vector<std::pair<int, int>> workitems;
    calculate_work_items(workitems);
    pthread_mutex_t mutex; // mutex lock of getting workitem
    pthread_mutex_init(&mutex, NULL);
    for (int rank = 0; rank < n_thd; ++rank)
    {
        args[rank].m = m;
        args[rank].rank = rank;
        args[rank].workitems = &workitems;
        args[rank].mutex = &mutex;
        args[rank].x = x;
        args[rank].y = y;
        args[rank].vx = vx;
        args[rank].vy = vy;
        args[rank].new_vx = new_vx;
        args[rank].new_vy = new_vy;
    }
    // printf("Finished initialization\n");
    for (int i = 0; i < n_iteration; i++)
    {
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        // TODO: assign jobs
        // calculate velocity
        int progress = 0;
        for (int rank = 0; rank < n_thd; ++rank)
        {
            args[rank].flag = 1;
            args[rank].progress = &progress;
            // Send it to each rank
            pthread_create(&thds[rank], NULL, worker, &args[rank]);
            // printf("Create 1 pthread %d.\n", rank);
        }
        for (int rank = 0; rank < n_thd; rank++)
        {
            pthread_join(thds[rank], NULL);
        }

        progress = 0;
        for (int rank = 0; rank < n_thd; ++rank)
        {
            args[rank].flag = 2;
            // Send it to each rank
            pthread_create(&thds[rank], NULL, worker, &args[rank]);
            // printf("Create 2 pthread %d.\n", rank);
        }
        for (int rank = 0; rank < n_thd; rank++)
        {
            pthread_join(thds[rank], NULL);
        }
        // TODO End

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
    pthread_mutex_destroy(&mutex);
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
    n_thd = atoi(argv[3]);

#ifdef GUI
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(800, 800);
    glutCreateWindow("N Body Simulation pthread Implementation");
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gluOrtho2D(0, bound_x, 0, bound_y);
#endif
    master();

    printf("Student ID: 119010265\n"); // replace it with your student id
    printf("Name: SHI Wenlan\n");      // replace it with your name
    printf("Assignment 3: N Body Simulation pthread Implementation\n");
    printf("n_body = %d, n_iteration = %d, n_thd = %d\n", n_body, n_iteration, n_thd);
    printf("Used time: %.3f\n", total_time);

    return 0;
}
