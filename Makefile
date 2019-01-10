default: pingpong.c
	mpicc pingpong.c -o pingpong
	mpicc pingpong_fault.c -o pingpong_fault

clean:
	rm -rf mpi*
	rm -rf slurm*
	rm -rf local-*
	rm -rf *.log
	rm -rf *.cp
	rm -rf check*

veryclean: clean
	rm -rf pingpong
	rm -rf pingpong_fault
