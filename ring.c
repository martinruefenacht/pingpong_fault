#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mpi.h"

#define ARRAY_LEN 4
#define TIMES_AROUND_LOOP 5
#define TAG 0

#define DEBUG
#define MPIX_SAFE_CALL(__operation,__predicate,__label) {int __code = __operation; if(__code != MPI_SUCCESS) {__predicate; goto __label;}}

void inject_fault(int rank)
{
	printf("proc %i injecting fault, exit now\n", rank);
	exit(MPIX_TRY_RELOAD);
}

int Application_Checkpoint_Read(int epoch, int rank, int *r, int *size, int smallmessage[]);
void Application_Checkpoint_Write(int epoch, int rank, int size, int smallmessage[]);

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
		case MPI_SUCCESS: //process create/recreate situation
			MPI_Init(&argc, &argv); //Function name might change later
			MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
			break;
		case MPIX_TRY_RELOAD:
			MPIX_Checkpoint_read(); //read the second to last checkpointed MPI state for this process and clear pending communication
			break;
		default:
			MPI_Abort(MPI_COMM_WORLD, -1);
			break;
		}
	    MPIX_Get_fault_epoch(&fault_epoch);
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
	
	int failed_iteration = 3;

	if (epoch > 0) {
		MPIX_SAFE_CALL(MPI_Comm_rank(MPI_COMM_WORLD, &crank), code = MPIX_TRY_RELOAD, fail_return);
		Application_Checkpoint_Read(epoch - 1, crank, &rank, &size, smallmessage);
		
		printf("%d: Size=%d, rank=%d, epoch=%d\n", rank, size, rank, epoch);
		
		failed_iteration = TIMES_AROUND_LOOP + 1;
	}
	else {
		MPIX_SAFE_CALL(MPI_Comm_size(MPI_COMM_WORLD, &size), code = MPIX_TRY_RELOAD, fail_return);
		MPIX_SAFE_CALL(MPI_Comm_rank(MPI_COMM_WORLD, &rank), code = MPIX_TRY_RELOAD, fail_return);
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
                	Application_Checkpoint_Write(epoch, rank, size, smallmessage);
                	MPIX_SAFE_CALL(MPIX_Checkpoint_write(), code = MPIX_TRY_RELOAD, fail_return);
		}
		printf("%d: About to MPI_Send...\n", rank);

		MPIX_SAFE_CALL(MPI_Send(smallmessage, ARRAY_LEN, MPI_INT, (rank + 1) % size, TAG, MPI_COMM_WORLD), code = MPIX_TRY_RELOAD, fail_return);
		
		if (rank != 0) {
			MPIX_SAFE_CALL(MPIX_Get_fault_epoch(&epoch), code = MPIX_TRY_RELOAD, fail_return);
                	Application_Checkpoint_Write(epoch, rank, size, smallmessage);
                	MPIX_SAFE_CALL(MPIX_Checkpoint_write(), code = MPIX_TRY_RELOAD, fail_return);
		}

		//if (smallmessage[0] == failed_iteration && rank == 0) {
		//	inject_fault(rank);
		//}

		if (smallmessage[0] == TIMES_AROUND_LOOP) {
			break;
		}
	}
	if (rank == 0) {
		MPIX_SAFE_CALL(MPI_Recv(smallmessage, ARRAY_LEN, MPI_INT, (rank - 1 + size) % size, TAG, MPI_COMM_WORLD, &status), code = MPIX_TRY_RELOAD, fail_return);
		printf("%d:  smallmessage[0] is now %d\n", rank, smallmessage[0]);
	}
	
	printf("%d: Exiting\n", rank);
	if (code == MPI_SUCCESS) {
		*done = 1;
	}

	return code;

	fail_return:
	return MPIX_TRY_RELOAD;
}

int Application_Checkpoint_Read(int epoch, int rank, int *r, int *size, int smallmessage[]) {
	FILE *fp;
	char buf[10];
	sprintf(buf, "check_%d_%d", epoch, rank);
	if ((fp = fopen(buf, "rb")) == NULL) {
		printf("ERROR: Opening File");
	}
	fread(smallmessage, sizeof(int), ARRAY_LEN, fp);
	fread(r, sizeof(int), 1, fp);
	fread(size, sizeof(int), 1, fp);
	fclose(fp);
	return 0;
}

void Application_Checkpoint_Write(int epoch, int rank, int size, int smallmessage[]) {
	FILE *fp;
	char buf[10];
	sprintf(buf, "check_%d_%d", epoch, rank);
	if ((fp = fopen(buf, "wb")) == NULL) {
		printf("ERROR: Opening File\n");
	}

	fwrite(smallmessage, sizeof(int), ARRAY_LEN, fp);
	fwrite(&rank, sizeof(int), 1, fp);
	fwrite(&size, sizeof(int), 1, fp);
	fclose(fp);
}
