#!/bin/bash

if [ -z $1 ]; then
  echo "[!] Useage: bash submmit.sh slurm.slurm"
  exit 0
fi

slurm_file=$1
job_id=$(sbatch "$slurm_file" | awk '{print $4}')
sbatch_output_name="slurm-$job_id.out"

date=$(date '+%Y-%m-%d_%H:%M:%S')
final_output_name="$slurm_file_$data_$job_id.out"
echo "[+] Submmited $slurm_file"
echo "[+] Waitting $sbatch_output_name -> $final_output_name ..."

job_status=$(squeue --job "$job_id")
echo "$job_status" | head -n 1
while [ $(echo "$job_status" | wc -l) -gt 1 ]; do
  job_status=$(squeue --job "$job_id")
  status=$(echo "$job_status" | tail -n 1)
  echo -en "$status \r"
  sleep 2
done

sleep 2
mv "$sbatch_output_name" "$final_output_name"

echo "[+] Received $final_output_name"
echo " "

if [ $(cat "$final_output_name" | wc -l) -lt 10 ]; then
  cat "$final_output_name"
else
  head -n 5 "$final_output_name"
  echo "... ..."
  tail -n 5 "$final_output_name"
fi

tmp=$(cat "$final_output_name")

echo "$slurm_file" >"$final_output_name"
echo "$tmp" >>"$final_output_name"
