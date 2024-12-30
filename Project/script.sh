#!/bin/bash

# Load the required module
module load openmpi/4.1.5

# Compile the C++ program
mpic++ final.cpp

# Array of node counts for testing
node_counts=(1 10 20 30 40 50 60 70 80 90 100)

# input number
input=0

for nodes in "${node_counts[@]}"; do
    sbatch --ntasks="$nodes" --job-name="Floyd_Warshall_$nodes" \
           --output="./out_${input}/output_${nodes}.txt" \
           --error="err.txt" \
           --wrap="mpirun ./a.out ./Dataset/in_${input}.txt"
done

# for file in output_*; do
#    tail -n 1 "$file" >> final_output.txt
# done