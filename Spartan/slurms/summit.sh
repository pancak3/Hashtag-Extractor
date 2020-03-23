#!/bin/bash
echo "[*] Please run this script under the directory where it is."

slurm_file=$1
job_id=$(echo "Submitted batch job 15497286" | awk '{print $4}')
output_name="slurm-$job_id.out"

echo "[*] Waitting output ..."

while [ ! -f "$output_name" ]; do
  sleep 1
done
echo "[*] $slurm_file done"
mv $output_name "$slurm_file.out"
