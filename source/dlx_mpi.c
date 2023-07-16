#include <stdbool.h>
#include <stdio.h>
#include <openmpi-x86_64/mpi.h>
#include <string.h>

#include "dlink.h"
#include "dlx.h"
#include "stack.h"

#define COLS 324
#define SUDOKU_SIZE 81
#define ROOT 0

int numrows;
/**
 * @brief  allocates a 2d array of chars
 * 
 * @param array array to allocate
 * @param n number of rows
 * @param m number of columns
 * @return int 0 on success, -1 on failure
 */
int malloc2dchar(char ***array, int n, int m) {

    /* allocate the n*m contiguous items */
    char *p = (char *)malloc(n*m*sizeof(char));
    if (!p) return -1;

    /* allocate the row pointers into the memory */
    (*array) = (char **)malloc(n*sizeof(char*));
    if (!(*array)) {
       free(p);
       return -1;
    }

    /* set up the pointers into the contiguous memory */
    for (int i=0; i<n; i++)
       (*array)[i] = &(p[i*m]);

    return 0;
}

int mpi_2dchar_win(char ***array, int n, int m, MPI_Comm comm, MPI_Win *win) {
    MPI_Aint size;
    char *baseptr;

    /* calculate the size of the contiguous memory block */
    size = n * m * sizeof(char);

    MPI_Aint displacement = m * sizeof(char);
    /* allocate the memory window */
    MPI_Win_allocate(size, displacement, MPI_INFO_NULL, comm, &baseptr, win);
    /* allocate the row pointers into the memory */
    (*array) = (char **)malloc(n * sizeof(char *));
    if (!(*array)) {
        MPI_Win_free(win);
        return -1;
    }

    /* set up the pointers into the contiguous memory */
    for (int i = 0; i < n; i++) {
        (*array)[i] = &(baseptr[i * m]);
    }

    return 0;
}

void read_sudoku_win(char **sudokus, FILE *fp) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    getline(&line, &len, fp);
    for (int i = 0; i < numrows; i++) {
        read = getline(&line, &len, fp);
        if (read == -1) {
            fprintf(stderr, "Error reading sudoku file\n");
            exit(1);
        }
        strncpy(sudokus[i], line, SUDOKU_SIZE);
        sudokus[i][SUDOKU_SIZE+1] = '\0';
    }
}

void read_num_rows(FILE *fp) {
    int num_sudokus;
    fscanf(fp, "%d", &num_sudokus);
    numrows = num_sudokus;
}

/**
 * @brief reads a sudoku file and returns an array of single line sudoku strings
 *
 * @param fp file pointer to sudoku file
 * @return char** array of sudoku strings
 */
char **read_sudoku(FILE *fp) {
    /* read sudoku file with following format:
    first line: number of sudokus
    81 characters per line
    0-9 and . are valid characters
    remove newlines and whitespaces
    */
    int num_sudokus;
    char **sudokus;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fscanf(fp, "%d", &num_sudokus);
    getline(&line, &len, fp);
    numrows = num_sudokus;
    malloc2dchar(&sudokus, numrows, SUDOKU_SIZE+1);
    for (int i = 0; i < num_sudokus; i++) {
        read = getline(&line, &len, fp);
        if (read == -1) {
            fprintf(stderr, "Error reading sudoku file\n");
            exit(1);
        }
        strncpy(sudokus[i], line, SUDOKU_SIZE);
        sudokus[i][SUDOKU_SIZE] = '\0';
    }

    return sudokus;
}

void remove_clues(dlink_t *header, char* sudoku, stack_t *chosenRows) {
    for (int i = 0; i < 9 ; i++) {
        for (int j = 0; j < 9; j++) {
            if (sudoku[i*9 + j] != '0' && sudoku[i*9 + j] != '.') {
                sudoku_t toAdd;
                toAdd.data = sudoku[i*9 + j] - '0';
                toAdd.cellNum = (9 * i) + j + 1;

                dlink_t *node = findRow(&toAdd, header);
                if (node != NULL) {
                    cover(node->colHeader);
                    dlink_t* rowNode = node->next;
                    while(rowNode != node) {
                        cover(rowNode->colHeader);
                        rowNode = rowNode->next;
                    }
                    stack_push(chosenRows, node);
                }
            }
        }
    }
}

