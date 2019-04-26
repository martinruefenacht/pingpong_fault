#! /bin/bash
#SBATCH -n 3
#SBATCH --partition=pdebug

#printenv | grep SLURM

#mpiexec ./ring
mpiexec --separate_rank --separate_error --logexec --logfault --loghead --enable_stages -n 3 ./ring
