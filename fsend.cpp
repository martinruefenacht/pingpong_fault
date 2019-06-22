#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>

int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);
	int rank, size;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	std::cout << rank << std::endl;
	std::cout << size << std::endl;

	int j = 1000;
	if(rank == 0)
	{
		j = 50;
		MPI_Send(&j, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
		MPI_Send(&j, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
	}
	else if(rank == 1)
	{
		MPI_Recv(&j, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		MPI_Send(&j, 1, MPI_INT, 3, 0, MPI_COMM_WORLD);
	}
	else if(rank == 2)
	{
		MPI_Recv(&j, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	}
	else
	{
		MPI_Recv(&j, 1, MPI_INT, 1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	}

	std::cout << rank << std::endl;
	std::cout << j << std::endl;

	MPI_Finalize();
}