int get_next_sudoku(MPI_Win sudoku_num_win, int granularity) {
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, ROOT, 0, sudoku_num_win);
    int sudoku_num;
    MPI_Get(&sudoku_num, 1, MPI_INT, ROOT, 0, 1, MPI_INT, sudoku_num_win);
    //check if sudoku_num is out of bounds
    if (sudoku_num >= numrows) {
        MPI_Win_unlock(ROOT, sudoku_num_win);
        return -1;
    }
    else {
        sudoku_num += granularity;
    }
    MPI_Put(&sudoku_num, 1, MPI_INT, ROOT, 0, 1, MPI_INT, sudoku_num_win);
    MPI_Win_unlock(ROOT, sudoku_num_win);
    return sudoku_num-1;
}

void get_sudoku(char** sudoku, MPI_Win window, int sudoku_index, int granularity) {
    int num_get = granularity;
    if (sudoku_index + granularity > numrows) {
        num_get = numrows - sudoku_index;
    }

    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, ROOT, 0, window);
    for (int i = 0; i < num_get; i++) {
        MPI_Get(sudoku[i], SUDOKU_SIZE+1, MPI_CHAR, ROOT, sudoku_index+i, SUDOKU_SIZE+1, MPI_CHAR, window);
    }
    MPI_Win_unlock(ROOT, window);
}

void shared_memory_loop(MPI_Win window, MPI_Win sudoku_num_win, dlink_t *header, stack_t *chosenRows, int granularity) {
    int sudoku_num = 0;
    char **sudoku;
    malloc2dchar(&sudoku, granularity, SUDOKU_SIZE+1);
    fprintf(stderr, "granularity: %d\n", granularity);
    while ((sudoku_num = get_next_sudoku(sudoku_num_win, granularity)) != -1) {
        fprintf(stderr, "granularity: %d\n", granularity);
        get_sudoku(sudoku, window, sudoku_num, granularity);
        for (int i = 0; i < granularity; i++) {

            remove_clues(header, sudoku[i], chosenRows);
            fprintf(stderr, "granularity: %d\n", granularity);
            if (!dlx(header, chosenRows, 0)) {
                fprintf(stderr, "Sudoku %d has no solution\n", sudoku_num);
            }
            reset_cover(chosenRows);
        }
    }
    free(sudoku);
}

void worker_help(char *sudoku, dlink_t* header, stack_t* chosenRows) {
    remove_clues(header, sudoku, chosenRows);
    if (!dlx(header, chosenRows, 0)) {
        fprintf(stderr, "Sudoku has no solution\n");
    }
    reset_cover(chosenRows);
}

