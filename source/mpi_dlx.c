#include <stdbool.h>
#include <stdio.h>
#include <openmpi-x86_64/mpi.h>

#include "dlink.h"
#include "dlx.h"

#define COLS 324
#define ROOT 0

#define OTHER_RANK 2


int myranktemp;

/**
 * @brief Get the lowest col mem object
 * 
 * @return int the col with lowest nodes 
 */
int get_lowest_col_mem(MPI_Win window) {
    int arr[COLS];
    int lowest = 9;
    int column = 0;

    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, ROOT, 0, window);
    MPI_Get(&arr, COLS, MPI_INT, ROOT, 0, COLS, MPI_INT, window); 

    for (int i = 0; i < COLS; i++) {
        if(arr[i] < lowest && arr[i] > 0) {
            column = i;
            lowest = arr[i];
        }
    }
    // Set the lowest to 0 to indicate that it has been chosen
    arr[column] = 0;
    MPI_Put(&arr, COLS, MPI_INT, ROOT, 0, COLS, MPI_INT, window);
    MPI_Win_unlock(ROOT, window);

    return column;
}

bool dlx_is_finished(MPI_Win window) {
    int check;

    MPI_Win_lock(MPI_LOCK_SHARED, ROOT, 0, window);
    MPI_Get(&check, 1, MPI_INT, ROOT, 0, 1, MPI_INT, window);
    MPI_Win_unlock(ROOT, window);

    return check == -1;
}

void found_solution(MPI_Win window) {
    int check = -1;
    
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, ROOT, 0, window);
    MPI_Put(&check, 1, MPI_INT, ROOT, 0, 1, MPI_INT, window);
    MPI_Win_unlock(ROOT, window);
}

/**
 * @brief Create a mpi window object
 * 
 * @return MPI_Win the window object that has been allocated
 */
MPI_Win create_mpi_window() {
    MPI_Win window;
    int *window_buffer;
    MPI_Win_allocate((MPI_Aint)(COLS * sizeof(int)), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &window_buffer, &window);
    return window;
}

/**
 * @brief Set the window buffer object
 *
 * In theory, this should be called by the root process to set the window buffer
 *
 * @param window_buffer the window buffer to set
 * @param header header of the dancing links
 */
void set_window_buffer(int *window_buffer, dlink_t *header) {
    dlink_t* temp = header->next;
    for (int i = 0; i < COLS; i++) {
        window_buffer[i] = 0;
    }

    while(temp != header && temp->col != -1) {
        window_buffer[temp->col] = temp->numNodes;
        temp = temp->next;
    }
}

int serial_dlx(MPI_Win window, dlink_t* header, stack_t* chosenRows, int count) {
    if (header->next == header) {
        found_solution(window);
        fprintf(stderr, "Found solution at depth %d\n", count);
        return true;
    }
    if (count % 4 == 0) {
        if (dlx_is_finished(window)) {
            return OTHER_RANK;
        }
    }
    dlink_t *col = chooseColumn(header);
    cover(col);

    dlink_t *nextDown;
    dlink_t *nextRight;
    dlink_t *nextLeft;

    for (nextDown = col->down; nextDown != col;
        nextDown = nextDown->down) {
        stack_push(chosenRows, nextDown);
        for (nextRight = nextDown->next; nextDown != nextRight;
            nextRight = nextRight->next) {
            cover(nextRight);
        }

        switch (serial_dlx(window, header, chosenRows, count+1)) {
            case true:
                return true;
                break;
            case OTHER_RANK:
                return OTHER_RANK;
                break;

            case false:
                break;
            default:
                break;
        }
        stack_pop(chosenRows);

        col = nextDown->colHeader;
        for (nextLeft = nextDown->prev; nextDown != nextLeft;
            nextLeft = nextLeft->prev) {
        uncover(nextLeft);
        }
    }
    uncover(col);
    return false;
}

int mpi_dlx(MPI_Win window, dlink_t* header, stack_t* chosenRows, int count) {
    if (header->next == header) {
        found_solution(window);
        return true;
    }
    dlink_t *col = dlinkFindCol(header,  get_lowest_col_mem(window));
    cover(col);

    dlink_t *nextDown;
    dlink_t *nextRight;
    dlink_t *nextLeft;

    for (nextDown = col->down; nextDown != col;
       nextDown = nextDown->down) {
        stack_push(chosenRows, nextDown);
        for (nextRight = nextDown->next; nextDown != nextRight;
            nextRight = nextRight->next) {
        cover(nextRight);
        }

        switch (serial_dlx(window, header, chosenRows, count+1)) {
            case true:
                return true;
                break;
            case OTHER_RANK:
                return OTHER_RANK;
                break;
            default:
                break;
        }

        stack_pop(chosenRows);

        col = nextDown->colHeader;
        for (nextLeft = nextDown->prev; nextDown != nextLeft;
            nextLeft = nextLeft->prev) {
            uncover(nextLeft);
        }
    }
    uncover(col);
    return false;
    
}


int main(int argc, char* argv[]) {
    int num_procs, my_rank;
    MPI_Win window;
    char **sudoku;
    FILE *fp;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    myranktemp = my_rank;

    fp = fopen(argv[1], "r");
    int *window_buffer;
    MPI_Win_allocate((MPI_Aint)(COLS * sizeof(int)), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &window_buffer, &window);

    dlink_t *header = dlinkEmpty();
    createRows(header);
    stack_t *chosenRows = stack_create(); 

    sudoku = calloc(9, sizeof(char *));

    for (int i = 0; i < 9; i++) {
        sudoku[i] = calloc(9, sizeof(char));
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();
    while (!feof(fp)) {
        toArrDot(sudoku, fp);
        if (!feof(fp)) {
            fseek(fp, 1, SEEK_CUR);
        }
        removeClues(header, sudoku, chosenRows);

        if (my_rank == ROOT) {
            MPI_Win_lock(MPI_LOCK_EXCLUSIVE, ROOT, 0, window);
            set_window_buffer(window_buffer, header);
            MPI_Win_unlock(ROOT, window);
        }

        MPI_Barrier(MPI_COMM_WORLD);

        switch (mpi_dlx(window, header, chosenRows, 0)) {
            case true:
                break;
            case OTHER_RANK:
                break;
            case false:
                found_solution(window);
                break;
            default:
                break;
        } 
        
        MPI_Barrier(MPI_COMM_WORLD);
        
        reset_cover(chosenRows);
    }
    
    fclose(fp);

    double end = MPI_Wtime();

    double time = end - start;
    

    if (my_rank == ROOT) {
        printf("Time taken: %f\n", time);
    }

    // Destroy the window
    MPI_Win_free(&window);

    MPI_Finalize();
    return 0;
}
