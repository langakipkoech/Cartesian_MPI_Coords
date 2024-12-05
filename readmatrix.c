#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define ROWS 4
#define COLS 5

//function to read the matrix
void read_matrix(int* matrix, const char* filename){
    FILE* file = fopen(filename, "r");
    //check if file exists
    if(!file){
        fprintf(stderr, "cannot open matrix %s\n ", filename);
        exit(1);
    }
    //read the rows and the cols from first two lines error checking
    int read_rows, read_cols;
    if(fscanf(file, "%d\n%d\n", &read_rows, &read_cols) != 2){
        fprintf(stderr, "Error reading matrix dimensions\n ");
        exit(1);
    }

    //verif dimension match
    if( read_rows != ROWS || read_cols != COLS ){
        fprintf(stderr, "Error mismatch!\n ");
        exit(1);
    }

    //read the matrix values
    for(int i = 0; i < ROWS * COLS; i++){
        if(fscanf(file, "%d", &matrix[i]) != 1){
            fprintf(stderr, "error reading matrix values\n ");
            fclose(file);
            exit(1);
        }
    }
    
    //matrix read successfully 
    fprintf(stdout, "matrix read successfully\n ");

    //print matrix to stdout
    for(int i = 0; i < ROWS; i++){
        for(int j = 0; j < COLS; j++){
            fprintf(stdout, " %d ", matrix[i * COLS + j ]);

        }
        printf("\n ");
    }
    fclose(file);

}

//function to distribute the matrix to different processes based 


int main(int argc, char* argv[]){
    //allocated memory for full matrix
    int* FULL_MATRIX = (int*)malloc(ROWS * COLS * sizeof(int));

    read_matrix(FULL_MATRIX, "/home/user/Desktop/LAB_COURSE_II/week11/test.txt");

    free(FULL_MATRIX);

    return 0;
}