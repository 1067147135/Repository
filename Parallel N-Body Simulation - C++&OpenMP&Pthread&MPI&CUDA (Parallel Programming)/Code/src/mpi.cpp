#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <mpi.h>
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

int my_rank;
int world_size;

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

void update_position(double *x, double *y, double *vx, double *vy, double *new_vx, double *new_vy, int start, int end)
{
    // TODO: update position
    for (int i = start; i < end; i++)
    {
        vx[i] = new_vx[i];
        vy[i] = new_vy[i];

        x[i] += vx[i] * dt;
        y[i] += vy[i] * dt;
        // printf("x[%d] = %f, y[%d] = %f\n", i, x[i], i, y[i]);
    }
    // printf("Slave %d: finish calculating positions from %d to %d\n", my_rank, start, end);
}

void update_velocity(double *m, double *x, double *y, double *vx, double *vy, double *new_vx, double *new_vy, int start, int end)
{
    // TODO: calculate force and acceleration, update velocity
    //  Compute Accelerations
    //  printf("Slave %d: enter update_velocity\n", my_rank);
    for (int i = start; i < end; i++)
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
    // printf("Slave %d: finish calculating final velocities from %d to %d\n", my_rank, start, end);
}

void slave()
{
    // TODO: MPI routine
    double *local_m = new double[n_body];
    double *local_x = new double[n_body];
    double *local_y = new double[n_body];
    double *local_vx = new double[n_body];
    double *local_vy = new double[n_body];
    double *new_vx = new double[n_body];
    double *new_vy = new double[n_body];
    MPI_Status status;
    MPI_Request request_send[4];

    MPI_Bcast(local_m, n_body, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    for (int i = 0; i < n_iteration; i++)
    {
        // copy global data to local
        MPI_Bcast(local_x, n_body, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(local_y, n_body, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(local_vx, n_body, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(local_vy, n_body, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        // printf("receive Bcast\n");
        int work[3];
        while (1)
        {
            MPI_Recv(work,
                     3,
                     MPI_INT,
                     0,
                     MPI_ANY_TAG,
                     MPI_COMM_WORLD,
                     &status);
            // printf("Slave %d: receive workitem from %d to %d <- master\n", my_rank, work[0], work[1]);

            if (status.MPI_TAG == 0)
            { // exit
                break;
            }
            int start = work[0];
            int end = work[1];
            int flag = work[2];
            int n_items = end - start;

            if (flag == 0)
            {
                update_velocity(local_m, local_x, local_y, local_vx, local_vy, new_vx, new_vy, start, end);
                // notify the result is ready
                MPI_Send(work,               /* message buffer */
                         3,                  /* one data item */
                         MPI_INT,            /* data item is an integer */
                         0,                  /* destination process rank */
                         status.MPI_TAG + 1, /* user chosen message tag */
                         MPI_COMM_WORLD);    /* default communicator */
                // printf("Slave %d: finish workitem from %d to %d (velocity)\n", my_rank, work[0], work[1]);
            }
            if (flag == 1)
            {
                update_position(local_x, local_y, local_vx, local_vy, new_vx, new_vy, start, end);
                // notify the result is ready
                MPI_Send(work,               /* message buffer */
                         3,                  /* one data item */
                         MPI_INT,            /* data item is an integer */
                         0,                  /* destination process rank */
                         status.MPI_TAG + 1, /* user chosen message tag */
                         MPI_COMM_WORLD);    /* default communicator */
                // printf("Slave %d: finish workitem from %d to %d (position)\n", my_rank, work[0], work[1]);

                // send the result
                // printf("Slave %d: send result local_x[start] = %f\n", my_rank, local_x[start]);
                MPI_Isend(&local_x[start],    /* message buffer */
                          n_items,            /* one data item */
                          MPI_DOUBLE,         /* data item is an integer */
                          0,                  /* destination process rank */
                          status.MPI_TAG + 2, /* user chosen message tag */
                          MPI_COMM_WORLD,     /* default communicator */
                          &request_send[0]);
                MPI_Isend(&local_y[start],    /* message buffer */
                          n_items,            /* one data item */
                          MPI_DOUBLE,         /* data item is an integer */
                          0,                  /* destination process rank */
                          status.MPI_TAG + 3, /* user chosen message tag */
                          MPI_COMM_WORLD,     /* default communicator */
                          &request_send[1]);
                MPI_Isend(&local_vx[start],   /* message buffer */
                          n_items,            /* one data item */
                          MPI_DOUBLE,         /* data item is an integer */
                          0,                  /* destination process rank */
                          status.MPI_TAG + 4, /* user chosen message tag */
                          MPI_COMM_WORLD,     /* default communicator */
                          &request_send[2]);
                MPI_Isend(&local_vy[start],   /* message buffer */
                          n_items,            /* one data item */
                          MPI_DOUBLE,         /* data item is an integer */
                          0,                  /* destination process rank */
                          status.MPI_TAG + 5, /* user chosen message tag */
                          MPI_COMM_WORLD,     /* default communicator */
                          &request_send[3]);
                MPI_Wait(&request_send[0], MPI_STATUS_IGNORE);
                MPI_Wait(&request_send[1], MPI_STATUS_IGNORE);
                MPI_Wait(&request_send[2], MPI_STATUS_IGNORE);
                MPI_Wait(&request_send[3], MPI_STATUS_IGNORE);
                // printf("Slave %d: send result of workitem from %d to %d -> master\n", my_rank, work[0], work[1]);
            }
        }
    }
    // TODO End
}

void calculate_work_items(std::vector<std::pair<int, int>> &workitems)
{
    int start = 0;
    int end = 0;
    // 1 master + (world_size-1) slaves
    int q = n_body / (world_size - 1);
    int r = n_body % (world_size - 1);
    int i = 0;
    for (; i < r; i++)
    {
        start = end;
        end += q + 1;
        workitems.push_back(std::make_pair(start, end));
    }
    for (; i < (world_size - 1); i++)
    {
        start = end;
        end += q;
        workitems.push_back(std::make_pair(start, end));
    }
}

void master()
{
    double *total_m = new double[n_body];
    double *total_x = new double[n_body];
    double *total_y = new double[n_body];
    double *total_vx = new double[n_body];
    double *total_vy = new double[n_body];

    generate_data(total_m, total_x, total_y, total_vx, total_vy, n_body);

    Logger l = Logger("MPI", n_body, bound_x, bound_y);

    std::vector<std::pair<int, int>> workitems;
    calculate_work_items(workitems);
    MPI_Status status;
    MPI_Request request_send[world_size - 1];
    MPI_Request request_recv[4];

    MPI_Bcast(total_m, n_body, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    for (int i = 0; i < n_iteration; i++)
    {
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

        // TODO: MPI routine

        MPI_Bcast(total_x, n_body, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(total_y, n_body, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(total_vx, n_body, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(total_vy, n_body, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        // printf("send Bcast\n");

        for (int rank = 1; rank < world_size; ++rank)
        {

            // {start index, end index, flag}
            int work[3] = {workitems[rank - 1].first, workitems[rank - 1].second, 0};

            // Send it to each rank
            // tag = 0: exit; tag = other: (workitem index+1) * 6 + (0~5)
            // +0: send workload; +1: notify result ready; +2~+5: send result

            MPI_Isend(work,           /* message buffer */
                      3,              /* one data item */
                      MPI_INT,        /* data item is an integer */
                      rank,           /* destination process rank */
                      rank * 6,       /* user chosen message tag */
                      MPI_COMM_WORLD, /* default communicator */
                      &request_send[rank - 1]);
            //  printf("Master: send slave %d workitem from %d to %d (velocity) with tag %d\n", rank, work[0], work[1], status.MPI_TAG);
        }
        for (int rank = 1; rank < world_size; ++rank)
        {
            MPI_Wait(&request_send[rank - 1], MPI_STATUS_IGNORE);
        }
        for (int rank = 1; rank < world_size; ++rank)
        {
            // Get notified that a result is ready
            int res[3];
            MPI_Recv(res,            /* message buffer */
                     3,              /* one data item */
                     MPI_INT,        /* of type double real */
                     MPI_ANY_SOURCE, /* receive from any sender */
                     MPI_ANY_TAG,    /* any type of message */
                     MPI_COMM_WORLD, /* default communicator */
                     &status);       /* info about the received message */
            // printf("Master: get notified from slave %d workitem from %d to %d (velocity) is ready with tag %d\n", status.MPI_SOURCE, res[0], res[1], status.MPI_TAG);
        }
        for (int rank = 1; rank < world_size; ++rank)
        {

            // {start index, end index, flag}
            int work[3] = {workitems[rank - 1].first, workitems[rank - 1].second, 1};

            // Send it to each rank
            // tag = 0: exit; tag = other: world_size * 6 + (workitem index+1) * 6 + (0~5)
            // +0: send workload; +1: notify result ready; +2~+5: send resulta

            MPI_Isend(work,                      /* message buffer */
                      3,                         /* one data item */
                      MPI_INT,                   /* data item is an integer */
                      rank,                      /* destination process rank */
                      world_size * 6 + rank * 6, /* user chosen message tag */
                      MPI_COMM_WORLD,            /* default communicator */
                      &request_send[rank - 1]);
            //  printf("Master: send slave %d workitem from %d to %d (velocity) with tag %d\n", rank, work[0], work[1], status.MPI_TAG);
        }
        // Rreceive all the results from the slaves.
        for (int rank = 1; rank < world_size; ++rank)
        {
            // Get notified that a result is ready
            int res[3];
            MPI_Recv(res,            /* message buffer */
                     3,              /* one data item */
                     MPI_INT,        /* of type double real */
                     MPI_ANY_SOURCE, /* receive from any sender */
                     MPI_ANY_TAG,    /* any type of message */
                     MPI_COMM_WORLD, /* default communicator */
                     &status);       /* info about the received message */
            // printf("Master: get notified from slave %d workitem from %d to %d is ready with tag %d\n", status.MPI_SOURCE, res[0], res[1], status.MPI_TAG);
            int start = res[0];
            int end = res[1];
            int n_items = end - start;
            // Receive results from that slave
            MPI_Irecv(&total_x[start],    /* message buffer */
                      n_items,            /* one data item */
                      MPI_DOUBLE,         /* of type double real */
                      status.MPI_SOURCE,  /* receive from any sender */
                      status.MPI_TAG + 1, /* any type of message */
                      MPI_COMM_WORLD,     /* default communicator */
                      &request_recv[0]);  /* info about the received message */
            // printf("Master: receive result total_x[start] = %f\n", total_x[start]);
            MPI_Irecv(&total_y[start],    /* message buffer */
                      n_items,            /* one data item */
                      MPI_DOUBLE,         /* of type double real */
                      status.MPI_SOURCE,  /* receive from any sender */
                      status.MPI_TAG + 2, /* any type of message */
                      MPI_COMM_WORLD,     /* default communicator */
                      &request_recv[1]);  /* info about the received message */
            MPI_Irecv(&total_vx[start],   /* message buffer */
                      n_items,            /* one data item */
                      MPI_DOUBLE,         /* of type double real */
                      status.MPI_SOURCE,  /* receive from any sender */
                      status.MPI_TAG + 3, /* any type of message */
                      MPI_COMM_WORLD,     /* default communicator */
                      &request_recv[2]);  /* info about the received message */
            MPI_Irecv(&total_vy[start],   /* message buffer */
                      n_items,            /* one data item */
                      MPI_DOUBLE,         /* of type double real */
                      status.MPI_SOURCE,  /* receive from any sender */
                      status.MPI_TAG + 4, /* any type of message */
                      MPI_COMM_WORLD,     /* default communicator */
                      &request_recv[3]);  /* info about the received message */
            MPI_Wait(&request_recv[0], MPI_STATUS_IGNORE);
            MPI_Wait(&request_recv[1], MPI_STATUS_IGNORE);
            MPI_Wait(&request_recv[2], MPI_STATUS_IGNORE);
            MPI_Wait(&request_recv[3], MPI_STATUS_IGNORE);

            // printf("Master: receive result from slave %d of workitem from %d to %d (position) with tag %d\n", status.MPI_SOURCE, res[0], res[1], status.MPI_TAG);
        }
        // Tell all the slaves to enter next iteration by sending an empty message with the tag -1.
        for (int rank = 1; rank < world_size; ++rank)
        {
            MPI_Send(0, 0, MPI_INT, rank, 0, MPI_COMM_WORLD);
        }

        // TODO End

        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = t2 - t1;
        total_time += time_span;

        l.save_frame(total_x, total_y);

        // printf("Iteration %d, elapsed time: %.3f\n", i, time_span);
// sleep(1);
#ifdef GUI
        glClear(GL_COLOR_BUFFER_BIT);
        glColor3f(1.0f, 0.0f, 0.0f);
        glPointSize(2.0f);
        glBegin(GL_POINTS);
        double xi;
        double yi;
        for (int i = 0; i < n_body; i++)
        {
            xi = total_x[i];
            yi = total_y[i];
            glVertex2f(xi, yi);
        }
        glEnd();
        glFlush();
        glutSwapBuffers();
#else

#endif
    }

    delete total_m;
    delete total_x;
    delete total_y;
    delete total_vx;
    delete total_vy;
}

int main(int argc, char *argv[])
{
    n_body = atoi(argv[1]);
    n_iteration = atoi(argv[2]);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (my_rank == 0)
    {
#ifdef GUI
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
        glutInitWindowSize(800, 800);
        glutInitWindowPosition(0, 0);
        glutCreateWindow("N Body Simulation MPI Implementation");
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // glMatrixMode(GL_PROJECTION);
        gluOrtho2D(0, bound_x, 0, bound_y);
#endif
        master();
    }
    else
    {
        slave();
    }

    if (my_rank == 0)
    {
        printf("Student ID: 119010265\n"); // replace it with your student id
        printf("Name: SHI Wenlan\n");      // replace it with your name
        printf("Assignment 3: N Body Simulation MPI Implementation\n");
        printf("n_body = %d, n_iteration = %d, world_size = %d\n", n_body, n_iteration, world_size);
        printf("Used time: %.3f\n", total_time);
    }

    MPI_Finalize();

    return 0;
}
