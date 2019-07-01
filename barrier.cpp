#include <mpi.h>
#include <iostream>

int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);

	MPI_Barrier(MPI_COMM_WORLD);

	std::cout << "successful barrier" << std::endl;

	MPI_Finalize();
}
