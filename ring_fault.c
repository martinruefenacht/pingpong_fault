#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mpi.h"

#define ARRAY_LEN 4
#define TIMES_AROUND_LOOP 5
#define TAG 0

#define DEBUG
#define MPIX_SAFE_CALL(__operation,__predicate,__label) {int __code = __operation; if(__code != MPI_SUCCESS) {__predicate; goto __label;}}

void inject_fault(int rank);
void application_checkpoint_read(int epoch, int rank, int smallmessage[]);
void application_checkpoint_write(int epoch, int rank, int smallmessage[]);
int main_loop(int restart_iteration, int *done);

int main(int argc, char** argv)
{
	int abort = 0;
	int done = 0;
	int recovery_code;
	int code = MPI_SUCCESS;
	int fault_epoch = 0;

	while (!abort && !done) {
		switch(code) {
		case MPI_SUCCESS: 
			code = MPI_Init(&argc, &argv);
			code = MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
			break;

		case MPIX_TRY_RELOAD:
			code = MPIX_Checkpoint_read(); 
			break;

		default:
			MPI_Abort(MPI_COMM_WORLD, 100);
			break;
		}

	    code = MPIX_Get_fault_epoch(&fault_epoch);
	    code = main_loop(fault_epoch, &done);
	}

	MPI_Finalize();

	return 0;
}

int main_loop(int epoch, int *done) {
	int i;
	int code = MPI_SUCCESS;
	int rank, size, crank;
	int smallmessage[ARRAY_LEN];
	MPI_Status status;
	
	// Randomly choose an iteration to fail between 0 and MAX_ITERATION - 1
	srand(time(NULL));
	int failed_iteration = rand() % TIMES_AROUND_LOOP;

	MPIX_SAFE_CALL(MPI_Comm_size(MPI_COMM_WORLD, &size), code = MPIX_TRY_RELOAD, fail_return);
	MPIX_SAFE_CALL(MPI_Comm_rank(MPI_COMM_WORLD, &rank), code = MPIX_TRY_RELOAD, fail_return);

	if (epoch > 0) {
		application_checkpoint_read(epoch - 1, rank, smallmessage);
		failed_iteration = TIMES_AROUND_LOOP + 1;
	}
	else {
		printf("%d: Size=%d, rank=%d\n", rank, size, rank);
		smallmessage[0] = 0;
		for (i = 1; i < ARRAY_LEN; i++) {
			smallmessage[i] = 0;
		}
	}
	if (rank == 0) {
		MPIX_SAFE_CALL(MPI_Send(smallmessage, ARRAY_LEN, MPI_INT, (rank + 1) % size, TAG, MPI_COMM_WORLD), code = MPIX_TRY_RELOAD, fail_return);
	}

	while (1) {
		printf("%d: About to MPI_Recv...\n", rank);
		MPIX_SAFE_CALL(MPI_Recv(smallmessage, ARRAY_LEN, MPI_INT, (rank - 1 + size) % size, TAG, MPI_COMM_WORLD, &status), code = MPIX_TRY_RELOAD, fail_return);
		printf("%d:  smallmessage[0] is now %d\n", rank, smallmessage[0]);

		if (rank == 0) {
			++smallmessage[0];
			MPIX_SAFE_CALL(MPIX_Get_fault_epoch(&epoch), code = MPIX_TRY_RELOAD, fail_return);
                	application_checkpoint_write(epoch, rank, smallmessage);
                	MPIX_SAFE_CALL(MPIX_Checkpoint_write(), code = MPIX_TRY_RELOAD, fail_return);
		}
		printf("%d: About to MPI_Send...\n", rank);

		MPIX_SAFE_CALL(MPI_Send(smallmessage, ARRAY_LEN, MPI_INT, (rank + 1) % size, TAG, MPI_COMM_WORLD), code = MPIX_TRY_RELOAD, fail_return);
		
		if (rank != 0) {
			MPIX_SAFE_CALL(MPIX_Get_fault_epoch(&epoch), code = MPIX_TRY_RELOAD, fail_return);
                	application_checkpoint_write(epoch, rank, smallmessage);
                	MPIX_SAFE_CALL(MPIX_Checkpoint_write(), code = MPIX_TRY_RELOAD, fail_return);
		}

		if (smallmessage[0] == failed_iteration && rank == 0) {
			inject_fault(rank);
		}

		if (smallmessage[0] == TIMES_AROUND_LOOP) {
			break;
		}
	}
	if (rank == 0) {
		MPIX_SAFE_CALL(MPI_Recv(smallmessage, ARRAY_LEN, MPI_INT, (rank - 1 + size) % size, TAG, MPI_COMM_WORLD, &status), code = MPIX_TRY_RELOAD, fail_return);
		printf("%d:  smallmessage[0] is now %d\n", rank, smallmessage[0]);
	}
	
	printf("%d: Exiting\n", rank);
	
	MPIX_SAFE_CALL(MPI_Barrier(MPI_COMM_WORLD), code = MPIX_TRY_RELOAD, fail_return);
	if (code == MPI_SUCCESS) {
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

void application_checkpoint_read(int epoch, int rank, int smallmessage[]) {
	FILE *fp;
	char buf[10];
	sprintf(buf, "check_%d_%d", epoch, rank);
	if ((fp = fopen(buf, "rb")) == NULL) {
		printf("ERROR: Opening File");
	}
	fread(smallmessage, sizeof(int), ARRAY_LEN, fp);
	fclose(fp);
	return 0;
}

void application_checkpoint_write(int epoch, int rank, int smallmessage[]) {
	FILE *fp;
	char buf[10];
	sprintf(buf, "check_%d_%d", epoch, rank);
	if ((fp = fopen(buf, "wb")) == NULL) {
		printf("ERROR: Opening File\n");
	}

	fwrite(smallmessage, sizeof(int), ARRAY_LEN, fp);
	fclose(fp);
}
