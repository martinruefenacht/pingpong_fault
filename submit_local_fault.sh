#! /bin/bash

printenv | grep SLURM

mpiexec -n 2 --separate_rank --separate_error --logexec --logfault --loghead --logcontrol ./pingpong_fault

#mpiexec -n 2 --separate_rank --separate_error --logexec --logfault --loghead --logcontrol konsole -e "gdb ./pingpong_fault"

#mpiexec -n 2 --separate_rank --separate_error --logexec --logfault --loghead --logcontrol valgrind ./pingpong_fault

#mpiexec -n 2 ./pingpong_fault
