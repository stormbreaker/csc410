/*
 * File:     mpi_odd_even.c
 * Purpose:  Implement parallel odd-even sort of an array of 
 *           nonegative ints
 * Input:
 *    A:     elements of array (optional)
 * Output:
 *    A:     elements of A after sorting
 *
 * Compile:  mpicc -g -Wall -o mpi_odd_even mpi_odd_even.c
 * Run:
 *    mpiexec -n <p> mpi_odd_even <g|i> <global_n> 
 *       - p: the number of processes
 *       - g: generate random, distributed list
 *       - i: user will input list on process 0
 *       - global_n: number of elements in global list
 *
 * Notes:
 * 1.  Except for debug output, process 0 does all I/O
 * 2.  Optional -DDEBUG compile flag for verbose output
 *
 * IPP:  Section 3.7.2 (pp. 129 and ff.)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

/* Local functions */
void Usage(char* program);
void Print_list(int local_A[], int local_n, int rank);
void Merge_low(int local_A[], int temp_B[], int temp_C[], 
         int local_n, int partner_n);
void Merge_high(int local_A[], int temp_B[], int temp_C[], 
        int local_n, int partner_n);
void Generate_list(int local_A[], int global_n, int my_rank);
int  Compare(const void* a_p, const void* b_p);
void Permute_array(int *A, int n);

/* Functions involving communication */
void Get_args(int argc, char* argv[], int* global_n_p, int* local_n_p, 
         char* gi_p, int my_rank, int p, MPI_Comm comm);
void Sort(int local_A[], int global_n, int my_rank, 
         int p, MPI_Comm comm);
void Odd_even_iter(int local_A[], int temp_B[], int temp_C[],
         int local_n, int partner_n, int phase, int even_partner, int odd_partner,
         int my_rank, int p, MPI_Comm comm);
void Print_local_lists(int local_A[], int global_n, 
         int my_rank, int p, MPI_Comm comm);
void Print_global_list(int local_A[], int global_n, int my_rank,
         int p, MPI_Comm comm);
void Read_list(int local_A[], int global_n, int my_rank, int p,
         MPI_Comm comm);

/* User defined Macros */
#define BLOCK_LOW(id,p,n)	((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n)	(BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n)	\
		(BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)

int all_n;

/*-------------------------------------------------------------------*/
int main(int argc, char* argv[]) 
{
   int my_rank, p;
   char g_i;
   int *local_A;
   int global_n;
   int local_n;
   MPI_Comm comm;

   MPI_Init(&argc, &argv);
   comm = MPI_COMM_WORLD;
   MPI_Comm_size(comm, &p);
   MPI_Comm_rank(comm, &my_rank);

   Get_args(argc, argv, &global_n, &local_n, &g_i, my_rank, p, comm);
   local_A = (int*) malloc(local_n*sizeof(int));
   if (g_i == 'g') {
      Generate_list(local_A, global_n, my_rank);
      Print_local_lists(local_A, global_n, my_rank, p, comm);
   } 
   else 
   {
      Read_list(local_A, global_n, my_rank, p, comm);
#     ifdef DEBUG
	  if(my_rank == 0)
		  printf("Unsorted local lists:\n");
      Print_local_lists(local_A, global_n, my_rank, p, comm);
#     endif
   }

#  ifdef DEBUG
   // printf("Proc %d > Before Sort\n", my_rank);
   fflush(stdout);
#  endif
   Sort(local_A, global_n, my_rank, p, comm);

#  ifdef DEBUG
   if(my_rank == 0)
      printf("Sorted local lists:\n");
   Print_local_lists(local_A, global_n, my_rank, p, comm);
   fflush(stdout);
#  endif

   Print_global_list(local_A, global_n, my_rank, p, comm);

   free(local_A);

   MPI_Finalize();

   return 0;
}  /* main */


/*-------------------------------------------------------------------
 * Function:   Generate_list
 * Purpose:    Fill list with random ints
 * Input Args: local_n, my_rank
 * Output Arg: local_A
 */
