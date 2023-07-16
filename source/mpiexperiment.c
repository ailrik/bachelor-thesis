#include <stdio.h>
#include <stdlib.h>
#include <openmpi-x86_64/mpi.h>

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);

    // Check that only 2 MPI processes are spawned
    int comm_size;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    

    // Get my rank
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // Create the window
    const int ARRAY_SIZE = 324;
    int *window_buffer;
    MPI_Win window;

    MPI_Win_allocate((MPI_Aint)(ARRAY_SIZE * sizeof(int)), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &window_buffer, &window);
    
    if (my_rank == 0) {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            window_buffer[i] = (i % 9) + 1;
        }
    }
    
    MPI_Win_fence(0, window);
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, window);

    
    int remote_arr[ARRAY_SIZE];
    if (my_rank > 0) {
        MPI_Get(&remote_arr, ARRAY_SIZE, MPI_INT, 0, 0, ARRAY_SIZE, MPI_INT, window);

        int min = 9;
        int col = 0;

        for (int i = 0; i < ARRAY_SIZE; i++) {
            if (remote_arr[i] < min && remote_arr[i] > 0) {
                min = remote_arr[i];
                col = i;
            }
        }

        fprintf(stderr, "min: %d, col: %d\n", min, col);

        remote_arr[col] = 0;

        MPI_Put(&remote_arr, ARRAY_SIZE, MPI_INT, 0, 0, ARRAY_SIZE, MPI_INT, window);
    }
    
    MPI_Win_unlock( 0, window);
    MPI_Win_fence(0, window);

    if (my_rank == 0) {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            printf("%d ", window_buffer[i]);
        }
        printf("\n");
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Destroy the window
    MPI_Win_free(&window);

    MPI_Finalize();

    return EXIT_SUCCESS;
}
