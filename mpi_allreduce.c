/* File:       
 *    mpi_allreduce.c
 *
 * Purpose:    
 *    An "Allreduce" program that uses MPI
 *
 * Compile:    
 *    mpicc -g -Wall -std=c99 -o mpi_allreduce mpi_allreduce.c -lm 
 * Usage:        
 *    mpiexec -np <number of processes> ./mpi_allreduce 
 *
 * Input:      
 *    None
 * Output:     
 *    Two messages that each step tells who sent a message and who
 *    was the receiver, and from which sender a receiver expects a
 *    message.
 *
 * Algorithm:  
 *    Reducing the global sum and broadcast it to all processes
 *    using a butterfly algorithm
 *
 * IPP:  Section 3.4.4 (pp. 106 and ff.)
 */
#include <stdio.h>
#include <string.h>  /* For strlen             */
#include <math.h>
#include <mpi.h>     /* For MPI functions, etc */ 

const int MAX_STRING = 100;

/* User defined functions */
void my_AllReduce(int, int, int *);
void sender(int, int, int, int *, char[]);
void receiver(int, int, int, int *, char[]);
void print_msg(int, int, char[], char[]);

int IsPowerOfTwo(int x)
{
    return (x & (x - 1)) == 0;
}

int main(void) 
{
   int	comm_sz, my_rank;	/* Number of processes, process rank	*/
   int	sum, q;				/* Global sum, counter						*/

   /* Start up MPI */
   MPI_Init(NULL, NULL); 
   /* Get the number of processes */
   MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
   /* Get my rank among all the processes */
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 
   /* Check to see if comm_sz is a power of 2 */
   if(!IsPowerOfTwo(comm_sz))
   {
	if(my_rank == 0)
	printf("The number of processes has to be a power of 2\n");
	/* Shut down MPI */
	MPI_Finalize();
	return 0;
   }
   sum = my_rank;
   /* Call the Broadcast function */
   my_AllReduce(my_rank, comm_sz, &sum);
   /* Print the global sum */
   if(my_rank != 0)
      MPI_Send(&sum, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
   else {
		printf("Rank: %2d, sum: %d\n", my_rank, sum);
		for (q = 1; q < comm_sz; q++) {
			MPI_Recv(&sum, 1, MPI_INT, q, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			printf("Rank: %2d, sum: %d\n", q, sum);
		}
   }
   /* Shut down MPI */
   MPI_Finalize(); 
   return 0;
}  /* main */

/*------------------------------------------------------------------
 * Function:     my_AllReduce
 * Purpose:      An implementation of the Allreduce function
 * Input args:   my_rank:  process rank in MPI_COMM_WORLD
 *               comm_sz:  number of processes in MPI_COMM_WORLD
 * Output args:  None
 */
void my_AllReduce(int my_rank, int comm_sz, int *sum)
{
   int	power;				/* comm_sz <= 2^power		*/
   int	step;					/* counters		  				*/
   char	msg1[MAX_STRING];	/* String holding message	*/
   char	msg2[MAX_STRING];	/* String holding message	*/

   /* Calculate the ceiling of log2 of comm_sz */
   power = (int) ceil(log2(comm_sz));

# ifdef DEBUG 
   if(my_rank == 0)
       printf("Power: %d\n", power);
# endif

   /* Reduce and send the sum in log2(comm_sz) steps */
   for(step=0; step<power; step++)
   {

# ifdef DEBUG
      /* Clean the greeting */
      sprintf(msg1, " ");
      sprintf(msg1, " ");
# endif

      /* Collect all processes before each step */
      MPI_Barrier(MPI_COMM_WORLD);
      /* As sending is non-blocking send first then receive */
      sender(my_rank, comm_sz, step, sum, msg1);
      /* Receiving is blocking so receive when we know there is a message */
      receiver(my_rank, comm_sz, step, sum, msg2);

# ifdef DEBUG   
      /* Display the debugging message */
      print_msg(my_rank, comm_sz, msg1, msg2);
# endif

   }  /* End for step=power */
}  /* End my_Bcast() */

/*------------------------------------------------------------------
 * Function:     sender
 * Purpose:      The process is a sender of a message
 * Input args:   my_rank:  process rank in MPI_COMM_WORLD
 *               comm_sz:  number of processes in MPI_COMM_WORLD
                 step:     the current step in the broadcast process
				 sum:	   partial sum
                 greeting: The greeting message
 * Output args:  None
 */
void sender(int my_rank, int comm_sz, int step, int *sum, char greeting[])
{
	if(my_rank/((int)pow(2.0, step))%2 == 0) {

# ifdef DEBUG
		/* Create a debugging message */
		sprintf(greeting, "Step: %2d \t Rank: %2d sends to      %2d", 
				  step, my_rank, my_rank+(int)pow(2.0, step));
# else
		MPI_Send(sum, 1, MPI_INT, my_rank+(int)pow(2.0, step), 0, MPI_COMM_WORLD);
# endif

	}
   else {

# ifdef DEBUG
		/* Create a debugging message */
		sprintf(greeting, "Step: %2d \t Rank: %2d sends to      %2d", 
                   step, my_rank, my_rank-(int)pow(2.0, step));
# else
		MPI_Send(sum, 1, MPI_INT, my_rank-(int)pow(2.0, step), 0, MPI_COMM_WORLD);
# endif

	}
}  /* End sender() */

/*------------------------------------------------------------------
 * Function:     reveiver
 * Purpose:      Every process is a receiver of a message
 * Input args:   my_rank:  process rank in MPI_COMM_WORLD
 *               comm_sz:  number of processes in MPI_COMM_WORLD
                 step:     the current step in the broadcast process
                 greeting: The greeting message
 * Output args:  received partial sum
 */
void receiver(int my_rank, int comm_sz, int step, int *sum, char greeting[])
{
	int tmp_sum = 0;
	if(my_rank/((int)pow(2.0, step))%2 == 0) {

# ifdef DEBUG
		/* Create a debugging message */
		sprintf(greeting, "Step: %2d \t Rank: %2d receives from %2d", 
                  step, my_rank, my_rank+(int)pow(2.0, step));
# else
		MPI_Recv(&tmp_sum, 1, MPI_INT, my_rank+(int)pow(2.0, step), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
# endif

	}
	else {

# ifdef DEBUG
		/* Create a debugging message */
		sprintf(greeting, "Step: %2d \t Rank: %2d receives from %2d", 
                  step, my_rank, my_rank-(int)pow(2.0, step));
# else
		MPI_Recv(&tmp_sum, 1, MPI_INT, my_rank-(int)pow(2.0, step), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
# endif

	}
	/* Update the sum */
	*sum += tmp_sum;
}  /* End receiver() */ 

/*------------------------------------------------------------------
 * Function:     print_msg
 * Purpose:      Create a visual output of the messages
 * Input args:   my_rank:  process rank in MPI_COMM_WORLD
 *               comm_sz:  number of processes in MPI_COMM_WORLD
                 greeting: The greeting message
 * Output args:  None
 */
void print_msg(int my_rank, int comm_sz, char msg1[], char msg2[])
{
   int q;

   /* Print message in order */
   if (my_rank != 0) {
      /* Send message to process 0 */
      MPI_Send(msg1, strlen(msg1)+1, MPI_CHAR, 0, 1,
                  MPI_COMM_WORLD);
      MPI_Send(msg2, strlen(msg2)+1, MPI_CHAR, 0, 2,
                  MPI_COMM_WORLD);
   }  /* End if my_rank != 0 */

   else {  /* This is root */
      /* Print my message */
      printf("%s\n", msg1);
	  printf("%s\n", msg2);
      /* Print the message from all the other processes */
      for (q = 1; q < comm_sz; q++) 
      {
         /* Receive message from process q */
         MPI_Recv(msg1, MAX_STRING, MPI_CHAR, q,
            1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
         MPI_Recv(msg2, MAX_STRING, MPI_CHAR, q,
            2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
         /* Print message from process q */
         printf("%s\n", msg1);
         printf("%s\n", msg2);
      }
   }  /* End if my_rank == 0 */ 
}  /* End print_msg() */
