#!/bin/bash

if [[ -z $1 || -z $2 ]]; then
  echo -e "Usage: \r\n\tbash submmit.sh \$Nodes \$CoresPerNode\r\nE.g:\r\n\tbash submmit.sh 2 4 # run on two nodes with 8 cores in total."
  exit 0
fi
slurm_file="job.slurm"
n_nodes=$1
n_tasks=$1
n_cpus=$2
job_id=$(sbatch --nodes="$n_nodes" --ntasks="$n_tasks" --cpus-per-task="$n_cpus" "$slurm_file" | awk '{print $4}')

date_str=$(date '+%Y-%m-%d_%H:%M:%S')

out="slurm-$job_id.out"
touch "$out"
new_out="$n_nodes-nodes-$n_cpus-cores-$date_str.out"
ln -s "$out" "$new_out"

echo -e "Output of job $n_nodes nodes $n_cpus cores is at:\r\n\t$out -> $new_out"
