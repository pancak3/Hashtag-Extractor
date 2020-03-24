#!/bin/bash
#SBATCH --partition=cloud
#SBATCH --time=0-0:59:00

module load OpenMPI/3.1.0-GCC-8.2.0
make clean && make
/usr/bin/time mpirun ./tp /data/projects/COMP90024/bigTwitter.json lang.csv
