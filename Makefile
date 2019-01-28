default: perf

perf: pingpong_perf.c
	mpicc pingpong_perf.c -O2 -g -o pingpong_perf

all: pingpong.c pingpong_fault.c pingpong_perf.c
	mpicc pingpong.c -o pingpong
	mpicc pingpong_fault.c -o pingpong_fault
	mpicc pingpong_perf.c -o pingpong_perf

clean:
	rm -rf mpi*
	rm -rf slurm*
	rm -rf local-*
	rm -rf *.log
	rm -rf *.cp
	rm -rf check*
	rm -rf callgrind*

veryclean: clean
	rm -rf pingpong
	rm -rf pingpong_fault
	rm -rf pingpong_perf
