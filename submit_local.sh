#! /bin/bash

printenv | grep SLURM

mpiexec -n 2 --separate_rank --separate_error --logexec --logfault --loghead --logcontrol ./pingpong
#mpiexec -n 2 ./pingpong
