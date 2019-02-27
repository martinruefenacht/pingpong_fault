#! /bin/bash

#mpiexec -n 2 --separate_rank --separate_error --logexec --logfault --loghead --logcontrol ./pingpong_perf 100
mpiexec -n 2 --separate_rank --separate_error --logexec --logfault --loghead --logcontrol ./pingpong_perf 5 

#mpiexec -n 2 --mca btl tcp,self ./pingpong_perf 100
