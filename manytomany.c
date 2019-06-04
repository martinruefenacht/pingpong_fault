#include "mpi.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);

	int rank, size;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	// prepare non blocking buffers
	int sendbuf = rank * 2 + 1;

	for(int iteration = 0; iteration < 1; ++iteration)
	{
		// send all many to many
		MPI_Request requests[size];

		for(int irank = 1; irank < size; ++irank)
		{
			int target = (rank + irank) % size;

			MPI_Isend((void*)&sendbuf, 1, MPI_INT, target, iteration, MPI_COMM_WORLD, &requests[irank-1]);
		}

		//MPI_Barrier(MPI_COMM_WORLD);

		// recv all
		for(int irank = 1; irank < size; ++irank)
		{
			int recv_rank = (rank + irank) % size;
			int res[1] = {-1};

			MPI_Recv(res, 1, MPI_INT, recv_rank, iteration, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			int correct = recv_rank * 2 + 1;

			printf("address %p\n", res);
			printf("rank %i recv_rank %i data %i == %i?\n", rank, recv_rank, res[0], correct);
		}

		MPI_Waitall(size-1, requests, MPI_STATUSES_IGNORE);
	}
	printf("complete\n");

	MPI_Finalize();
}
