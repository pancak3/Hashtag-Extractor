#!/bin/bash
#SBATCH --partition=cloud
#SBATCH --job-name="comp90024-p1"
#SBATCH --mail-user=example@mail.coom

#SBATCH --nodes=2
#SBATCH --ntasks=2
#SBATCH --cpus-per-task=4

# The maximum running time of the job in days-hours:mins:sec
#SBATCH --time=0-0:59:00

module load OpenMPI/1.10.2-GCC-4.9.2

cd /home/f07397/comp90024-p1
make clean && make
/usr/bin/time mpirun ./tp /data/projects/COMP90024/bigTwitter.json files/lang.csv