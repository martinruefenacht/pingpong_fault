#! /bin/bash

for iters in $(seq 1000 10 1000); do
	#mpiexec --mca btl tcp,self -n 2 ./pingpong_perf $iters
	mpiexec -n 2 ./pingpong_perf $iters
done

