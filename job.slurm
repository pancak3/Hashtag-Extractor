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

/usr/bin/time -f  \
"[*] Time results (/usr/bin/time)\n\tcmd\t%C\n\treal\t%es\n\tuser\t%Us\t\n\tsys\t%Ss\n\tmemory\t%MKB\t(peak)\n\tcpu\t%P\n\n\taverage total (data+stack+text) memory\t%KKB\n\tswapped out times\t%W\n\tpage faults\n\t\t%F\t(major, or I/O-requiring)\n\t\t%R\t(minor, or recoverable)\n\tfs output\t%O\n\tshared text\t%XKB\n\tsys page size\t%ZKB\n\tsockets num\n\t\t%r\t(received)\n\t\t%s\t(sent)" \
mpirun --bind-to none ./tp /data/projects/COMP90024/bigTwitter.json lang.csv
