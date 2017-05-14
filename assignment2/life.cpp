#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <mpi.h>
#include <ctime>
#include "life.h"
#include <cmath>
#include <algorithm>
#include <cstring>

/*
This program is a parallel implementation of Conway’s Game of Life.  
It is supposed to take a bunch of parameters on the command line and 
use them to start the game board off.  These parameters also indicated 
how long to run the game and when to print the game board.  The parameters 
included the number of live cells on the board, and the size of the board.  

The algorithm is described pretty in depth in my Homework 3 submission which
I have also included in this submission.  Essentially what the main algorithm 
is is a for loop that goes through each row and checks the 8 cells around and 
counts to see whether or not they are alive.  
*/

using namespace std;

int main(int argc, char *argv[])
{
	int commSize;
	int myRank;

	const int MAX_STRING = 100;

	int m, n, j, i, k;

	// alias to these pointers because I don't feel like remember single characters
	const int& rows = m;
	const int& columns = n;
	const int& iterations = j;
	const int& originalLivingCells = i;
	const int& printIteration = k;

	if (argc < 6)
	{
		cout << "Usage: mpiexec -n <number of processes> ./life <number of living cells> <number of iterations> <number of iterations to print on> <number of rows> <number of columns>" << endl;
		exit(1);
	}

	i = atoi(argv[1]);
	j = atoi(argv[2]);
	k = atoi(argv[3]);
	m = atoi(argv[4]);
	n = atoi(argv[5]);

	int* aliveArray = new int[originalLivingCells];

	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &commSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

	if (myRank == 0)
	{
		generateAlive(aliveArray, originalLivingCells, rows, columns);
	}

	MPI_Bcast(aliveArray, originalLivingCells, MPI_INT, 0, MPI_COMM_WORLD);

	int rowsPerProcessor = ceil(rows/(double)commSize);

	int blockLength = rowsPerProcessor * columns;

	int* myRows = new int[blockLength];
	int* rowAbove = new int[blockLength];
	int* rowBelow = new int[blockLength];

	// strictly for printing
	int* printRow = new int[blockLength];

	fill(myRows, myRows + blockLength, 0);

	fillGrid(myRows, aliveArray, originalLivingCells, myRank, rowsPerProcessor, columns);

	MPI_Barrier(MPI_COMM_WORLD);

	for (int counter = 0; counter < iterations; counter++)
	{
		// before I do ANYTHING I need to send my shit.

		int destinationProcessTopRows;
		int sourceProcessTopRows;
		int destinationProcessBottomRows;
		int sourceProcessBottomRows;


		if (myRank == 0)
		{
			destinationProcessTopRows = commSize - 1;

		}
		else
		{
			destinationProcessTopRows = myRank - 1;
		}

		if (myRank == commSize - 1)
		{
			destinationProcessBottomRows = 0;
		}
		else
		{
			destinationProcessBottomRows = myRank + 1;
		}

		if (myRank == 0)
		{
			sourceProcessTopRows = commSize - 1;
			sourceProcessBottomRows = myRank + 1;
		}
		else if (myRank == commSize - 1)
		{
			sourceProcessTopRows = myRank - 1;
			sourceProcessBottomRows = 0;
		}
		else
		{
			sourceProcessTopRows = myRank - 1;
			sourceProcessBottomRows = myRank + 1;
		}

		// Send my top row up
		MPI_Send(myRows, columns, MPI_INT, destinationProcessTopRows, 0, MPI_COMM_WORLD);
		// send my bottom row down
		MPI_Send(myRows + ((rowsPerProcessor - 1) * columns), columns, MPI_INT, destinationProcessBottomRows, 0, MPI_COMM_WORLD);
		// receive my top row
		MPI_Recv(rowAbove, columns, MPI_INT, sourceProcessTopRows, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		// receive my bottom row
		MPI_Recv(rowBelow, columns, MPI_INT, sourceProcessBottomRows, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


		for (int rowCounter = 0; rowCounter < rowsPerProcessor; rowCounter++)
		{
			for (int elementCounter = rowCounter * columns; elementCounter < blockLength; elementCounter++)
			{
				int aliveCounter = 0;


				// top row of my block
				if (rowCounter == 0)
				{
					//left
					//one row per processor
					if (rowsPerProcessor == 1)
					{
						if (elementCounter == 0)
						{
							if (rowAbove[0] == 1)
							{
							//	cout << "ben" << endl;
								aliveCounter++;
							}
							if (rowAbove[1] == 1)
							{
							//	cout << "is " << endl;
								aliveCounter++;
							}
							if (rowAbove[columns - 1] == 1)
							{
							//	cout << "awesome" << endl;
								aliveCounter++;
							}
							if (rowBelow[0] == 1)
							{
							//	cout << "like" << endl;
								aliveCounter++;
							}
							if (rowBelow[1] == 1)
							{
							//	cout << "super" << endl;
								aliveCounter++;
							}
							if (rowBelow[columns - 1] == 1)
							{
							//	cout << "crazy" << endl;
								aliveCounter++;
							}
							if (myRows[1] == 1)
							{
							//	cout << "brilliantly" << endl;
								aliveCounter++;
							}
							if (myRows[columns - 1]  == 1)
							{
							//	cout << "great!" << endl;
								aliveCounter++;
							}
						}
						else if (elementCounter == columns - 1)
						{
							if (myRows[0] == 1)
							{
								aliveCounter++;
							}
							if (myRows[columns - 2] == 1)
							{
								aliveCounter++;
							}
							if (rowAbove[0] == 1)
							{
								aliveCounter++;
							}
							if (rowAbove[columns - 1] == 1)
							{
								aliveCounter++;
							}
							if (rowAbove[columns - 2] == 1)
							{
								aliveCounter++;
							}
							if (rowBelow[0] == 1)
							{
								aliveCounter++;
							}
							if (rowBelow[columns - 2] == 1)
							{
								aliveCounter++;
							}
							if (rowBelow[columns - 1] == 1)
							{
								aliveCounter++;
							}
						}
						else
						{
							if (rowAbove[elementCounter - 1] == 1)
							{
								aliveCounter++;
							}
							if (rowAbove[elementCounter] == 1)
							{
								aliveCounter++;
							}
							if (rowAbove[elementCounter + 1] == 1)
							{
								aliveCounter++;
							}
							if (myRows[elementCounter - 1] == 1)
							{
								aliveCounter++;
							}
							if (myRows[elementCounter + 1] == 1)
							{
								aliveCounter++;
							}
							if (rowBelow[elementCounter - 1] == 1)
							{
								aliveCounter++;
							}
							if (rowBelow[elementCounter] == 1)
							{
								aliveCounter++;
							}
							if (rowBelow[elementCounter + 1] == 1)
							{
								aliveCounter++;
							}
						}
						//cout << "myRank: " << myRank << " " <<  aliveCounter << endl;
					}
					else if (rowsPerProcessor > 1)
					{
						if (elementCounter % columns == 0)
						{
                            if (rowCounter < rowsPerProcessor - 1)
                            {
                                if (myRows[(rowCounter + 1) * columns] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (myRows[((rowCounter + 1) * columns) + 1] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (myRows[((rowCounter + 1) * columns) + (columns - 1)] == 1)
                                {
                                    aliveCounter++;
                                }
                            }
                            else if (rowCounter == rowsPerProcessor - 1)
                            {
                                if (rowBelow[0] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (rowBelow[1] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (rowBelow[columns - 1] == 1)
                                {
                                    aliveCounter++;
                                }
                            }
                            if (myRows[elementCounter + 1] == 1)
                            {
                                aliveCounter++;
                            }
                            if (myRows[elementCounter + (columns - 1)] == 1)
                            {
                                aliveCounter++;
                            }
                            if (rowCounter > 0)
                            {
                                if (myRows[elementCounter - columns] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (myRows[elementCounter - (columns - 1)] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (myRows[elementCounter - 1] == 1)
                                {
                                    aliveCounter++;
                                }
                            }
                            else if (rowCounter == 0)
                            {
                                if (rowAbove[0] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (rowAbove[1] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (rowAbove[columns - 1] == 1)
                                {
                                    aliveCounter++;
                                }
                            }
                            // printf("FAMILY/TURKEY WAS FAR MORE IMPORTANT");
						}
						else if (elementCounter % columns == columns - 1)
						{
                            if (rowCounter < rowsPerProcessor - 1)
                            {
                                if (myRows[(rowCounter + 1) * columns] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (myRows[((rowCounter + 1) * columns) + columns - 2] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (myRows[((rowCounter + 1) * columns) + columns - 1] == 1)
                                {
                                    aliveCounter++;
                                }
                            }
                            else if (rowCounter == rowsPerProcessor - 1)
                            {
                                if (rowBelow[columns - 1] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (rowBelow[columns - 2] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (rowBelow[0] == 1)
                                {
                                    aliveCounter++;
                                }
                            }
                            if (myRows[rowCounter * columns] == 1)
                            {
                                aliveCounter++;
                            }
                            if (myRows[(rowCounter * columns) + columns - 2] == 1)
                            {
                                aliveCounter++;
                            }
                            if (rowCounter > 0)
                            {
                                if (myRows[(rowCounter - 1) * columns] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (myRows[(rowCounter - 1) * columns + (columns - 2)] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (myRows[(rowCounter - 1) * columns + (columns - 1)] == 1)
                                {
                                    aliveCounter++;
                                }
                            }
                            else if (rowCounter == 0)
                            {
                                if (rowAbove[0] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (rowAbove[columns - 2] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (rowAbove[columns - 1] == 1)
                                {
                                    aliveCounter++;
                                }
                            }

						}
                        // everything but corner of blocks of rows
						else
						{
                            if (rowsPerProcessor == 1)
                            {
                                if (rowBelow[elementCounter] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (rowBelow[elementCounter + 1] == 1)
                                {
                                    aliveCounter;
                                }
                                if (rowBelow[elementCounter - 1] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (rowAbove[elementCounter] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (rowAbove[elementCounter + 1] == 1)
                                {
                                    aliveCounter++;
                                }
                                if (rowAbove[elementCounter - 1] == 1)
                                {
                                    aliveCounter++;
                                }
                            }
                            else if (rowsPerProcessor > 1)
                            {
                                if (rowCounter == 0)
                                {
                                    if (rowAbove[elementCounter - 1] == 1)
                                    {
                                        aliveCounter++;
                                    }
                                    if (rowAbove[elementCounter] == 1)
                                    {
                                        aliveCounter++;
                                    }
                                    if (rowAbove[elementCounter + 1] == 1)
                                    {
                                        aliveCounter++;
                                    }
                                }
                                else if (rowCounter > 0)
                                {
                                    if (myRows[elementCounter - columns - 1] == 1)
                                    {
                                        aliveCounter++;
                                    }
                                    if (myRows[elementCounter - columns] == 1)
                                    {
                                        aliveCounter++;
                                    }
                                    if (myRows[elementCounter - columns + 1] == 1)
                                    {
                                        aliveCounter++;
                                    }
                                }
                                if (rowCounter < rowsPerProcessor - 1)
                                {
                                    if (myRows[elementCounter + columns - 1] == 1)
                                    {
                                        aliveCounter++;
                                    }
                                    if (myRows[elementCounter + columns] == 1)
                                    {
                                        aliveCounter++;
                                    }
                                    if (myRows[elementCounter + columns + 1] == 1)
                                    {
                                        aliveCounter++;
                                    }
                                }
                                else if (rowCounter == rowsPerProcessor - 1)
                                {
                                    if (rowBelow[elementCounter % columns] == 1)
                                    {
                                        aliveCounter++;
                                    }
                                    if (rowBelow[(elementCounter % columns) - 1] == 1)
                                    {
                                        aliveCounter++;
                                    }
                                    if (rowBelow[(elementCounter % columns) + 1] == 1)
                                    {
                                        aliveCounter++;
                                    }
                                }
                            }

                            if (myRows[elementCounter + 1] == 1)
                            {
                                aliveCounter++;
                            }
                            if (myRows[elementCounter - 1] == 1)
                            {
                                aliveCounter++;
                            }
						}
					}
				}



				// bottom row of my block
				else if (rowCounter == rowsPerProcessor - 1)
				{
					// not actually necessary lol
					if (rowsPerProcessor == 1)
					{
					}
					//but neither is this I don't think
					else if (rowsPerProcessor > 1)
					{
						if (elementCounter == rowsPerProcessor * columns)
						{
						}
						else if (elementCounter == ((rowsPerProcessor * columns) + (columns - 1)))
						{
						}
					}
				}



				//rules governing life and death
				if (myRows[elementCounter] == 1 && (aliveCounter < 2 || aliveCounter > 3))
				{
					myRows[elementCounter] = 0;
				}
				if (myRows[elementCounter] == 0 && aliveCounter == 3)
				{
					myRows[elementCounter] = 1;
				}
			}
		}

		MPI_Barrier(MPI_COMM_WORLD);

		//print crap
		if (counter % printIteration == 0)
		{
			if (myRank != 0)
        	{
        	    //send message
        	    MPI_Send(myRows, blockLength, MPI_INT, 0, 0, MPI_COMM_WORLD);
        	}
    		else if (myRank == 0)
    		{
    		    //Print my message
				cout << endl;

				for (int columnCounter = 0; columnCounter < blockLength; columnCounter++)
				{
					cout << myRows[columnCounter];
					if (columnCounter % columns == columns -1)
					{
						cout << endl;
					}
				}

				int printedRows = rowsPerProcessor;
    		    for (int q = 1; q < commSize; q++)
				{
    				//Receive message from process q
    				MPI_Recv(printRow, blockLength, MPI_INT, q, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					for (int columnCounter = 0; columnCounter < blockLength; columnCounter++)
					{
						if (printedRows < rows)
						{
							cout << printRow[columnCounter];
							if (columnCounter % columns == columns - 1)
							{
								cout << endl;
								printedRows++;
							}
						}
					}
					//cout << endl;
	    		}
			}
   		}
	}

	MPI_Finalize();

	delete[] aliveArray;
	delete[] myRows;
	delete[] rowAbove;
	delete[] rowBelow;


	return 0;
}

void generateAlive(int* alive, int numberAlive, int rows, int columns)
{
	srand(time(NULL));
	for (int i = 0; i < numberAlive; i++)
	{
		alive[i] = rand() % (rows * columns);
	}

	return;
}

void fillGrid(int* gridRow, int* alive, int numAlive, int myRank, int rowsPerProcess, int columns)
{
	int totalElementsPerProcess = rowsPerProcess * columns;

	for (int i = 0; i < numAlive; i++)
	{
		if (myRank == alive[i] / totalElementsPerProcess)
		{
			gridRow[alive[i] % totalElementsPerProcess] = 1;
		}
	}
}
