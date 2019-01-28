#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ITERATION 33333 

struct timespec diff(struct timespec start, struct timespec end)
{
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);

    int size, rank;
    int start, msg;
    int code = MPI_SUCCESS;

    // Choose an iteration to fail
    int failed_iteration = 3;

    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(size > 2)
    {
    	MPI_Abort(MPI_COMM_WORLD, 100);
    }

	// initial payload
	if(rank == 0)
		msg = 0;
	else
		msg = -1;

    // Print off a hello world message
    printf("Hello world from rank %d out of %d processors\n", rank, size);

	// start time
	struct timespec end, stime;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stime);
		
    for (int i = 0; i < MAX_ITERATION; ++i)
    {
    	if(rank == 0) 
		{
    		MPI_Send(&msg, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    		MPI_Recv(&msg, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    	}
    	else
		{
    		MPI_Recv(&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			msg += 1;
    		MPI_Send(&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    	}
    }

	// end time
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	printf("rank %i checksum %i == %i\n", rank, msg, MAX_ITERATION);

	struct timespec d = diff(stime, end);
	printf("rank %i tot sec %li nsec %li\n", rank, d.tv_sec, d.tv_nsec);
	printf("rank %i avg sec %li nsec %li\n", rank, d.tv_sec/MAX_ITERATION, d.tv_nsec/MAX_ITERATION);

	MPI_Finalize();
}
