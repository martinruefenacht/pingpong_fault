#! /bin/bash
#SBATCH -n 2

echo "##############################################################################"
printenv | grep SLURM
echo "##############################################################################"

mpiexec ./pingpong
#mpiexec --separate_rank --separate_error --logexec --logfault --loghead ./pingpong
