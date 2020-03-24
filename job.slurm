#!/bin/bash
#SBATCH --partition=cloud
#SBATCH --job-name="comp90024-p1"

#SBATCH --nodes=2
#SBATCH --ntasks=2
#SBATCH --cpus-per-task=4

# The maximum running time of the job in days-hours:mins:sec
#SBATCH --time=0-0:59:00

module load OpenMPI/3.1.0-GCC-8.2.0
make clean && make
/usr/bin/time mpirun ./tp /data/projects/COMP90024/bigTwitter.json files/lang.csv