void Generate_list(int local_A[], int global_n, int my_rank) 
{
   int i, p;
   int *temp = NULL, *displs = NULL, *scounts = NULL;

   MPI_Comm_size(MPI_COMM_WORLD, &p);

   if (my_rank == 0) {
      srandom(0);
      temp = (int*) malloc(global_n*sizeof(int));
      for (i = 0; i < global_n; i++)
         temp[i] = i;
      Permute_array(temp, global_n);
      // Create arrays for displacements and send counts
      displs = (int *)malloc(p*sizeof(int)); 
      scounts = (int *)malloc(p*sizeof(int));
      for (i=0; i<p; ++i) 
      {
         // Displacement is equal to the beginning of the block 
         displs[i] = BLOCK_LOW(i, p, global_n);
         // Send count is equal to the local_n for each process
         scounts[i] = BLOCK_SIZE(i, p, global_n); 
      }
	   MPI_Scatterv(temp, scounts, displs, MPI_INT, local_A, 
            BLOCK_SIZE(my_rank, p, global_n), MPI_INT, 0, MPI_COMM_WORLD);
      free(temp); free(scounts); free(displs);
   }
   else 
	   MPI_Scatterv(temp, scounts, displs, MPI_INT, local_A, 
            BLOCK_SIZE(my_rank, p, global_n), MPI_INT, 0, MPI_COMM_WORLD);

}  /* Generate_list */


/*-------------------------------------------------------------------
 * Function:  Usage
 * Purpose:   Print command line to start program
 * In arg:    program:  name of executable
 * Note:      Purely local, run only by process 0;
 */
void Usage(char* program) 
{
   fprintf(stderr, "usage:  mpirun -np <p> %s <g|i> <global_n>\n",
       program);
   fprintf(stderr, "   - p: the number of processes \n");
   fprintf(stderr, "   - g: generate random, distributed list\n");
   fprintf(stderr, "   - i: user will input list on process 0\n");
   fprintf(stderr, "   - global_n: number of elements in global list");
   fflush(stderr);
}  /* Usage */


/*-------------------------------------------------------------------
 * Function:    Get_args
 * Purpose:     Get and check command line arguments
 * Input args:  argc, argv, my_rank, p, comm
 * Output args: global_n_p, local_n_p, gi_p
 */
void Get_args(int argc, char* argv[], int* global_n_p, int* local_n_p, 
         char* gi_p, int my_rank, int p, MPI_Comm comm) 
{
   if (my_rank == 0) 
   {
      if (argc != 3) 
      {
         Usage(argv[0]);
         *global_n_p = -1;  /* Bad args, quit */
      } 
      else 
      {
         *gi_p = argv[1][0];
         if (*gi_p != 'g' && *gi_p != 'i') 
         {
            Usage(argv[0]);
            *global_n_p = -1;  /* Bad args, quit */
         } 
         else 
         {
            *global_n_p = atoi(argv[2]);
         }
      }
   }  /* my_rank == 0 */

   MPI_Bcast(gi_p, 1, MPI_CHAR, 0, comm);
   MPI_Bcast(global_n_p, 1, MPI_INT, 0, comm);

   if (*global_n_p <= 0) {
      MPI_Finalize();
      exit(-1);
   }
   all_n = *global_n_p;

   *local_n_p = BLOCK_SIZE(my_rank,p,*global_n_p);
#  ifdef DEBUG
   int local_n;
   if (my_rank == 0) {
      int q;
      printf("Proc %d > gi = %c, global_n = %d, local_n = %d\n",
      my_rank, *gi_p, *global_n_p, *local_n_p);
      for (q = 1; q < p; q++) 
      {
         MPI_Recv(&local_n, 1, MPI_INT, q, 0, comm, MPI_STATUS_IGNORE);
         printf("Proc %d > gi = %c, global_n = %d, local_n = %d\n",
             q, *gi_p, *global_n_p, local_n);
      }
   } 
   else 
   {
      local_n = BLOCK_SIZE(my_rank,p,*global_n_p);
      MPI_Send(&local_n, 1, MPI_INT, 0, 0, comm);
   }
   fflush(stdout);
#  endif

}  /* Get_args */


