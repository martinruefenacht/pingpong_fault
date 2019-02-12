#! /bin/bash

mpiexec -n 2 --separate_rank --separate_error --logexec --logfault --loghead --logcontrol ./pingpong_perf
