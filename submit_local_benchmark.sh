#! /bin/bash

for iters in $(seq 100 10 100); do
	#mpiexec --mca btl tcp,self -n 2 ./pingpong_perf $iters
	mpiexec -n 2 ./pingpong_perf $iters
done

