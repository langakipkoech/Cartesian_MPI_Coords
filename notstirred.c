#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROWS 400
#define COLS 600
#define LOCAL_ROWS 100
#define LOCAL_COLS 50
#define GRID_ROWS 4
#define GRID_COLS 12

void read_matrix(double* matrix, const char* filename){
    FILE* file = fopen(filename, "r");
    if(!file){
        fprintf(stderr, "Cannot open matrix %s\n", filename);
        exit(1);
    }

    int read_rows, read_cols;
    if(fscanf(file, "%d\n%d\n", &read_rows, &read_cols) != 2){
        fprintf(stderr, "Error reading matrix dimensions\n");
        exit(1);
    }

    if( read_rows != ROWS || read_cols != COLS ){
        fprintf(stderr, "Error: Mismatch in dimensions!\n");
        exit(1);
    }

    for(int i = 0; i < ROWS * COLS; i++){
        if(fscanf(file, "%lf", &matrix[i]) != 1){
            fprintf(stderr, "Error reading matrix values\n");
            fclose(file);
            exit(1);
        }
    }
    
    fclose(file);
}

void distribute_matrix(double* global_matrix, double* local_matrix, MPI_Comm cart_comm) {
    int rank;
    MPI_Comm_rank(cart_comm, &rank);

    if (rank == 0) {
        for (int p = 0; p < GRID_ROWS * GRID_COLS; ++p) {
            int p_coords[2];
            MPI_Cart_coords(cart_comm, p, 2, p_coords);
            
            double* temp_matrix = malloc(LOCAL_ROWS * LOCAL_COLS * sizeof(double));
            
            for (int local_r = 0; local_r < LOCAL_ROWS; ++local_r) {
                for (int local_c = 0; local_c < LOCAL_COLS; ++local_c) {
                    int global_r = p_coords[0] * LOCAL_ROWS + local_r;
                    int global_c = p_coords[1] * LOCAL_COLS + local_c;
                    temp_matrix[local_r * LOCAL_COLS + local_c] = 
                        global_matrix[global_r * COLS + global_c];
                }
            }

            if (p == 0) {
                memcpy(local_matrix, temp_matrix, LOCAL_ROWS * LOCAL_COLS * sizeof(double));
            } else {
                MPI_Send(temp_matrix, LOCAL_ROWS * LOCAL_COLS, MPI_DOUBLE, p, 0, cart_comm);
            }
            
            free(temp_matrix);
        }
    } else {
        MPI_Recv(local_matrix, LOCAL_ROWS * LOCAL_COLS, MPI_DOUBLE, 0, 0, cart_comm, MPI_STATUS_IGNORE);
    }
}

