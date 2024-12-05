#!/bin/bash
#SBATCH --job-name=matrix_shifts     # Job name
#SBATCH --nodes=2                    # Number of nodes (to accommodate 48 processes on 24-core nodes)
#SBATCH --ntasks=48                  # Total number of tasks (MPI processes)
#SBATCH --ntasks-per-node=24         # Maximum tasks per node
#SBATCH --exclusive                  # Use the entire node exclusively
#SBATCH --time=1:00:00               # Maximum walltime (HH:MM:SS)
#SBATCH --output=matrix_shifts.out   # Standard output log
#SBATCH --error=matrix_shifts.err    # Standard error log
#SBATCH --partition=compute2011     # Specify the partition name

# Load the required MPI module
module load mpi/openmpi/4.1.0

# Display loaded modules (for reproducibility)
module list

# Start timer
start_time=$(date +%s)
echo "Starting MPI program at $(date)..."

# Run the MPI program
mpirun -np 48 ./matrix_shifts

# End timer and calculate elapsed time
end_time=$(date +%s)
elapsed_time=$((end_time - start_time))
echo "MPI program finished at $(date)."
echo "Total execution time: ${elapsed_time} seconds"
