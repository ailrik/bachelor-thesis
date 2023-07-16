#include <bits/types/FILE.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "dlink.h"

typedef struct sudoku_t {
    int data;
    int cellNum;
} sudoku_t;

void toArr(char **arr, FILE *fp);

void toArrDot(char **arr, FILE *fp);

void createRows(dlink_t *header);

sudoku_t* rowToSudoku(dlink_t *node);

dlink_t* findRow(sudoku_t *square, dlink_t *header);