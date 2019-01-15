#! /bin/bash
#SBATCH -n 2
#SBATCH -N 2

printenv | grep SLURM

mpiexec ./pingpong
#mpiexec --separate_rank --separate_error --logexec --logfault --loghead ./pingpong
