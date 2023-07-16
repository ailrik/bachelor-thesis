#include <stdio.h>
#include <stdlib.h>
#include <openmpi-x86_64/mpi.h>

#define ROWS 2
#define COLS 81

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int** matrix = (int**) malloc(ROWS * sizeof(int*));
    for (int i = 0; i < ROWS; i++) {
        matrix[i] = (int*) malloc(COLS * sizeof(int));
    }

    // Initialize the matrix with some data
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = i * COLS + j;
            fprintf(stderr, "%d ", matrix[i][j]);
        }
        fprintf(stderr, "\n");
    }

    int disp_unit = COLS * sizeof(int);
    MPI_Win win;
    MPI_Win_create(&matrix[0][0], ROWS * disp_unit, disp_unit, MPI_INFO_NULL, MPI_COMM_WORLD, &win);

    // Use MPI_Get to access the second row of the matrix
    int row_index = 0;
    int buffer[COLS];
    MPI_Win_fence(0, win);
    MPI_Get(buffer, COLS, MPI_INT, rank, row_index, COLS, MPI_CHAR, win);

    MPI_Win_fence(0, win);
    // Print the contents of the buffer
    for (int i = 0; i < COLS; i++) {
        printf("%d ", buffer[i]);
    }

    MPI_Win_free(&win);
    MPI_Finalize();

    // Free the dynamically allocated matrix
    for (int i = 0; i < ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);

    return 0;
}