#! /bin/bash
#SBATCH -n 2
#SBATCH -N 2

pwd
hostname

module load openmpi/3.1.3

make

mpiexec ./pingpong_perf