/*-------------------------------------------------------------------
 * Function:   Read_list
 * Purpose:    process 0 reads the list from stdin and scatters it
 *             to the other processes.
 * In args:    global_n, my_rank, p, comm
 * Out arg:    local_A
 */
void Read_list(int local_A[], int global_n, int my_rank, int p,
         MPI_Comm comm) 
{
   int i;
   int *temp = NULL, *displs = NULL, *scounts = NULL;

   if (my_rank == 0) {
      temp = (int*) malloc(global_n*sizeof(int));
      printf("Enter the elements of the list\n");
      for (i = 0; i < global_n; i++)
         scanf("%d", &temp[i]);
      // Create arrays for displacements and send counts
      displs = (int *)malloc(p*sizeof(int)); 
      scounts = (int *)malloc(p*sizeof(int));
      for (i=0; i<p; ++i) 
      {
         // Displacement is equal to the beginning of the block 
         displs[i] = BLOCK_LOW(i, p, global_n);
         // Send count is equal to the local_n for each process
         scounts[i] = BLOCK_SIZE(i, p, global_n); 
      }
	   MPI_Scatterv(temp, scounts, displs, MPI_INT, local_A, 
            BLOCK_SIZE(my_rank, p, global_n), MPI_INT, 0, MPI_COMM_WORLD);
      free(temp);
   } 
   else
	   MPI_Scatterv(temp, scounts, displs, MPI_INT, local_A, 
            BLOCK_SIZE(my_rank, p, global_n), MPI_INT, 0, MPI_COMM_WORLD);
}  /* Read_list */


/*-------------------------------------------------------------------
 * Function:   Print_global_list
 * Purpose:    Print the contents of the global list A
 * Input args: local_A, global_n, my_rank, p, comm 
 * Out arg:    none
 *    
 * Note:       A, the list is purely local, called only by process 0
 */
void Print_global_list(int local_A[], int global_n, int my_rank, int p, 
      MPI_Comm comm) 
{
   int* A = NULL, *displs = NULL, *rcounts = NULL;
   int i;

   if (my_rank == 0) 
   {
      A = (int*) malloc(global_n*sizeof(int));
      // Create arrays for displacements and receive counts
      displs = (int *)malloc(p*sizeof(int)); 
      rcounts = (int *)malloc(p*sizeof(int));
      for (i=0; i<p; ++i) 
      {
         // Displacement is equal to the beginning of the block 
         displs[i] = BLOCK_LOW(i, p, global_n);
         // Receive count is equal to the local_n for each process
         rcounts[i] = BLOCK_SIZE(i, p, global_n); 
      }
      MPI_Gatherv(local_A, BLOCK_SIZE(my_rank, p, global_n), MPI_INT, A, 
            rcounts, displs, MPI_INT, 0, comm);
      printf("Global list:\n");
      for (i = 0; i < global_n; i++)
         printf("%d ", A[i]);
      printf("\n\n");
      free(A);
   }
   else
      MPI_Gatherv(local_A, BLOCK_SIZE(my_rank, p, global_n), MPI_INT, A, 
            rcounts, displs, MPI_INT, 0, comm);

}  /* Print_global_list */

/*-------------------------------------------------------------------
 * Function:    Compare
 * Purpose:     Compare 2 ints, return -1, 0, or 1, respectively, when
 *              the first int is less than, equal, or greater than
 *              the second.  Used by qsort.
 */
int Compare(const void* a_p, const void* b_p) 
{
   int a = *((int*)a_p);
   int b = *((int*)b_p);

   if (a < b)
      return -1;
   else if (a == b)
      return 0;
   else /* a > b */
      return 1;
}  /* Compare */

/*-----------------------------------------------------------------
 * Function:  Permute_array
 * Purpose:   Use random number generator to create a permutation of 
 *            the array with a uniform distribution.
 * In args:   n
 * Out args:  a
 */
