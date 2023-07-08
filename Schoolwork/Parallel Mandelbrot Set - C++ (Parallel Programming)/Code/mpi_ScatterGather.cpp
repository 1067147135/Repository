#include "asg2.h"
#include <stdio.h>
#include <mpi.h>

int rank;
int world_size;
// Create the datatype
MPI_Datatype point_type;
int lengths[3] = {1, 1, 1};

void print_point(Point p){
	printf("x = %d, y = %d, color = %f\n", p.x, p.y, p.color);
}

void separate(int num_numbers, int num_threads, int res[])
{
	int q = num_numbers / num_threads;
	int r = num_numbers % num_threads;
	int i = 1;
	res[0] = 0;
	for (; i < r; i++)
	{
		res[i] = q + 1;
	}
	for (; i < num_threads+1; i++)
	{
		res[i] = q;
	}
}

void master()
{
	// TODO: procedure run in master process
	//  MPI_Scatter...
	//  MPI_Gather...
	//  the following code is not a necessary, please replace it with MPI implementation.
	// TODO END
	MPI_Bcast(&total_size,1,MPI_INT,0,MPI_COMM_WORLD);
	// printf("Master start...\n");
	int sep[world_size];
	int disp[world_size];
	separate(total_size, world_size - 1, sep);
	int sum = 0;
	disp[0] = 0;
	for (int i = 1; i < world_size; i++)
	{
		disp[i] = sum;
		sum += sep[i];
	}

	// printf("Values in the sep of root process:");
	// for(int i = 0; i < world_size; i++)
	// {
	// 	printf(" %d", sep[i]);
	// }
	// printf("\nValues in the disp of root process:");
	// for(int i = 0; i < world_size; i++)
	// {
	// 	printf(" %d", disp[i]);
	// }
	// printf("\n");

	MPI_Scatterv(data, sep, disp, point_type, 0, 0, point_type, 0, MPI_COMM_WORLD);
	// printf("Master sent data to slaves.\n");
	MPI_Gatherv(0, 0, point_type, data, sep, disp, point_type, 0, MPI_COMM_WORLD);
	// printf("Master finished.\n");
	return;
}



void slave()
{
	// TODO: procedure run in slave process
	//  MPI_Scatter...
	//  MPI_Gather...
	// TODO END
	MPI_Bcast(&total_size,1,MPI_INT,0,MPI_COMM_WORLD);
	// printf("Slave %d start...\n", rank);
	int sep[world_size];
	separate(total_size, world_size - 1, sep);

	int num_my_element = sep[rank]; // number of elements allocated to each process
	// printf("Slave %d should got %d entries.\n", rank, num_my_element);
	Point* my_element = new Point[num_my_element];
	// printf("Slave %d allocated my_element.\n", rank);

	MPI_Scatterv(NULL, NULL, NULL, point_type, my_element, num_my_element, point_type, 0, MPI_COMM_WORLD);
	// printf("Slave %d received data from master and begins to process.\n", rank);
	Point* p = my_element;
	for (int index = 0; index < num_my_element; index++)
	{
		// print_point(my_element[index]);
		compute(p);
		p++;
		// print_point(my_element[index]);
	}
	// printf("Slave %d finished processing data, and try to send the result to master.\n", rank);
	MPI_Gatherv(my_element, num_my_element, point_type, NULL, NULL, NULL, point_type, 0, MPI_COMM_WORLD);
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
    Point dummy_point;
    MPI_Aint base_address;
    MPI_Get_address(&dummy_point, &base_address);
    MPI_Get_address(&dummy_point.x, &displacements[0]);
    MPI_Get_address(&dummy_point.y, &displacements[1]);
    MPI_Get_address(&dummy_point.color, &displacements[2]);
    displacements[0] = MPI_Aint_diff(displacements[0], base_address);
    displacements[1] = MPI_Aint_diff(displacements[1], base_address);
    displacements[2] = MPI_Aint_diff(displacements[2], base_address);

    MPI_Datatype types[3] = { MPI_INT, MPI_INT, MPI_FLOAT };
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
		printf("Assignment 2 MPI, Scatter-Gather Version\n");
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
