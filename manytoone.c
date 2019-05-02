#include "mpi.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);

	int rank, size;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(rank == 0)
	{
		MPI_Barrier(MPI_COMM_WORLD);

		sleep(5);

		for(int irank = 1; irank < size; ++irank)
		{
			int res; 
			MPI_Recv(&res, 1, MPI_INT, irank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			printf("rank %i irank %i data %i\n", rank, irank, res);
		}
	}

	else
	{
		int payload = rank * 2 + 1;

		MPI_Send(&payload, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

		MPI_Barrier(MPI_COMM_WORLD);
	}

	MPI_Finalize();
}
