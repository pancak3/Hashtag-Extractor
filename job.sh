#!/bin/bash

slurm_file="job.slurm"
n_nodes=$1
n_tasks=$1
n_cpus=$2

if [[ -z $1 || -z $2 ]]; then
  echo -e "usage: $0 nodes cpus-per-node"
  exit 1
fi

# submission time
date_str=$(date '+%Y-%m-%d_%H:%M:%S')

# run job and extract id
job_id=$(sbatch --nodes="$n_nodes" --ntasks="$n_tasks" --cpus-per-task="$n_cpus" "$slurm_file" | awk '{print $4}')

# where jobs logs are deposited
out="slurm-$job_id.out"

# create symlink
touch "$out"
new_out="$n_nodes-nodes-$n_cpus-cores-$date_str.out"
ln -s "$out" "$new_out"

echo -e "Output of job $n_nodes nodes $n_cpus cores is at:\n\t$out -> $new_out"
