/*
Name: John Stephenson
*/

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <mpi.h>

using namespace std;

const int ALIVE = 1;
const int DEAD = 0;
int globalChanged = 0;
int gameOver = 0;

/* modifies the nextGen table to represent the next generation of The Game of Life */
void nextGeneration(int** table, int** nextGen, int N2rows, int N2cols) {
	int changed = 0;
	for (int i = 1; i < N2rows-1; i++) {
		for (int j = 1; j < N2cols-1; j++) {
			int neighbors = 0;
			int localChange = 0;
			neighbors += table[i - 1][j];
			neighbors += table[i - 1][j-1];
			neighbors += table[i - 1][j+1];
			neighbors += table[i + 1][j];
			neighbors += table[i + 1][j-1];
			neighbors += table[i + 1][j+1];
			neighbors += table[i][j+1];
			neighbors += table[i][j-1];

			if (table[i][j] == DEAD && neighbors == 3) {
				nextGen[i][j] = ALIVE;
				changed = 1;
				localChange = 1;
			}

			if (neighbors <= 1 || neighbors >= 4) {
				if (table[i][j] == ALIVE) {
					nextGen[i][j] = DEAD;
					changed = 1;
					localChange = 1;
				}
			}
			/* this is used to make sure the two tables stay up to date with each other over the generations since they are being swapped after each iteration */
			if(localChange == 0) {
				nextGen[i][j] = table[i][j];
			}
			neighbors = 0;
		}
	}

	/*
		sums all of the changed variables across the process to determine if any of them changed
		if they all haven't changed then the gameover flag is set
	*/
	MPI_Allreduce(&changed, &globalChanged, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	if(globalChanged == 0){
		gameOver = 1;
	}
}

/* initializes a table according to the size provided by the user with each element being randomized to be alive or dead */
void initTable(int** table, int N2rows, int N2cols, int &randcounter){
	for (int i = 0; i < N2rows; i++) {
		for (int j = 0; j < N2cols; j++) {
			if (i == N2rows - 1 || j == N2cols - 1 || i == 0 || j == 0) {
				table[i][j] = DEAD;
			}
			else {
				if (rand() % 2 < 1) {
					table[i][j] = ALIVE;
				}
				else {
					table[i][j] = DEAD;
				}
				randcounter += 1;
			}
		}
	}
}

int main(int argc, char *argv[]){
	double starttime, endtime;
	int rank;
	int size;
	int N;
	int maxGen;
	int randcounter = 0;

	/* MPI Initialization */
	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Status status;

	/* the root process gets the arguments from the command line and broadcasts them to the other processes */
	if(rank == 0){
		N = atoi(argv[1]);
		maxGen = atoi(argv[2]);
	}

	MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&maxGen, 1, MPI_INT, 0, MPI_COMM_WORLD);

	/* Nrows determines the work distribution by row, the columns remain the same for each */
	int Nrows = (N / size + (rank < N % size));
	int Ncols = N;
	int N2rows = Nrows + 2;
	int N2cols = N + 2;

	int** table = new int*[N2rows];
	for (int i = 0; i < N2rows; i++) {
		table[i] = new int[N2cols];
	}

	int** nextGen = new int*[N2rows];
	for (int i = 0; i < N2rows; i++) {
		nextGen[i] = new int[N2cols];
	}

	/* determines which processes the MPI Send and Recv calls correspond to */
	int prev = (rank == 0) ? MPI_PROC_NULL : rank-1;
	int next = (rank == size-1) ? MPI_PROC_NULL : rank+1;

	/* comment out to test correctness */
		srand(time(NULL) + rank);
		initTable(table, N2rows, N2cols, randcounter);
	/* end comment */

	/* start testing correctness */
		// srand(1);
		// freopen("output2.txt","w",stdout);
		// int* trow = new int[N2cols];
		// MPI_Barrier(MPI_COMM_WORLD);
		// if (rank == 0){
		// 	initTable(table, N2rows, N2cols, randcounter);
		// 	MPI_Send(&randcounter, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
		// } else{
		// 	MPI_Recv(&randcounter, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, &status);
		// 	for(int i = 0; i < randcounter; i++){
		// 		rand();
		// 	}
		// 	initTable(table, N2rows, N2cols, randcounter);
		// 	MPI_Send(&randcounter, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
		// }
	/* end testing correctness */

	/* copying the initial values of the table into the nextGen table */
	for (int i = 0; i < N2rows; i++) {
		for (int j = 0; j < N2cols; j++) {
			nextGen[i][j] = table[i][j];
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);
	if(rank == 0){
		starttime = MPI_Wtime();
	}

	/* the main game loop that continues until the max generation or the game over condition has been met */
	for(int g = 0; g < maxGen; g++){
		if(gameOver == 0){
			MPI_Sendrecv(&table[1][0], N+2, MPI_INT, prev, 0, &table[Nrows+1][0], N+2, MPI_INT, next, 0, MPI_COMM_WORLD, &status);
			MPI_Sendrecv(&table[Nrows][0], N+2, MPI_INT, next, 0, &table[0][0], N+2, MPI_INT, prev, 0, MPI_COMM_WORLD, &status);

			/* start testing correctness */
				// if(rank == 0){
				// 	int r;
				// 	int temprows;
				// 	if(rank == size-1){
				// 		r = -1;
				// 	} else{
				// 		r = 0;
				// 	}
				// 	cout << "Generation " << g << ":\n";
				// 	for (int i = 0; i < N2rows; i++) {
				// 		if (r == 0 && i == N2rows - 1){
				// 			continue;
				// 		}
				// 		if (r == 1){
				// 			if(i == 0 || i == N2rows - 1){
				// 				continue;
				// 			}
				// 		}
				// 		if (r == 2 && i == 0){
				// 			continue;
				// 		}
				// 		for (int j = 0; j < N2cols; j++) {
				// 			cout << table[i][j] << " ";
				// 		}
				// 		cout << "\n";
				// 	}
				// 	for(int i = 1; i < size; i++){
				// 		if(i == size-1){
				// 			r = 2;
				// 		} else{
				// 			r = 1;
				// 		}
				// 		MPI_Recv(&temprows, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
				// 		for(int j = 0; j < temprows; j++){
				// 			MPI_Recv(&trow[0], N2cols, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
				// 			if (r == 0 && j == temprows - 1){
				// 				continue;
				// 			}
				// 			if (r == 1){
				// 				if(j == 0 || j == temprows - 1){
				// 					continue;
				// 				}
				// 			}
				// 			if (r == 2 && j == 0){
				// 				continue;
				// 			}
				// 			for(int k = 0; k < N2cols; k++){
				// 				cout << trow[k] << " ";
				// 			}
				// 			cout << "\n";
				// 		}
				// 	}
				// } else{
				// 	MPI_Send(&N2rows, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
				// 	for(int i = 0; i < N2rows; i++){
				// 		MPI_Send(&table[i][0], N2cols, MPI_INT, 0, 2, MPI_COMM_WORLD);
				// 	}
				// }
			/* end testing correctness*/

			nextGeneration(table, nextGen, N2rows, N2cols);
			swap(table, nextGen);
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);
	if(rank == 0){
		endtime = MPI_Wtime();
		printf("Time taken = %lf seconds\n", endtime-starttime);
	}

	MPI_Finalize();
	return 0;
}