void root_loop(char **sudokus, int num_procs, int granularity) {
    int sudoku_num = 0; //index of sudoku to send
    while(sudoku_num < numrows) {
        //recv work request and send work
        int worker_rank;
        MPI_Status status;
        MPI_Recv(&worker_rank, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        for (int i = 0; i < granularity; i++) {
            if (sudoku_num+i >= numrows) {
                sudokus[0][0] = '\0';
                MPI_Send(sudokus[0], SUDOKU_SIZE+1, MPI_CHAR, worker_rank, 0, MPI_COMM_WORLD);
            }
            else {
                 MPI_Send(sudokus[sudoku_num+i], SUDOKU_SIZE+1, MPI_CHAR, worker_rank, 0, MPI_COMM_WORLD);
            }
        }
        sudoku_num+=granularity;
    }

    sudokus[0][0] = '\0';
    //send termination signal
    for (int i = 1; i < num_procs; i++) {
        for (int j = 0; j < granularity; j++) {
            MPI_Send(sudokus[0], SUDOKU_SIZE+1, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        }
    }
    fprintf(stderr, "Finished sending sudokus num sudokus %d\n", sudoku_num);
}

void worker_loop(dlink_t *header, stack_t *chosenRows, int my_rank, int granularity) {
    char** sudoku;
    malloc2dchar(&sudoku, granularity, SUDOKU_SIZE+1);
    while (1) {
        //send work request
        MPI_Send(&my_rank, 1, MPI_INT, ROOT, 0, MPI_COMM_WORLD);
        //recv work
        MPI_Status status;
        for (int i = 0; i < granularity; i++) {
            MPI_Recv(sudoku[i], SUDOKU_SIZE+1, MPI_CHAR, ROOT, 0, MPI_COMM_WORLD, &status);
        }

        if (sudoku[0][0] == '\0') {
            fprintf(stderr, "recieved termination signal in rank %d\n", my_rank);
            break;
        }
       // fprintf(stderr, "recieved sudoku in rank %d\n", my_rank);
        for (int i = 0; i < granularity; i++) {
            worker_help(sudoku[i], header, chosenRows);
        }
    }
}

int main(int argc, char *argv[]) {
    int my_rank, num_procs, opt, granularity;
    char **sudokus;
    double start, end;
    MPI_Win window;
    FILE *fp;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (my_rank == ROOT) {
        if (argc != 4) {
            printf("Usage: %s <sudoku file> <opt 0 1> (0 for put get, 1 for send recieve) <granularity> (number of sudokus to get and recieve)\n", argv[0]);
            return 1;
        }

        fp = fopen(argv[1], "r");
        opt = atoi(argv[2]);
        granularity = atoi(argv[3]);
        if (fp == NULL) {
            fprintf(stderr, "Error opening file %s\n", argv[1]);
            return 1;
        }
        if (opt == 0) {
            //Forced to do this because MPI_Win_create doesn't work on Debian 11
            read_num_rows(fp);
        }
        else {
            sudokus = read_sudoku(fp);
            fclose(fp);
        }
    }

    MPI_Bcast(&numrows, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(&opt, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(&granularity, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

    dlink_t *header = dlinkEmpty();
    createRows(header);
    stack_t *chosenRows = stack_create();

    if (opt == 0) {
        MPI_Win sudoku_num_win;
        int sudoku_num_val;
        MPI_Win_allocate(sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &sudoku_num_val, &sudoku_num_win);
        //Forced to do this because MPI_Win_create doesn't work on Debian 11
        mpi_2dchar_win(&sudokus, numrows, SUDOKU_SIZE+1, MPI_COMM_WORLD, &window);
        if (my_rank == ROOT) {
            sudoku_num_val = 0;
            //Forced to do this because MPI_Win_create doesn't work on Debian 11
            read_sudoku_win(sudokus, fp);
            fclose(fp);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if (my_rank == ROOT) {
            start = MPI_Wtime();
        }
        shared_memory_loop(window, sudoku_num_win, header, chosenRows, granularity);
        if (my_rank == ROOT) {
            end = MPI_Wtime();
            fprintf(stdout, "%f\n", end-start);
        }
        MPI_Win_free(&sudoku_num_win);
    }

    else if (opt == 1) {
        if (my_rank == ROOT) {
            start = MPI_Wtime();
            root_loop(sudokus, num_procs, granularity);
            end = MPI_Wtime();
            fprintf(stdout, "%f\n", end-start);
        }
        else {
            worker_loop(header, chosenRows, my_rank, granularity);
        }
    }

    else if (opt == 2) {
        if (my_rank == ROOT) {
            start = MPI_Wtime();
            for (int i = 0; i < numrows; i++) {
                remove_clues(header, sudokus[i], chosenRows);
                if (!dlx(header, chosenRows, 0)) {
                    fprintf(stderr, "Sudoku %d has no solution\n", i);
                }
                reset_cover(chosenRows);
            }
            end = MPI_Wtime();
            fprintf(stdout, "%f\n", end-start);
        }
    }

    dlinkKill(header);
    stack_kill(chosenRows);
    if (my_rank == ROOT) {
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();
    return 0;
}
