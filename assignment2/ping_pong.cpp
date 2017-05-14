#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <mpi.h>
#include <chrono>
#include <string>
#include <cstring>

/*
This program is a test of the networks bandwidth by using a ping pong program to time when a 
message is sent to another machine and then when the other machine sends that message back.  
Dr. Karlsson wanted us to implement blocking, sendrecv, and nonblocking versions of the ping-pong algorithm.  
This program consists of four functions including main.  The other three are the different implementations 
of the ping-pong program.  The first function called “normal” or “regular” is the normal blocking implementation 
of the ping-pong program.  The second function called “sendrecv” is the SendRecv version of the program.  
The final function called “nonblocking” is what I believe to be the non-blocking version of the ping pong program.  
*/


using namespace std;

// MPI_Ssend and MPI_Recv
void regular(int myRank, char* myMessage, int currentSize);
// MPI_SendRecv
void sendrecv(int myRank, char* myMessage, int currentSize);
// MPI_Isend and MPI_Irecv
void nonblocking(int myRank, char* myMessage, int currentSize);

const int MAX_STRING = 4000000;
const int ITERATIONS = 5000;

int main()
{
	int commSize;
	int myRank;

	int currentMessageSize = 64;

	char message[MAX_STRING];
/*
	for (int i = 0; i < MAX_STRING  - 1; i++)
	{
		message[i] = 'B';
	}
*/


	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &commSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

	//while (currentMessageSize < MAX_STRING)
	//{
		for (int k = 0; k < currentMessageSize; k++)
		{
			message[k] = 'B';	
		}
		MPI_Barrier(MPI_COMM_WORLD);
		//cout << "Normal at " << currentMessageSize << " bytes" << endl;
		//for (int i = 0; i < ITERATIONS; i++)
		//{	
			regular(myRank, message, currentMessageSize);
		//}
		MPI_Barrier(MPI_COMM_WORLD);
		//cout << "SendRecv at " << currentMessageSize << " bytes" << endl;
		//for (int i = 0; i < ITERATIONS; i++)
		//{
			sendrecv(myRank, message, currentMessageSize);
		//}	
		MPI_Barrier(MPI_COMM_WORLD);
		//cout << "Nonblocking at " << currentMessageSize << " bytes" << endl;
		//for (int i = 0; i < ITERATIONS; i++)
		//{
			nonblocking(myRank, message, currentMessageSize);
		//}

//		currentMessageSize *= 2;

	//}
	
	MPI_Finalize();	
}

/*
	Blocking Implementation
*/
void regular(int myRank, char* myMessage, int currentSize)
{
	double startTime, endTime;
	char newMessage[MAX_STRING];
	if (myRank == 0)
	{
		startTime = MPI_Wtime();
		MPI_Ssend(myMessage, currentSize, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
		MPI_Recv(newMessage, currentSize, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		endTime = MPI_Wtime();

		if (strcmp(newMessage, myMessage) == 0)
		{
			cout << "Normal at " << currentSize << " bytes" << endl;
			cout << ((endTime - startTime) / 2) * 1000 << endl;//" ms" << endl;
		}
	}
	else if (myRank == 1)
	{
		MPI_Recv(myMessage, currentSize, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Send(myMessage, currentSize, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
	}
}
/*
	SendRecv Implementation
*/
void sendrecv(int myRank, char* myMessage, int currentSize)
{
	char myNewMessage[MAX_STRING];
	double startTime, endTime;

	if (myRank == 0)
	{
		startTime = MPI_Wtime();
		MPI_Sendrecv(myMessage, currentSize, MPI_CHAR, 1, 0, myNewMessage, currentSize, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 	
		endTime = MPI_Wtime();
		if (strcmp(myNewMessage, myMessage) == 0)
		{
			cout << "SendRecv at " << currentSize << " bytes" << endl;
			cout << ((endTime - startTime) / 2) * 1000 << endl;//" ms" << endl;
		}
	}
	else if (myRank == 1)
	{
		MPI_Sendrecv(myMessage, currentSize, MPI_CHAR, 0, 0, myNewMessage, currentSize, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
}

/*
	Nonblocking Implementation
*/
void nonblocking(int myRank, char* myMessage, int currentSize)
{
	double startTime, endTime;
	char myNewMessage[MAX_STRING];

	MPI_Request requests[2];

	if (myRank == 0)
	{
		startTime = MPI_Wtime();
		MPI_Isend(myMessage, currentSize, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &requests[0]);
		MPI_Wait(&requests[0], MPI_STATUS_IGNORE);
		MPI_Irecv(myNewMessage, currentSize, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &requests[1]);
		endTime = MPI_Wtime();
		if (strcmp(myNewMessage, myMessage) == 0)
		{
			cout << "Non Blocking at " << currentSize << " bytes" << endl;
			cout << ((endTime - startTime) / 2) * 1000 << endl; //" ms" << endl;
		}
	}
	else if (myRank == 1)
	{
		MPI_Irecv(myMessage, currentSize, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &requests[0]);
		MPI_Wait(&requests[0], MPI_STATUS_IGNORE);
		MPI_Isend(myMessage, currentSize, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &requests[1]);
	}
}
