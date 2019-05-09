#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ITERATION 5

#define MPIX_SAFE_CALL(__operation, __predicate, __label) {int code = __operation; if (code != MPI_SUCCESS) {__predicate; goto __label;}}

void inject_fault(int rank);
int main_loop(int argc, char **argv, int epoch, int *done);
void application_checkpoint_read(int epoch, int rank, int *start, int *payload);
void application_checkpoint_write(int epoch, int rank, int start, int payload);

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
			MPI_Abort(MPI_COMM_WORLD, 100);
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

    // Randomly choose an iteration to fail between 0 and MAX_ITERATION - 1
    srand(time(NULL));
    int failed_iteration = rand() % MAX_ITERATION;

    MPIX_SAFE_CALL(MPI_Comm_size(MPI_COMM_WORLD, &size), code = MPIX_TRY_RELOAD, fail_return);
    MPIX_SAFE_CALL(MPI_Comm_rank(MPI_COMM_WORLD, &rank), code = MPIX_TRY_RELOAD, fail_return);

    if(size > 2)
    {
    	MPI_Abort(MPI_COMM_WORLD, 100);
    }

    if (epoch > 0) {
    	application_checkpoint_read(epoch - 1, rank, &start, &msg);

	// NOTE this disables failing again
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

    printf("Hello world from rank %d out of %d processors at epoch %i\n", rank, size, epoch);

    // PING PONG
    for (int i = start; i < MAX_ITERATION; ++i)
    {
    	if(rank == 0) 
	{
    		MPIX_SAFE_CALL(MPI_Send(&msg, 1, MPI_INT, 1, 0, MPI_COMM_WORLD), code = MPIX_TRY_RELOAD, fail_return);
    		MPIX_SAFE_CALL(MPI_Recv(&msg, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE), code = MPIX_TRY_RELOAD, fail_return);
		
    		printf("proc %i recv %i\n", rank, msg);
    	}
    	else
	{
    		MPIX_SAFE_CALL(MPI_Recv(&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE), code = MPIX_TRY_RELOAD, fail_return);
    		printf("proc %i recv %i\n", rank, msg);

		msg += 1;
			
		// inject fault
    		if(i == failed_iteration) 
		{
			inject_fault(rank);
    		}

    		MPIX_SAFE_CALL(MPI_Send(&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD), code = MPIX_TRY_RELOAD, fail_return);
    	}
    	// Checkpoint at the end of each iteration
    	MPIX_SAFE_CALL(MPIX_Get_fault_epoch(&epoch), code = MPIX_TRY_RELOAD, fail_return);
    	application_checkpoint_write(epoch, rank, i, msg);
    	MPIX_SAFE_CALL(MPIX_Checkpoint_write(), code = MPIX_TRY_RELOAD, fail_return);
    }

    printf("%d checksum %i == %i\n", rank, msg, MAX_ITERATION);

    code = MPIX_SAFE_CALL(MPI_Barrier(MPI_COMM_WORLD), code = MPIX_TRY_RELOAD, fail_return);
    if (code == MPI_SUCCESS)
    {
    	*done = 1;
    }

    return code;

    fail_return:
    	return MPIX_TRY_RELOAD;
}

void inject_fault(int rank)
{
	printf("proc %i injecting fault, exit now\n", rank);
	exit(MPIX_TRY_RELOAD);
}

void application_checkpoint_read(int epoch, int rank, int *start, int *payload) {
	FILE *fp;
	char buf[10];
	sprintf(buf, "check_%d_%d", epoch, rank);
	if ((fp = fopen(buf, "rb")) == NULL) 
	{
		printf("ERROR: Failure in Opening File");
	}

	fread(start, sizeof(int), 1, fp);
	fread(payload, sizeof(int), 1, fp);
	fclose(fp);
}

void application_checkpoint_write(int epoch, int rank, int start, int payload) {
	FILE *fp;
	char buf[10];
	sprintf(buf, "check_%d_%d", epoch, rank);
	if ((fp = fopen(buf, "wb")) == NULL)
	{
		printf("ERROR: Failure in Opening File\n");
	}

	fwrite(&start, sizeof(int), 1, fp);
	fwrite(&payload, sizeof(int), 1, fp);
	
	fclose(fp);
}
