#include "asg2.h"
#include <stdio.h>
#include <mpi.h>

int rank;
int world_size;
// Create the datatype
MPI_Datatype point_type;
int lengths[3] = {1, 1, 1};

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

void print_point(Point p){
	printf("x = %d, y = %d, color = %f\n", p.x, p.y, p.color);
}

void master()
{
	// TODO: procedure run in master process
	//  MPI_Scatter...
	//  MPI_Gather...
	//  the following code is not a necessary, please replace it with MPI implementation.
	// TODO END

	// printf("Master start...\n");
	MPI_Bcast(&total_size,1,MPI_INT,0,MPI_COMM_WORLD);

	// for (int r = 1; r < world_size; r++) {
	// 	MPI_Send(0, 0, MPI_INT, r, 0, MPI_COMM_WORLD);
	// }
	
	int sep[world_size - 1];
	int disp[world_size - 1];
	// printf("Master: total_size = %d, world_size = %d\n", total_size, world_size);
	separate(total_size, world_size - 1, sep);
	int sum = 0;
	for (int i = 0; i < world_size - 1; i++)
	{
		disp[i] = sum;
		sum += sep[i];
	}

	// printf("Values in the sep of root process:");
	// for(int i = 0; i < world_size-1; i++)
	// {
	// 	printf(" %d", sep[i]);
	// }
	// printf("\nValues in the disp of root process:");
	// for(int i = 0; i < world_size-1; i++)
	// {
	// 	printf(" %d", disp[i]);
	// }
	// printf("\n");

	MPI_Request request_send[world_size - 1];
	for (int r = 1; r < world_size; r++)
	{	
		// MPI_Send(&data[disp[r - 1]], sep[r - 1], point_type, r, r * 2, MPI_COMM_WORLD); 
		MPI_Isend(&data[disp[r - 1]], sep[r - 1], point_type, r, r * 2, MPI_COMM_WORLD, &request_send[r - 1]);
		// printf("Master sent data of %d entries to slave %d with tag %d.\n", sep[r - 1], r, r * 2);
	}

	for (int r = 1; r < world_size; r++)
	{
		MPI_Wait(&request_send[r - 1], MPI_STATUS_IGNORE);
		// printf("Master is waiting for data from slave %d with tag %d.\n", r, 2 * r + 1);
		MPI_Recv(&data[disp[r - 1]], sep[r - 1], point_type, r, 2 * r + 1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		// printf("Master received data from slave %d.\n", r);
	}

	// printf("Master finished.\n");
}

void slave()
{
	// TODO: procedure run in slave process
	//  MPI_Scatter...
	//  MPI_Gather...
	// TODO END

	MPI_Bcast(&total_size,1,MPI_INT,0,MPI_COMM_WORLD);

	// printf("Slave %d wait for master...\n", rank);
	// MPI_Status status;
	// while (true) {
	// 	int signal;
	// 	MPI_Recv(&signal, 1, MPI_INT, 0, MPI_ANY_TAG,
    //          MPI_COMM_WORLD, &status);
 
	// 	/* Check the tag of the received message. */
	
	// 	if (status.MPI_TAG == 0) {
	// 		break;
	// 	}
	// }

	// printf("Slave %d start...\n", rank);
	int sep[world_size - 1];
	// printf("Slave %d: total_size = %d, world_size = %d\n", rank, total_size, world_size);
	separate(total_size, world_size - 1, sep);
	// printf("Values in the sep of slave %d process:", rank);
	// for(int i = 0; i < world_size-1; i++)
	// {
	// 	printf(" %d", sep[i]);
	// }
	// printf("\n");
	
	int num_my_element = sep[rank - 1]; // number of elements allocated to each process
	// printf("Slave %d should got %d entries.\n", rank, num_my_element);
	Point* my_element = new Point[num_my_element];
	// printf("Slave %d allocated my_element.\n", rank);
	
	// printf("Slave %d is waiting for data with tag %d from master.\n", rank, 2 * rank);
	MPI_Recv(my_element, num_my_element, point_type, 0, 2 * rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	// printf("Slave %d received data from master and begins to process.\n", rank);
	// print_point(my_element[0]);
	Point* p = my_element;
	for (int index = 0; index < num_my_element; index++)
	{
		// print_point(my_element[index]);
		compute(p);
		p++;
		// print_point(my_element[index]);
	}
	// printf("Slave %d finished processing data, and try to send the result with tag %d to master.\n", rank, 2 * rank + 1);
	MPI_Send(my_element, num_my_element, point_type, 0,	2 * rank + 1, MPI_COMM_WORLD); 
	// printf("Slave %d finished.\n", rank);
	delete [] my_element;
}

int main(int argc, char *argv[])
{
	if (argc == 4)
	{
		X_RESN = atoi(argv[1]);
		Y_RESN = atoi(argv[2]);
		max_iteration = atoi(argv[3]);
	}
	else
	{
		X_RESN = 1000;
		Y_RESN = 1000;
		max_iteration = 100;
	}

	if (rank == 0)
	{
#ifdef GUI
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
		glutInitWindowSize(500, 500);
		glutInitWindowPosition(0, 0);
		glutCreateWindow("MPI");
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glMatrixMode(GL_PROJECTION);
		gluOrtho2D(0, X_RESN, 0, Y_RESN);
		glutDisplayFunc(plot);
#endif
	}

	/* computation part begin */
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	// Calculate displacements
	// In C, by default padding can be inserted between fields. MPI_Get_address will allow
	// to get the address of each struct field and calculate the corresponding displacement
	// relative to that struct base address. The displacements thus calculated will therefore
	// include padding if any.
	MPI_Aint displacements[3];
	struct pointtype dummy_point;
	MPI_Aint base_address;
	MPI_Get_address(&dummy_point, &base_address);
	MPI_Get_address(&dummy_point.x, &displacements[0]);
	MPI_Get_address(&dummy_point.y, &displacements[1]);
	MPI_Get_address(&dummy_point.color, &displacements[2]);
	displacements[0] = MPI_Aint_diff(displacements[0], base_address);
	displacements[1] = MPI_Aint_diff(displacements[1], base_address);
	displacements[2] = MPI_Aint_diff(displacements[2], base_address);

	MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_FLOAT};
	MPI_Type_create_struct(3, lengths, displacements, types, &point_type);
	MPI_Type_commit(&point_type);

	if (rank == 0)
	{
		t1 = std::chrono::high_resolution_clock::now();

		initData();
		master();

		t2 = std::chrono::high_resolution_clock::now();
		time_span = t2 - t1;

		printf("Student ID: 119010265\n"); // replace it with your student id
		printf("Name: Shi Wenlan\n");	   // replace it with your name
		printf("Assignment 2 MPI, Send-Recv Version\n");
		printf("Run Time: %f seconds\n", time_span.count());
		printf("Problem Size: %d * %d, %d\n", X_RESN, Y_RESN, max_iteration);
		printf("Process Number: %d\n", world_size);
	}
	else
	{
		slave();
	}

	MPI_Finalize();
	/* computation part end */

	if (rank == 0)
	{
#ifdef GUI
		glutMainLoop();
#endif
	}

	return 0;
}