void Permute_array(int *a, int n)
{
   int i, j, k;
   
   for(i=0; i<(n-1); i++) 
   {
      j = i+(int)((n-i)*(rand()/(RAND_MAX+1.0)));
      k = a[i];
      a[i] = a[j];
      a[j] = k;
   }
}  /* Permute_array */

/*-------------------------------------------------------------------
 * Function:    Sort
 * Purpose:     Sort local list, use odd-even sort to sort
 *              global list.
 * Input args:  global_n, my_rank, p, comm
 * In/out args: local_A 
 */
void Sort(int local_A[], int global_n, int my_rank, 
         int p, MPI_Comm comm) 
{
   int phase, local_n = 0, partner_n = 0;
   int *temp_B, *temp_C;
   int even_partner;  /* phase is even or left-looking */
   int odd_partner;   /* phase is odd or right-looking */

   /* Temporary storage used in merge-split */
   local_n = BLOCK_SIZE(my_rank, p, global_n);
   temp_C = (int*) malloc(local_n*sizeof(int));

   /* Find partners:  negative rank => do nothing during phase */
   if (my_rank % 2 != 0) 
   {
      even_partner = my_rank - 1;
      odd_partner = my_rank + 1;
      if (odd_partner == p) odd_partner = MPI_PROC_NULL;  // Idle during odd phase
   } 
   else 
   {
      even_partner = my_rank + 1;
      if (even_partner == p) even_partner = MPI_PROC_NULL;  // Idle during even phase
      odd_partner = my_rank-1;
      if (odd_partner < 0) odd_partner = MPI_PROC_NULL;  // Idle during odd phase

   }

   /* Sort local list using built-in quick sort */
   qsort(local_A, BLOCK_SIZE(my_rank, p, global_n), sizeof(int), Compare);

#  ifdef DEBUG
   // printf("Proc %d > before loop in sort\n", my_rank);
   fflush(stdout);
#  endif

   for (phase = 0; phase < 2*p; phase++)
   {
      if (phase % 2 == 0)
         partner_n = BLOCK_SIZE(even_partner, p, global_n);
      else
         partner_n = BLOCK_SIZE(odd_partner, p, global_n);
      temp_B = (int*) malloc(partner_n*sizeof(int));
      Odd_even_iter(local_A, temp_B, temp_C, local_n, partner_n, phase, 
         even_partner, odd_partner, my_rank, p, comm);
      free(temp_B);
   }

   free(temp_C);

}  /* Sort */


/*-------------------------------------------------------------------
 * Function:    Odd_even_iter
 * Purpose:     One iteration of Odd-even transposition sort
 * In args:     local_n, phase, my_rank, p, comm
 * In/out args: local_A
 * Scratch:     temp_B, temp_C
 */
void Odd_even_iter(int local_A[], int temp_B[], int temp_C[],
        int local_n, int partner_n, int phase, int even_partner, int odd_partner,
        int my_rank, int p, MPI_Comm comm) 
{
   MPI_Status status;

   if (phase % 2 == 0) 
   {
      if (even_partner >= 0) 
      {
         MPI_Sendrecv(local_A, local_n, MPI_INT, even_partner, 0, 
            temp_B, partner_n, MPI_INT, even_partner, 0, comm,
            &status);
         if (my_rank % 2 != 0)
            Merge_high(local_A, temp_B, temp_C, local_n, partner_n);
         else
            Merge_low(local_A, temp_B, temp_C, local_n, partner_n);
      }
   } 
   else 
   { /* odd phase */
      if (odd_partner >= 0) 
	   {
         MPI_Sendrecv(local_A, local_n, MPI_INT, odd_partner, 0, 
            temp_B, partner_n, MPI_INT, odd_partner, 0, comm,
            &status);
         if (my_rank % 2 != 0)
            Merge_low(local_A, temp_B, temp_C, local_n, partner_n);
         else
            Merge_high(local_A, temp_B, temp_C, local_n, partner_n);
      }
   }
#  ifdef DEBUG
   if(my_rank == 0)
   {
      int i, q;
      int *A;
      printf("Phase: %d\n", phase);
      printf("%d: ", my_rank);
      for(i=0; i<local_n; i++)
         printf("%d ", local_A[i]);
      printf("\n");
      for(q=1; q<p; q++)      
      {
         A = (int*) malloc(BLOCK_SIZE(q, p, all_n)*sizeof(int));
         MPI_Recv(A, BLOCK_SIZE(q, p, all_n), MPI_INT, q, 0, comm, &status);
         printf("%d: ", q);
         for(i=0; i<BLOCK_SIZE(q, p, all_n); i++)
            printf("%d ", A[i]);
         printf("\n");
         free(A);
      }
   }
   else
      MPI_Send(local_A, BLOCK_SIZE(my_rank, p, all_n), MPI_INT, 0, 0, comm);
#  endif
}  /* Odd_even_iter */


