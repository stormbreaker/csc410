#include <iomanip>
#include <iostream>
#include <omp.h>

/*
 * CSC 410 - Parallel Programming
 * Dr. Christer Karlsson
 * Benjamin Kaiser
 *
 * This program is a test program written to compare three different algorithms for
 * the circuit satisfiability problem.  
 * The first one is a brute force serial approach which passes in 2^16 different possible
 * inputs which the check_circuit code written by Dr. Karlsson spits out an answer for each
 * of these.  It prints the correct answer if it passes.
 *
 * The next algorithm is the same algorithm but parallelized with the OpenMP library.
 * It uses a static scheduler to schedule one input at a time to a thread.  
 *
 * The final algorithm is the same algorithm and is parallelized with OpenMP but uses
 * a dynamic scheduler to schedule one input at a time to a thread.
 * */


using namespace std;


/* Return 1 if 'i'th bit of 'n' is 1; 0 otherwise */
#define EXTRACT_BIT(n,i) ((n&(1<<i))?1:0)


/* Check if a given input produces TRUE (a one) */
int check_circuit (int id, int z)
{
	int v[16]; /* Each element is a bit of z */
	int i;
	for (i = 0; i < 16; i++) v[i] = EXTRACT_BIT(z,i);
	{
		if ((v[0] || v[1]) && (!v[1] || !v[3]) && (v[2] || v[3])
		&& (!v[3] || !v[4]) && (v[4] || !v[5])
		&& (v[5] || !v[6]) && (v[5] || v[6])
		&& (v[6] || !v[15]) && (v[7] || !v[8])
		&& (!v[7] || !v[13]) && (v[8] || v[9])
		&& (v[8] || !v[9]) && (!v[9] || !v[10])
		&& (v[9] || v[11]) && (v[10] || v[11])
		&& (v[12] || v[13]) && (v[13] || !v[14])
		&& (v[14] || v[15])) 
		{

			printf ("%d) %d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d\n", id,
			v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7],v[8],v[9],
			v[10],v[11],v[12],v[13],v[14],v[15]);


			fflush (stdout);

			return 1;
		}
		else
		{
			return 0;
		}
	}
}

int main()
{

	double serialstarttime = omp_get_wtime();
	

	//begin serial
	for (int i = 0; i < 65536; i++)
	{
		check_circuit(omp_get_thread_num(), i);
	}

	double serialendtime = omp_get_wtime();

	cout << endl;

	int thread_count = omp_get_num_procs();

	double staticstarttime = omp_get_wtime();


	//begin static
	# pragma omp parallel for num_threads(thread_count) schedule(static,1)
	for (int i = 0; i < 65536; i++)
	{
		check_circuit(omp_get_thread_num(), i);
	}

	double staticendtime = omp_get_wtime();

	cout << endl;

	double dynamicstarttime = omp_get_wtime();	


	//begin dynamice
	# pragma omp parallel for num_threads(thread_count) schedule(dynamic,1)
	for (int i = 0; i < 65536; i++)
	{
		check_circuit(omp_get_thread_num(), i);
	}

	double dynamicendtime = omp_get_wtime();
	

	cout << endl;

	// timing outputs

	cout << "Serial: " << (serialendtime - serialstarttime) * 1000 << " ms" << endl;

	cout << "Static Schedule: " << (staticendtime - staticstarttime) * 1000 << " ms" << endl;

	cout << "Dynamic Schedule: " << (dynamicendtime - dynamicstarttime) * 1000 << " ms" << endl;
	return 0;
}
