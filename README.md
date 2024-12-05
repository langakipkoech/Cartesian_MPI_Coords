# Cartesian_MPI_Coords
The goal of this project is to mixup pixel values of an image using cyclic rotation of rows and column values utilizing MPI and observing the resultant image

# Cartesian MPI Coordinates

This project demonstrates using MPI to perform cyclic rotations on an image matrix using a 4 × 12 process grid. The program reads matrix data, applies transformations based on input commands, and prints the transformed image.

## Key Steps

### 1. Clone the Repository

Clone this repository to your local machine:

```bash
git clone https://github.com/langakipkoech/Cartesian_MPI_Coords.git
cd Cartesian_MPI_Coords

mpicc -o cartesian_mpi_coords cartesian_mpi_coords.c

mpirun -np 48 ./cartesian_mpi_coords

This version focuses on the essential steps and keeps it brief for users to understand and quickly get started with the project.
