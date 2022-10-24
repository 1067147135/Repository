#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>
#include <termios.h>
#include <fcntl.h>

#define ROW 10
#define COLUMN 50

struct Node
{
	int x, y;
	Node(int _x, int _y) : x(_x), y(_y){}; // initialization list
	Node(){};							   // contructor function
} frog;

char map[ROW + 10][COLUMN];
char bar[12];
int bar_pos = 6;
pthread_mutex_t mutex;
pthread_mutex_t mutex2;
int isQuit = 0;
int isWin = 0;
int isLose = 0;
int speed = 200000;

// Determine a keyboard is hit or not. If yes, return 1. If not, return 0.
int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);

	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);

	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);

	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}

// Avoid printing alternately between two threads
void printer()
{
	/*  Print the map on the screen  */
	pthread_mutex_lock(&mutex);
	printf("\033[H\033[2J");
	printf("speed bar(k: speed down, l: speed up):");
	puts(bar);
	char tmp = map[frog.x][frog.y];
	map[frog.x][frog.y] = '0';
	for (int i = 0; i <= ROW; ++i)
		puts(map[i]);
	map[frog.x][frog.y] = tmp;
	pthread_mutex_unlock(&mutex);
}

// Avoid frog.x/frog.y add 1 in two threads spontaneously and only add 1 rather than 2 in the end
void mover(int x, int y)
{
	pthread_mutex_lock(&mutex2);
	frog.x += x;
	frog.y += y;
	pthread_mutex_unlock(&mutex2);
}

void *frog_move(void *t)
{
	/*  Check keyboard hits, to change frog's position or quit the game. */
	while (!isQuit && !isWin && !isLose)
	{
		if (kbhit())
		{
			char dir = getchar();
			if (dir == 'w' || dir == 'W')
				mover(-1, 0);

			if (dir == 'a' || dir == 'A')
				mover(0, -1);

			if (dir == 'd' || dir == 'D')
				mover(0, 1);

			if (dir == 's' || dir == 'S')
				mover(1, 0);

			if (dir == 'q' || dir == 'Q')
			{
				isQuit = 1;
			}
			if (dir == 'k' || dir == 'K'){	// speed down
				if (bar_pos > 1){	// bar_pos = 2 -> bar[1] = ' ', bar_pos = 1
					speed += 20000;
					bar[--bar_pos] = ' ';
				}
			} 
			if (dir == 'l' || dir == 'L'){	// speed up
				if (bar_pos < 11){	// bar_pos = 10 -> bar[10] = '=', bar_pos = 11
					speed -= 20000;
					bar[bar_pos++] = '=';
				}
			} 
			
			printer();
		}
		/*  Check game's status  */
		if (frog.y < 0 || frog.y > 49 || map[frog.x][frog.y] == ' ')
		{
			isLose = 1;
		}
		if (frog.x == 0)
		{
			isWin = 1;
		}
	}
	pthread_exit(NULL);
}

void *logs_move(void *t)
{

	// initialization
	int start[9];
	int length[9];
	for (int i = 0; i < ROW - 1; i++)
	{									  // 0~9
		length[i] = rand() % 11 + 10;	  // log length: 10~20
		start[i] = rand() % (COLUMN - 1); // start position: 0~49
		for (int j = 0; j < length[i]; j++)
		{
			map[i + 1][(start[i] + j) % (COLUMN - 1)] = '=';
		}
	}
	printer();
	// sleep(3);
	/*  Move the logs  */
	while (!isQuit && !isWin && !isLose)
	{
		for (int i = 1; i < ROW; i++)
		{
			if (i % 2)
			{ // move left
				if (frog.x == i)
				{
					mover(0, -1);
				}
				start[i - 1] = (start[i - 1] - 1 + (COLUMN - 1)) % (COLUMN - 1);
				pthread_mutex_lock(&mutex);	// protect data in map, avoid tmp in printer() store the wrong data to recover
				map[i][start[i - 1]] = '=';
				map[i][(start[i - 1] + length[i - 1]) % (COLUMN - 1)] = ' ';
				pthread_mutex_unlock(&mutex);
			}
			else
			{ // move right
				if (frog.x == i)
				{
					mover(0, 1);
				}
				map[i][(start[i - 1] + length[i - 1]) % (COLUMN - 1)] = '=';
				pthread_mutex_lock(&mutex);	// protect data in map, avoid tmp in printer() store the wrong data to recover
				map[i][start[i - 1]] = ' ';
				start[i - 1] = (start[i - 1] + 1) % (COLUMN - 1);
				pthread_mutex_unlock(&mutex);
			}
		}
		// sleep(1);
		/*  Check game's status  */
		if (frog.y < 0 || frog.y > 49 || map[frog.x][frog.y] == ' ')
		{
			isLose = 1;
		}
		if (frog.x == 0)
		{
			isWin = 1;
		}
		usleep(speed);
		printer();
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{

	// Initialize the river map and frog's starting position
	memset(map, 0, sizeof(map));
	int i, j;
	for (i = 1; i < ROW; ++i)
	{									 // 9 blank rows
		for (j = 0; j < COLUMN - 1; ++j) // 49 columns
			map[i][j] = ' ';
	}

	for (j = 0; j < COLUMN - 1; ++j) // upper river bank
		map[ROW][j] = map[0][j] = '|';

	for (j = 0; j < COLUMN - 1; ++j) // lower river bank
		map[0][j] = map[0][j] = '|';

	for (i = 1; i < 6; ++i){
		bar[i] = '=';
	}
	for (i = 6; i < 11; ++i){
		bar[i] = ' ';
	}
	bar[0] = '[';
	bar[11] = ']';
	frog = Node(ROW, (COLUMN - 1) / 2);
	map[frog.x][frog.y] = '0';

	//Print the map into screen
	for (i = 0; i <= ROW; ++i)
		puts(map[i]);

	/*  Create pthreads for wood move and frog control.  */

	pthread_t tid1, tid2;
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&mutex2, NULL);
	map[frog.x][frog.y] = '|'; // keep map without 0, only add 0 before print, then delete it after print
	pthread_create(&tid1, NULL, frog_move, NULL);
	pthread_create(&tid2, NULL, logs_move, NULL);

	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);

	/*  Display the output for user: win, lose or quit.  */
	printf("\033[H\033[2J");
	if (isWin)
	{
		printf("You win the game!!\n");
	}
	else if (isLose)
	{
		printf("You lose the game!!\n");
	}
	else if (isQuit)
	{
		printf("You exit the game.\n");
	}
	pthread_exit(NULL);
	return 0;
}

