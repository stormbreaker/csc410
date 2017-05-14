#include <stdio.h>
#include <mpi.h>
#include <string.h>

const int MAX_STRING = 100;

void printinorder();

int main(void)
{
	printinorder();
	return 0;
}

void printinorder()
{
	int my_rank, comm_sz;
	char greeting[MAX_STRING];

	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	if (my_rank != 0)
	{ 
	    /* Create message */
    	sprintf(greeting, "Proc %d of %d > Does anyone have a toothpick?", 
            my_rank, comm_sz);
      /* Send message to process 0 */
    	  MPI_Send(greeting, strlen(greeting)+1, MPI_CHAR, 0, 0,
            MPI_COMM_WORLD); 
   	}
	else
	{  
      /* Print my message */
      printf("Proc %d of %d > Does anyone have a toothpick?\n", my_rank, comm_sz);
      for (int q = 1; q < comm_sz; q++) 
	  {
         /* Receive message from process q */
         MPI_Recv(greeting, MAX_STRING, MPI_CHAR, q,
            0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
         /* Print message from process q */
         printf("%s\n", greeting);
		}
	}

	MPI_Finalize();
}