void perform_shifts(double* local_matrix, MPI_Comm cart_comm, int j, int k) {
    int dims[2], periods[2] = {1, 1}, reorder = 1;
    MPI_Cart_get(cart_comm, 2, dims, periods, &reorder);

    int rank, coords[2];
    MPI_Comm_rank(cart_comm, &rank);
    MPI_Cart_coords(cart_comm, rank, 2, coords);

    MPI_Comm row_comm, col_comm;
    int free_coords[2];
    free_coords[0] = 0; free_coords[1] = 1;
    MPI_Cart_sub(cart_comm, free_coords, &row_comm);
    
    free_coords[0] = 1; free_coords[1] = 0;
    MPI_Cart_sub(cart_comm, free_coords, &col_comm);

    double* send_buf = malloc(LOCAL_ROWS * LOCAL_COLS * sizeof(double));
    double* recv_buf = malloc(LOCAL_ROWS * LOCAL_COLS * sizeof(double));

    switch (j) {
        case 0: // Column shift down
            if (coords[1] == k) {
                int src_rank, dest_rank;
                int src_coords[2] = {(coords[0] + 3) % dims[0], k};
                int dest_coords[2] = {(coords[0] + 1) % dims[0], k};
                
                MPI_Cart_rank(cart_comm, src_coords, &src_rank);
                MPI_Cart_rank(cart_comm, dest_coords, &dest_rank);
                
                MPI_Sendrecv(local_matrix, LOCAL_ROWS * LOCAL_COLS, MPI_DOUBLE, dest_rank, 0,
                             recv_buf, LOCAL_ROWS * LOCAL_COLS, MPI_DOUBLE, src_rank, 0,
                             cart_comm, MPI_STATUS_IGNORE);
                
                memcpy(local_matrix, recv_buf, LOCAL_ROWS * LOCAL_COLS * sizeof(double));
            }
            break;
        
        case 1: // Column shift up
            if (coords[1] == k) {
                int src_rank, dest_rank;
                int src_coords[2] = {(coords[0] + 1) % dims[0], k};
                int dest_coords[2] = {(coords[0] + 3) % dims[0], k};
                
                MPI_Cart_rank(cart_comm, src_coords, &src_rank);
                MPI_Cart_rank(cart_comm, dest_coords, &dest_rank);
                
                MPI_Sendrecv(local_matrix, LOCAL_ROWS * LOCAL_COLS, MPI_DOUBLE, dest_rank, 0,
                             recv_buf, LOCAL_ROWS * LOCAL_COLS, MPI_DOUBLE, src_rank, 0,
                             cart_comm, MPI_STATUS_IGNORE);
                
                memcpy(local_matrix, recv_buf, LOCAL_ROWS * LOCAL_COLS * sizeof(double));
            }
            break;
        
        case 2: // Row shift right
            if (coords[0] == k) {
                int src_rank, dest_rank;
                int src_coords[2] = {k, (coords[1] + 11) % dims[1]};
                int dest_coords[2] = {k, (coords[1] + 1) % dims[1]};
                
                MPI_Cart_rank(cart_comm, src_coords, &src_rank);
                MPI_Cart_rank(cart_comm, dest_coords, &dest_rank);
                
                MPI_Sendrecv(local_matrix, LOCAL_ROWS * LOCAL_COLS, MPI_DOUBLE, dest_rank, 0,
                             recv_buf, LOCAL_ROWS * LOCAL_COLS, MPI_DOUBLE, src_rank, 0,
                             cart_comm, MPI_STATUS_IGNORE);
                
                memcpy(local_matrix, recv_buf, LOCAL_ROWS * LOCAL_COLS * sizeof(double));
            }
            break;
        
        case 3: // Row shift left
            if (coords[0] == k) {
                int src_rank, dest_rank;
                int src_coords[2] = {k, (coords[1] + 1) % dims[1]};
                int dest_coords[2] = {k, (coords[1] + 11) % dims[1]};
                
                MPI_Cart_rank(cart_comm, src_coords, &src_rank);
                MPI_Cart_rank(cart_comm, dest_coords, &dest_rank);
                
                MPI_Sendrecv(local_matrix, LOCAL_ROWS * LOCAL_COLS, MPI_DOUBLE, dest_rank, 0,
                             recv_buf, LOCAL_ROWS * LOCAL_COLS, MPI_DOUBLE, src_rank, 0,
                             cart_comm, MPI_STATUS_IGNORE);
                
                memcpy(local_matrix, recv_buf, LOCAL_ROWS * LOCAL_COLS * sizeof(double));
            }
            break;
    }

    free(send_buf);
    free(recv_buf);
    MPI_Comm_free(&row_comm);
    MPI_Comm_free(&col_comm);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Create Cartesian communicator
    int dims[2] = {GRID_ROWS, GRID_COLS};
    int periods[2] = {1, 1};  // Periodic in both dimensions
    int reorder = 1;  // Allow reordering of ranks
    MPI_Comm cart_comm;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &cart_comm);

    // Allocate matrix storage
    double* global_matrix = NULL;
    double* local_matrix = malloc(LOCAL_ROWS * LOCAL_COLS * sizeof(double));
    
    // Read matrix on process 0
    if (rank == 0) {
        global_matrix = malloc(ROWS * COLS * sizeof(double));
        read_matrix(global_matrix, "/work/korzec/LAB2/ex8/notstirred.txt");
    }

    // Distribute matrix
    distribute_matrix(global_matrix, local_matrix, cart_comm);

    // Read shifts
    int shifts[1000][2];
    if (rank == 0) {
        FILE* shifts_file = fopen("/work/korzec/LAB2/ex8/shifts.dat", "r");
        if (!shifts_file) {
            fprintf(stderr, "Cannot open shifts file\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        for (int i = 0; i < 1000; i++) {
            if (fscanf(shifts_file, "%d %d", &shifts[i][0], &shifts[i][1]) != 2) {
                fprintf(stderr, "Error reading shifts file\n");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
        fclose(shifts_file);
    }

    // Broadcast shifts to all processes
    MPI_Bcast(shifts, 1000 * 2, MPI_INT, 0, MPI_COMM_WORLD);

    // Perform matrix shifts
    for (int i = 0; i < 1000; i++) {
        perform_shifts(local_matrix, cart_comm, shifts[i][0], shifts[i][1]);
    }

    // Collect the transformed matrix into process 0
    // Collect the transformed matrix into process 0
double* final_matrix = NULL;
if (rank == 0) {
    final_matrix = malloc(ROWS * COLS * sizeof(double));  // Allocate memory on process 0
}

// Gather the local matrices into the final_matrix on process 0
MPI_Gather(local_matrix, LOCAL_ROWS * LOCAL_COLS, MPI_DOUBLE, final_matrix, LOCAL_ROWS * LOCAL_COLS, MPI_DOUBLE, 0, cart_comm);

// Print the transformed matrix on process 0
if (rank == 0) {
    // Print matrix dimensions
    printf("%d\n%d\n", ROWS, COLS);
    
    // Write matrix values in scientific notation with the desired precision (one number per line)
    FILE* output_file = fopen("output.txt", "w");
    if (!output_file) {
        fprintf(stderr, "Cannot open output file for writing\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Write matrix dimensions
    fprintf(output_file, "%d\n%d\n", ROWS, COLS);
    
    // Write each value in the matrix to the output file (one value per line in scientific notation)
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            // Print each number in scientific notation with 16 decimal places, each on a new line
            fprintf(output_file, "%.16e\n", final_matrix[i * COLS + j]);
        }
    }

    fclose(output_file); // Close the output file
    free(final_matrix);
    }


     // Cleanup
    free(local_matrix);
    if (rank == 0) free(global_matrix);

    MPI_Comm_free(&cart_comm);
    MPI_Finalize();
    return 0;
}

