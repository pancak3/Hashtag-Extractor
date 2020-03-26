#!/bin/bash
#SBATCH --job-name="comp90024-p1"
#SBATCH --partition=physical
#SBATCH --constraint=physg4
#SBATCH --nodelist=spartan-bm061
#SBATCH --contiguous
#SBATCH --wait-all-nodes=1
#SBATCH --time=0-0:15:00
#SBATCH --dependency=singleton

module purge
module load OpenMPI/3.1.0-GCC-8.2.0
make
/usr/bin/time mpirun --bind-to none ./tp /data/projects/COMP90024/bigTwitter.json lang.csv
