#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <mpi.h>
#include <time.h>
#include <unistd.h>
#include <algorithm>
#include <vector>

#ifdef GUI
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif


#include "./headers/physics.h"


int size; // problem size


int my_rank;
int world_size;


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


void slave(){
    // TODO: MPI routine
    float* local_old = new float[size * size];
    float* local_new = new float[size * size];
    bool* local_fire = new bool[size * size];

    MPI_Status status;
    int work[2];
    // copy global data to local
    MPI_Bcast(local_fire,size * size,MPI_CXX_BOOL,0,MPI_COMM_WORLD);
    MPI_Bcast(local_old, size * size,MPI_FLOAT,0,MPI_COMM_WORLD);
    MPI_Bcast(local_new, size * size,MPI_FLOAT,0,MPI_COMM_WORLD);
    // printf("slave: copy data finished\n");

    while (true) {
        MPI_Recv(work, 2, MPI_INT, 0, MPI_ANY_TAG,MPI_COMM_WORLD, &status);
        // printf("slave: receive message with tag %d\n", status.MPI_TAG);

        if (status.MPI_TAG == 1) {  // exit
            break;
        }
        if (status.MPI_TAG == 0) {  // enter next iteration
            // printf("slave: wait broadcast\n");
            MPI_Bcast(local_old, size * size,MPI_FLOAT,0,MPI_COMM_WORLD);
            // printf("slave: receive broadcast\n");
            continue;
        }

        int start = work[0];
        int end = work[1];
        int n_items = (end - start) * size;
        
        update(local_old, local_new, local_fire, start, end);
        // printf("slave: old = %f, new = %f\n", local_old[start * size + 500], local_new[start * size + 500]);
        // printf("Slave %d: finish workitem from %d to %d\n", my_rank, work[0], work[1]);

        // notify the result is ready
        MPI_Send(work, 2, MPI_INT, 0, status.MPI_TAG+1, MPI_COMM_WORLD);
        // printf("Slave %d: notify finish workitem from %d to %d\n", my_rank, work[0], work[1]);
        // send the result
        MPI_Send(&local_new[start * size], n_items, MPI_FLOAT, 0, status.MPI_TAG+2, MPI_COMM_WORLD); 
        // printf("Slave %d: finish send result of workitem from %d to %d\n", my_rank, work[0], work[1]);
    }
}

