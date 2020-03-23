#!/bin/bash

if [ -z $1 ]; then
  echo "[*] Useage: submmit.sh slurm.slurm"
  exit 0
fi

echo "[*] Please run this script under the directory where it is."

slurm_file=$1
job_id=$(sbatch "$slurm_file" | awk '{print $4}')
output_name="slurm-$job_id.out"

echo "[*] Waitting output ..."

while [ ! -f "$output_name" ]; do
  sleep 1
done
echo "[*] $slurm_file done"
mv "$output_name" "$slurm_file.out"