/*-------------------------------------------------------------------
 * Function:    Merge_low
 * Purpose:     Merge the smallest local_n elements in local_A 
 *              and temp_B into temp_C.  Then copy temp_C
 *              back into local_A.
 * In args:     local_n, partner_n, temp_B
 * In/out args: local_A
 * Scratch:     temp_C
 */
void Merge_low(int local_A[], int temp_B[], int temp_C[], 
        int local_n, int partner_n)
{
   int ai, bi, ci;
   
   ai = bi = ci = 0;

   while (ci < local_n) 
   {
      if (local_A[ai] <= temp_B[bi] || bi == partner_n) 
	  {
         temp_C[ci] = local_A[ai];
         ci++; ai++;
      } 
      else 
	  {
         temp_C[ci] = temp_B[bi];
         ci++; bi++;
      }   
   }

   memcpy(local_A, temp_C, local_n*sizeof(int));
}  /* Merge_low */

/*-------------------------------------------------------------------
 * Function:    Merge_high
 * Purpose:     Merge the largest local_n elements in local_A 
 *              and temp_B into temp_C.  Then copy temp_C
 *              back into local_A.
 * In args:     local_n, partner_n, temp_B
 * In/out args: local_A
 * Scratch:     temp_C
 */
void Merge_high(int local_A[], int temp_B[], int temp_C[], 
        int local_n, int partner_n) 
{
   int ai, bi, ci;
   
   ai = local_n-1;
   bi = partner_n-1;
   ci = local_n-1;

   while (ci >= 0) 
   {
      if (local_A[ai] >= temp_B[bi] || bi < 0) 
	  {
         temp_C[ci] = local_A[ai];
         ci--; ai--;
      } 
      else 
	  {
         temp_C[ci] = temp_B[bi];
         ci--; bi--;
      }
   }

   memcpy(local_A, temp_C, local_n*sizeof(int));
}  /* Merge_high */


/*-------------------------------------------------------------------
 * Only called by process 0
 */
void Print_list(int local_A[], int local_n, int rank) {
   int i;
   printf("%d: ", rank);
   for (i = 0; i < local_n; i++)
      printf("%d ", local_A[i]);
   printf("\n");
}  /* Print_list */

/*-------------------------------------------------------------------
 * Function:   Print_local_lists
 * Purpose:    Print each process' current list contents
 * Input args: all
 * Notes:
 * 1.  Assumes all participating processes are contributing
 *     BLOCK_SIZE(my_rank,p,global_n) elements
 */
void Print_local_lists(int local_A[], int global_n, 
         int my_rank, int p, MPI_Comm comm) {
   int*       A;
   int        q;
   MPI_Status status;

   if (my_rank == 0) {
      A = (int*) malloc((global_n/p+1)*sizeof(int));
      Print_list(local_A, BLOCK_SIZE(my_rank, p, global_n), my_rank);
      for (q = 1; q < p; q++) {
         MPI_Recv(A, BLOCK_SIZE(q, p, global_n), MPI_INT, q, 0, comm, &status);
         Print_list(A, BLOCK_SIZE(q, p, global_n), q);
      }
      free(A);
   } 
   else
      MPI_Send(local_A, BLOCK_SIZE(my_rank, p, global_n), MPI_INT, 0, 0, comm);

}  /* Print_local_lists */
