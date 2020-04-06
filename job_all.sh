#!/bin/bash

# Launch jobs according to project specification
# Run multiple times; 1 time by default
if [[ -z $1 ]]; then
  LOOP_NUM=1
else
  LOOP_NUM=$1
fi

for i in $(seq 1 "$LOOP_NUM"); do
  ./job.sh 1 1
  ./job.sh 1 8
  ./job.sh 2 4
done