void calculate_work_items(std::vector<std::pair<int, int>> &workitems){
    int start = 1;
    int end = 1;
    // 1 master + (world_size-1) slaves
    int row = size - 2;
    if (row < 100 * (world_size-1)){    // static allocation
        int q = row / (world_size-1);
        int r = row % (world_size-1);
        int i = 0;
        for (; i < r; i++)
        {   
            start = end;
            end += q + 1;
            workitems.push_back(std::make_pair(start, end));
        }
        for (; i < (world_size-1); i++)
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

void master() {
    // TODO: MPI routine 
    float* data_odd = new float[size * size];
    float* data_even = new float[size * size];
    bool* fire_area = new bool[size * size];

    #ifdef GUI
    GLubyte* pixels = new GLubyte[resolution * resolution * 3];
    #endif

    generate_fire_area(fire_area);
    initialize(data_odd, fire_area);
    initialize(data_even, fire_area);

    MPI_Bcast(fire_area,size * size,MPI_CXX_BOOL,0,MPI_COMM_WORLD);
    MPI_Bcast(data_odd, size * size,MPI_FLOAT,0,MPI_COMM_WORLD);
    MPI_Bcast(data_even, size * size,MPI_FLOAT,0,MPI_COMM_WORLD);
    // printf("master: broadcast data finished\n");
    std::vector<std::pair<int, int>> workitems;
    calculate_work_items(workitems);
    MPI_Status status;
    MPI_Request request_send[world_size - 1];

    // bool cont = true;
    int count = 1;
    double total_time = 0;
    
    for (int c = 0; c < 300; c++) {
    // while (cont) {
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

        for (int rank = 1; rank < world_size; ++rank) {
            // Find the next item of work to do 
            // {start index, end index}
            int work[2] = {workitems[rank-1].first, workitems[rank-1].second};
        
            // Send it to each rank 
            // tag = 0: exit this iteration; tag = 1: exit the process; tag = other: (workitem index+1) * 3 + (0~2)
            // +0: send workload; +1: notify result ready; +2: send result
            MPI_Isend(work, 2, MPI_INT, rank, rank*3, MPI_COMM_WORLD, &request_send[rank - 1]); 
            // printf("Master: send workitem from %d to %d -> slave %d with tag %d\n", work[0], work[1], rank, rank*3);
        }
        for (int rank = 1; rank < world_size; ++rank) {
            MPI_Wait(&request_send[rank - 1], MPI_STATUS_IGNORE);
        }

        // Loop over getting new work requests until there is no more work to be done
        int index = world_size-1; // worditem index
        while (index < workitems.size()){
            // Get notified that a result is ready
            int res[2];
            MPI_Recv(res, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);           
            // printf("Master: get notified from slave %d workitem from %d to %d is ready with tag %d\n",status.MPI_SOURCE, res[0], res[1], status.MPI_TAG);
            int start = res[0];
            int end = res[1];
            int n_items = (end - start) * size;
            // Receive results from that slave 
            if (count % 2 == 1) MPI_Recv(&data_even[start * size], n_items, MPI_FLOAT, status.MPI_SOURCE, status.MPI_TAG+1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            else MPI_Recv(&data_odd[start * size], n_items, MPI_FLOAT, status.MPI_SOURCE, status.MPI_TAG+1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // printf("Master: receive result total_x[start] = %f\n", total_x[start]);
            
            // Send the slave a new work unit 
            int work[2] = {workitems[index].first, workitems[index].second};
            MPI_Send(work, 2, MPI_INT, status.MPI_SOURCE, (index+1)*3, MPI_COMM_WORLD); 
            // printf("Master: send workitem from %d to %d -> slave %d with tag %d\n", work[0], work[1], status.MPI_SOURCE, (index+1)*3);

            // Get the next unit of work to be done 
            index++;
        }

        // There's no more work to be done, so receive all the results from the slaves. 
        for (int rank = 1; rank < world_size; ++rank) {
            // Get notified that a result is ready
            int res[2];
            MPI_Recv(res, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status); 
            // printf("Master: get notified from slave %d workitem from %d to %d is ready with tag %d (last iter)\n",status.MPI_SOURCE, res[0], res[1], status.MPI_TAG);
            int start = res[0];
            int end = res[1];
            int n_items = (end - start) * size;
            // Receive results from that slave 
            if (count % 2 == 1) MPI_Recv(&data_even[start * size], n_items, MPI_FLOAT, status.MPI_SOURCE, status.MPI_TAG+1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            else MPI_Recv(&data_odd[start * size], n_items, MPI_FLOAT, status.MPI_SOURCE, status.MPI_TAG+1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // printf("Master: receive result total_x[start] = %f\n", total_x[start]);
        }

        // Tell all the slaves to enter next iteration by sending an empty message with the tag 0.       
        for (int rank = 1; rank < world_size; ++rank) {
            MPI_Isend(0, 0, MPI_INT, rank, 0, MPI_COMM_WORLD, &request_send[rank - 1]);
        }
        for (int rank = 1; rank < world_size; ++rank) {
            MPI_Wait(&request_send[rank - 1], MPI_STATUS_IGNORE);
        }

        // printf("master: Iteration %d, broadcast for next iteration\n", count);
        if (count % 2 == 1) MPI_Bcast(data_even, size * size,MPI_FLOAT,0,MPI_COMM_WORLD);
        else MPI_Bcast(data_odd, size * size,MPI_FLOAT,0,MPI_COMM_WORLD);

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

    // Tell all the slaves to exit by sending an empty message with the tag 1.       
    for (int rank = 1; rank < world_size; ++rank) {
        MPI_Send(0, 0, MPI_INT, rank, 1, MPI_COMM_WORLD);
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

	MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);


	if (my_rank == 0) {
        #ifdef GUI
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
        glutInitWindowPosition(0, 0);
        glutInitWindowSize(resolution, resolution);
        glutCreateWindow("Heat Distribution Simulation MPI Implementation");
        gluOrtho2D(0, resolution, 0, resolution);
        #endif

        master();
	} else {
        slave();
    }

	if (my_rank == 0){
        printf("size = %d, world_size = %d\n", size, world_size);
		printf("Student ID: 119010265\n"); // replace it with your student id
		printf("Name: SHI Wenlan\n"); // replace it with your name
		printf("Assignment 4: Heat Distribution Simulation MPI Implementation\n");
	}

	MPI_Finalize();

	return 0;
}

