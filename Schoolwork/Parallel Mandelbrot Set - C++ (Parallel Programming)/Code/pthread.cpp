#include "asg2.h"
#include <stdio.h>
#include <pthread.h>

int n_thd; // number of threads

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

typedef struct {
    //TODO: specify your arguments for threads
    int rank;
    int world_size;
    int displacement;
    int count;
    //TODO END
} Args;


void* worker(void* args) {
    //TODO: procedure in each threads
    // the code following is not a necessary, you can replace it.
    
    // Args* my_arg = (Args*) args;
    // int a = my_arg->a;
    // int b = my_arg->b;
    Args* my_arg = (Args*) args;
    // printf("Thread %d start...", my_arg->rank);
    for (int index = 0; index < my_arg->count; index++){
        compute(&data[my_arg->displacement + index]); 
    }
    // printf("Thread %d finished", my_arg->rank);
    //TODO END

}


int main(int argc, char *argv[]) {

	if ( argc == 5 ) {
		X_RESN = atoi(argv[1]);
		Y_RESN = atoi(argv[2]);
		max_iteration = atoi(argv[3]);
        n_thd = atoi(argv[4]);
	} else {
		X_RESN = 1000;
		Y_RESN = 1000;
		max_iteration = 100;
        n_thd = 4;
	}

    #ifdef GUI
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Pthread");
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0, X_RESN, 0, Y_RESN);
	glutDisplayFunc(plot);
    #endif

	/* computation part begin */
    t1 = std::chrono::high_resolution_clock::now();
	
    initData();

	//TODO: assign jobs

    int sep[n_thd];
	int disp[n_thd];
	// printf("Master: total_size = %d, world_size = %d", total_size, n_thd);
	separate(total_size, n_thd, sep);
	int sum = 0;
	for (int i = 0; i < n_thd; i++)
	{
		disp[i] = sum;
		sum += sep[i];
	}

	// printf("Values in the sep of root process:");
	// for(int i = 0; i < n_thd; i++)
	// {
	// 	printf(" %d", sep[i]);
	// }
	// printf("\nValues in the disp of root process:");
	// for(int i = 0; i < n_thd; i++)
	// {
	// 	printf(" %d", disp[i]);
	// }
	// printf("\n");

    pthread_t thds[n_thd]; // thread pool
    Args args[n_thd]; // arguments for all threads
    for (int thd = 0; thd < n_thd; thd++){
        args[thd].rank = thd;
        args[thd].world_size = n_thd;
        args[thd].displacement = disp[thd];
        args[thd].count = sep[thd];
    }
    for (int thd = 0; thd < n_thd; thd++) pthread_create(&thds[thd], NULL, worker, &args[thd]);
    for (int thd = 0; thd < n_thd; thd++) pthread_join(thds[thd], NULL);
    //TODO END

    t2 = std::chrono::high_resolution_clock::now();  
	time_span = t2 - t1;
	/* computation part end */

    printf("Student ID: 119010265\n"); // replace it with your student id
	printf("Name: Shi Wenlan\n"); // replace it with your name
	printf("Assignment 2 Pthread\n");
	printf("Run Time: %f seconds\n", time_span.count());
	printf("Problem Size: %d * %d, %d\n", X_RESN, Y_RESN, max_iteration);
	printf("Thread Number: %d\n", n_thd);

    #ifdef GUI
	glutMainLoop();
    #endif

	return 0;
}

