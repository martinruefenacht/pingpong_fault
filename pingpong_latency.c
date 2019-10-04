#include<mpi.h>

#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include<unistd.h>

#define MAX_MSG_SIZE (1<<22)
#define MYBUFSIZE (MAX_MSG_SIZE)
#define LARGE_MESSAGE_SIZE 8192
#define LAT_LOOP_SMALL 10000
#define LAT_SKIP_SMALL 100
#define LAT_LOOP_LARGE 1000
#define LAT_SKIP_LARGE 10

#ifndef FIELD_WIDTH
#define FIELD_WIDTH 20
#endif

#ifndef FLOAT_PRECISION
#define FLOAT_PRECISION 2
#endif

int allocate_memory(char **sbuf, char **rbuf) {
	unsigned long align_size = sysconf(_SC_PAGESIZE);
	if (posix_memalign((void**)sbuf, align_size, MYBUFSIZE)) {
		fprintf(stderr, "Error allocating host memory\n");
		return 1;
	}	

	if (posix_memalign((void**)rbuf, align_size, MYBUFSIZE)) {
                fprintf(stderr, "Error allocating host memory\n");
                return 1;
        }
}

void touch_data(void *sbuf, void *rbuf, size_t size) {
	memset(sbuf, 'a', size);
	memset(rbuf, 'b', size);
}

void free_memory(void *sbuf, void *rbuf) {
	free(sbuf);
	free(rbuf);
}

int main(int argc, char *argv[]) {
	int myid, numprocs, i;
	int size;
	MPI_Status reqstat;
	int loop = LAT_LOOP_SMALL, skip = LAT_SKIP_SMALL, loop_large = LAT_LOOP_LARGE, skip_large = LAT_SKIP_LARGE;
	char *s_buf, *r_buf;
	double t_start = 0.0, t_end = 0.0;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	if (numprocs != 2) {
		if (myid == 0) {
			fprintf(stderr, "This test requires exactly two processes\n");
		}
		MPI_Finalize();
		exit(1);
	}

	// allocate memory
	if (allocate_memory(&s_buf, &r_buf)) {
		MPI_Finalize();
		exit(1);
	}

	// Latency test
	for (size = 0; size <= MAX_MSG_SIZE; size = (size ? size * 2 : 1)) {
		touch_data(s_buf, r_buf, size);
		
		if (size > LARGE_MESSAGE_SIZE) {
			loop = loop_large;
			skip = skip_large;
		}

		MPI_Barrier(MPI_COMM_WORLD);

		if (myid == 0) {
			for (i = 0; i < loop + skip; i++) {
				if (i == skip) {
					t_start = MPI_Wtime();
				}
				MPI_Send(s_buf, size, MPI_CHAR, 1, 1, MPI_COMM_WORLD);
				MPI_Recv(r_buf, size, MPI_CHAR, 1, 1, MPI_COMM_WORLD, &reqstat);
			}
			t_end = MPI_Wtime();
		}

		else if (myid == 1) {
			for (i = 0; i < loop + skip; i++) {
				MPI_Recv(r_buf, size, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &reqstat);
				MPI_Send(s_buf, size, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
			}
		}

		if (myid == 0) {
			double latency = (t_end - t_start) * 1e6 / (2.0 * loop);
			printf("%-*d%*.*f\n", 10, size, FIELD_WIDTH, FLOAT_PRECISION, latency);
			//fflush(stdout);
		}
	}
	free_memory(s_buf, r_buf);
	MPI_Finalize();
	return 0;
}
