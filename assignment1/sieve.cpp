#include <iomanip>
#include <iostream>
#include <omp.h>
#include <vector>
#include <ctime>

/*
 * Author: Benjamin Kaiser
 * CSC 410 - Parallel Programming
 * Dr. Christer Karlsson
 *
 * This program is a program which compares three different ways of doing the Sieve of Eratosthenes.
 * 
 * The first implementation is a serial portion which loops through the numbers which need checked.
 * It contains an innter loop which then increments through the loop again with an increment of
 * the number it is checking and essentially eliminates them from the list by setting them to 1.
 *
 * The second implementation parallelizes this algorithm using the OpenMP library and schedules things
 * statically.
 *
 * The final implementation parallelizes the algorithm using the OpenMP library and schedules dynamically
 */

using namespace std;

int main(int argc, char** argv)
{
	vector<long int> numberList;

	//usage statement
	if (argc != 2)
	{
		cout << "./prime <upper limit>" << endl;
		return 0;
	}

	// overhead for filling the list
	long int upperLimit = strtol(argv[1], NULL, 10);

	for (long int i = 0; i <= upperLimit; i++)
	{
		numberList.push_back(i);
	}


	double serialstarttime, serialendtime;
	double staticstarttime, staticendtime;
	double dynamicstarttime, dynamicendtime;



	int thread_count = 8; //hard coded as per specifications but I'd much prefer omp_get_num_procs();


	int length = numberList.size();

	// serial
	serialstarttime = omp_get_wtime();
	for (long int i = 2; i < length; i++)
	{
		if (numberList[i] != 1)
		{
			for (long int j = 2 * i; j < length; j += i)
			{
				numberList[j] = 1;
			}
		}
	}
	serialendtime = omp_get_wtime();
	

	//static
	staticstarttime = omp_get_wtime();

	# pragma omp parallel for num_threads(thread_count) schedule(static,1)
	for (long int i = 2; i < length; i++)
	{
		if (numberList[i] != 1)
		{
			for (long int j = 2 * i; j < length; j += i)
			{
				numberList[j] = 1;
			}
		}
	}

	staticendtime = omp_get_wtime();


	//dynamic
	dynamicstarttime = omp_get_wtime();
	# pragma omp parallel for num_threads(thread_count) schedule(dynamic,1)
	for (long int i = 2; i < length; i++)
	{
		if (numberList[i] != 1)
		{
			for (long int j = 2 * i; j < length; j += i)
			{
				numberList[j] = 1;
			}
		}
	}
	dynamicendtime = omp_get_wtime();

	vector<long int> answers;


	//put the answers in their own array for output
	for (int i = 2; i < numberList.size(); i++)
	{
		if (numberList[i] != 1)
		{
			answers.push_back(numberList[i]);
		}
	}

	//output
	for (int i = 0; i < answers.size(); i++)
	{
		if (i % 10 == 0)
		{
			cout << endl << i << ":  ";
		}
		cout << answers[i] << " ";
	}

	cout << endl << endl;

	// timing output
	cout << endl << "Serial:  " << (serialendtime - serialstarttime) * 1000 << endl;
	cout << "Static:  " << (staticendtime - staticstarttime) * 1000 << endl;
	cout << "Dynamic:  " << (dynamicendtime - dynamicstarttime) * 1000 << endl;

	return 0;
}
