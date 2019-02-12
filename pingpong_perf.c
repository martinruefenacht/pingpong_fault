#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_WARMUP 1000
#define BLOCK_SIZE 5
#define START_VALUE 19

#define TARGET 0
#define SOURCE 1
#define TAG 9731

struct timespec diff(struct timespec end, struct timespec start)
{
	struct timespec temp;

	if((end.tv_nsec - start.tv_nsec) < 0)
	{
		temp.tv_sec = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	} 
	else 
	{
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}
	return temp;
}

int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);

	int max_iterations = atoi(argv[1]);

    int size, rank;
    int start, msg;
    int code = MPI_SUCCESS;

    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(size > 2)
    {
    	MPI_Abort(MPI_COMM_WORLD, 100);
    }

	// initial payload
	if(rank == SOURCE)
		msg = START_VALUE;
	else
		msg = -1;

	// warm up rounds
	for (int i = 0; i < MAX_WARMUP; ++i)
    {
    	if(rank == SOURCE) 
		{
    		MPI_Send(&msg, 1, MPI_INT, TARGET, TAG, MPI_COMM_WORLD);
    		MPI_Recv(&msg, 1, MPI_INT, TARGET, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    	}
    	else
		{
    		MPI_Recv(&msg, 1, MPI_INT, SOURCE, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			msg += 1;
    		MPI_Send(&msg, 1, MPI_INT, SOURCE, TAG, MPI_COMM_WORLD);
    	}
    }

    // Print off a hello world message
    printf("Hello world from rank %d out of %d processors\n", rank, size);

	// start time
	struct timespec end, stime;
	struct timespec array[max_iterations / BLOCK_SIZE];
		
    for (int i = 0; i < max_iterations / BLOCK_SIZE; ++i)
    {
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stime);

		for(int j = 0; j < BLOCK_SIZE; ++j)
		{
    		if(rank == SOURCE) 
			{
    			MPI_Send(&msg, 1, MPI_INT, TARGET, TAG, MPI_COMM_WORLD);
    			MPI_Recv(&msg, 1, MPI_INT, TARGET, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    		}
    		else
			{
    			MPI_Recv(&msg, 1, MPI_INT, SOURCE, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				msg += 1;
    			MPI_Send(&msg, 1, MPI_INT, SOURCE, TAG, MPI_COMM_WORLD);
    		}
		}
	
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

		array[i] = diff(end, stime).tv_nsec / BLOCK_SIZE;
    }

	// end time

	printf("rank %i checksum %i == %i\n", rank, msg, START_VALUE+MAX_WARMUP+max_iterations);
	printf("rank %i iterations %i block size %i\n", rank, max_iterations, BLOCK_SIZE);

	for(size_t i = 0; i < max_iterations/BLOCK_SIZE; ++i)
	{
		printf("%li %li\n", array[i].tv_sec, array[i].tv_nsec);
	}
	
	MPI_Finalize();
}
