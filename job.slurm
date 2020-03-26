#!/bin/bash
#SBATCH --job-name="comp90024_p1"
#SBATCH --partition=physical
#SBATCH --constraint=physg4
#SBATCH --exclude=spartan-bm[027-030],spartan-bm[018-023]
#SBATCH --contiguous
#SBATCH --wait-all-nodes=1
#SBATCH --time=0-0:10:00
#SBATCH --dependency=singleton

module purge
module load OpenMPI/3.1.0-GCC-8.2.0
make
/usr/bin/time mpirun --bind-to none ./tp /data/projects/COMP90024/bigTwitter.json lang.csv
