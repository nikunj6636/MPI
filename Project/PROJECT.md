## To run locally:
```
mpic++ -o code code.cpp
mpiexec -np number_of_processes ./code input_path
Example :- mpiexec -np 3 ./code ./input.txt
```

## To run on rse cluster:
ssh cs3401.42@rce
logged in with the ssh-key-gen public key in .ssh/id_12...pub

## script.sh
```
#!/bin/bash
#
#SBATCH --job-name=test_mpi
#SBATCH --output=res_mpi.txt
#SBATCH --error=res_mpi_stderr.txt 
#
module load openmpi/4.1.5

# Compile the C program
mpicc OneGroup.c

# Launch the mpi program
mpirun  ./a.out 
```

## Run with n tasks on SLURM
```
sbatch -n 6 script.sh
```

## List all current jobs for a user
```
squeue -u $USER
sacct -j 3119 --format=user,jobid,jobname,partition,state,time,start,end,elapsed,alloctres,ncpus,nodelist
```

## To copy file:
```
scp [options] [[user@]host1:]source_file_or_directory ... [[user@]host2:]destination
use -r flag for cp directory recursively

eg: 
scp -r  "/home/nikunj/Downloads/Distributed systems/Project/code" cs3401.42@rce:~/Team61/
```

## Links:
https://hpc.iiit.ac.in/wiki/index.php/Ada_User_Guide#Partitions,_Account,_and_QoS \
http://rce.iiit.ac.in/wiki/index.php?title=Main_Page

## To run MPI using slrum
https://docs.open-mpi.org/en/main/launching-apps/slurm.html

## slrum commands:
```
module list
module avail
srun --mpi=list
```

## Command to see resource allocations:
```
sinfo -N -l
scontrol show partition
ulimit -v  # Maximum virtual memory
quota -u # Disk quota
```