#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ITERATION 5

#define MPIX_SAFE_CALL(__operation, __predicate, __label) {int code = __operation; if (code != MPI_SUCCESS) {__predicate; goto __label;}}

int main_loop(int argc, char **argv, int epoch, int *done);

int Application_Checkpoint_Read(int epoch, int rank, int *start, int *payload);
void Application_Checkpoint_Write(int epoch, int rank, int start, int payload);

int main(int argc, char **argv)
{
	int code = MPI_SUCCESS;
	int abort = 0;
	int done = 0;
	int fault_epoch = 0;

	while(!abort && !done) 
	{
		switch(code) 
		{
		case MPI_SUCCESS:
			MPI_Init(&argc, &argv);
			MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
			break;

		case MPIX_TRY_RELOAD:
			MPIX_Checkpoint_read();
			break;

		default:
			break;
		}

		MPIX_Get_fault_epoch(&fault_epoch);
		code = main_loop(argc, argv, fault_epoch, &done);
	}
	MPI_Finalize();

	return 0;
}

int main_loop(int argc, char **argv, int epoch, int *done)
{
    int size, rank;
    int start, msg;
    int code = MPI_SUCCESS;

    // Choose an iteration to fail
    int failed_iteration = 3;

    // Get the number of processes
    MPIX_SAFE_CALL(MPI_Comm_size(MPI_COMM_WORLD, &size), code = MPIX_TRY_RELOAD, fail_return);
    MPIX_SAFE_CALL(MPI_Comm_rank(MPI_COMM_WORLD, &rank), code = MPIX_TRY_RELOAD, fail_return);

    if(size > 2)
    {
    	MPI_Abort(MPI_COMM_WORLD, 100);
    }

    if (epoch > 0) {
    	Application_Checkpoint_Read(epoch - 1, rank, &start, &msg);
    	failed_iteration = MAX_ITERATION + 1;
		start += 1;
    }
    else {
    	start = 0;

		// initial payload
		if(rank == 0)
			msg = 0;
		else
			msg = -1;
    }

    // Print off a hello world message
    printf("Hello world from rank %d out of %d processors at epoch %i\n", rank, size, epoch);

    // PING PONG
    for (int i = start; i < MAX_ITERATION; ++i)
    {
    	if(rank == 0) 
		{
    		// send to 1
    		printf("proc %i has %i with iteration %i\n", rank, msg, i);

    		MPIX_SAFE_CALL(MPI_Send(&msg, 1, MPI_INT, 1, 0, MPI_COMM_WORLD), code = MPIX_TRY_RELOAD, fail_return);
    		printf("proc %i sent %i\n", rank, msg);

    		// recv from 1
    		MPIX_SAFE_CALL(MPI_Recv(&msg, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE), code = MPIX_TRY_RELOAD, fail_return);
    		printf("proc %i recv %i\n", rank, msg);
    	}
    	else
		{
    		printf("proc %i has %i with iteration %i\n", rank, msg, i);

    		// recv from 0
    		MPIX_SAFE_CALL(MPI_Recv(&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE), code = MPIX_TRY_RELOAD, fail_return);
    		printf("proc %i recv %i\n", rank, msg);

			msg += 1;
			
    		// send to 0
    		MPIX_SAFE_CALL(MPI_Send(&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD), code = MPIX_TRY_RELOAD, fail_return);
    		printf("proc %i sent %i\n", rank, msg);
    	}
    	// Checkpoint at the end of each iteration
    	MPIX_SAFE_CALL(MPIX_Get_fault_epoch(&epoch), code = MPIX_TRY_RELOAD, fail_return);
    	Application_Checkpoint_Write(epoch, rank, i, msg);
		printf("proc %i saved iteration %i payload %i\n", rank, i, msg);
    	MPIX_SAFE_CALL(MPIX_Checkpoint_write(), code = MPIX_TRY_RELOAD, fail_return);
    }


    if (code == MPI_SUCCESS)
    {
    	*done = 1;
    }

    return code;

    fail_return:
    	return MPIX_TRY_RELOAD;
}

int Application_Checkpoint_Read(int epoch, int rank, int *start, int *payload) {
	FILE *fp;
	char buf[10];
	sprintf(buf, "check_%d_%d", epoch, rank);
	if ((fp = fopen(buf, "rb")) == NULL) 
	{
		printf("ERROR: Opening File");
	}

	fread(start, sizeof(int), 1, fp);
	fread(payload, sizeof(int), 1, fp);
	fclose(fp);
	return 0;
}

void Application_Checkpoint_Write(int epoch, int rank, int start, int payload) {
	FILE *fp;
	char buf[10];
	sprintf(buf, "check_%d_%d", epoch, rank);
	if ((fp = fopen(buf, "wb")) == NULL)
	{
		printf("ERROR: Opening File\n");
	}

	fwrite(&start, sizeof(int), 1, fp);
	fwrite(&payload, sizeof(int), 1, fp);
	
	fclose(fp);
}
