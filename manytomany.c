#include "mpi.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);

	int rank, size;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	for(int iteration = 0; iteration < 10; ++iteration)
	{
		// send all many to many
		MPI_Request requests[size];

		for(int irank = 1; irank < size; ++irank)
		{
			int res[12000] = {rank * 2 + 1}; 
			MPI_Isend(&res, 12000, MPI_INT, (rank + irank) % size, 0, MPI_COMM_WORLD, &requests[irank-1]);
		}


		MPI_Barrier(MPI_COMM_WORLD);

		// recv all
		for(int irank = 1; irank < size; ++irank)
		{
			int res[12000];
			int recv_rank = (rank + irank) % size;

			MPI_Recv(&res, 12000, MPI_INT, recv_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			printf("rank %i recv_rank %i data %i\n", rank, recv_rank, res[0]);
		}

		MPI_Waitall(size-1, requests, MPI_STATUSES_IGNORE);
	}

	MPI_Finalize();
}
