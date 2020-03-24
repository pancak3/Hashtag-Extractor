#!/bin/bash
#SBATCH --partition=physical
#SBATCH --constraint=physg4
#SBATCH --time=0-0:59:00

module load OpenMPI/3.1.0-GCC-8.2.0
make
/usr/bin/time mpirun --bind-to none ./tp /data/projects/COMP90024/bigTwitter.json lang.csv
