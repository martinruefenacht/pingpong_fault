#! /bin/bash
#SBATCH -n 2
#SBATCH -N 2

printenv | grep SLURM

mpiexec ./pingpong_fault
#mpiexec --separate_rank --separate_error --logexec --logfault --loghead ./pingpong_fault
