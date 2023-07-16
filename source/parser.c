#include "parser.h"
#include "dlink.h"
#include <stdio.h>
#include <stdlib.h>

#define COLS 324
#define ROWS 729

/**
 * @brief Reads sudoku from a file to an array
 * 
 * @param arr 
 * @param fp 
 */
void toArr(char **arr, FILE *fp) {
    for(int i=0; i<9; i++) {
        for(int j=0; j<9; j++) {
            fscanf(fp,"%c",&arr[i][j]);
        }
        fseek( fp, 1, SEEK_CUR );
    }
}

void toArrDot(char **arr, FILE *fp) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            fscanf(fp,"%c",&arr[i][j]);
        }
    }
}

/**
 * @brief Inserts a node in the correct column
 * 
 * @param node 
 * @param header 
 */
static void insertCol(dlink_t *node, dlink_t *header){
    dlink_t *next = header->next;

    while(next->col != node->col){
        next = next->next;
    }

    next->numNodes++;

    dlink_t *downNext = next->down;

    while(!((node->row >= downNext->row))){
        downNext = downNext->down;
    }

    node->down = downNext;
    node->up = downNext->up;

    downNext->up->down = node;
    downNext->up = node;

    node->colHeader = next;
    
}

/**
 * @brief Creates nodes to add to the doubly linked list
 * 
 * @param header 
 */
void createRows(dlink_t *header){
    int cellVal = 1;
    int cellCount = 0;
    int rowVal = 0;

    for(int i = 1; i <= ROWS; i++){
        //create cellNode
        dlink_t *cell = createNode(i, cellCount + 1, 1, cellVal, cellCount);
        //create rowNode
        dlink_t *row = createNode(i, 81 +  ((rowVal * 9) + cellVal), 1, cellVal, cellCount);
        //create colNode
        dlink_t *col = createNode(i, (81 * 2) + ((cellCount % 9) * 9)+ cellVal, 1, cellVal, cellCount);
        //create boxNode
        dlink_t *box = createNode(i, (81 * 3) + ((((cellCount % 9)/3) + ((rowVal/3) * 3)) * 9) + cellVal, 1, cellVal, cellCount);

        //Link nodes in one row
        cell->next = row;
        cell->prev = box;

        row->next = col;
        row->prev = cell;

        col->next = box;
        col->prev = row;

        box->next = cell;
        box->prev = col;
        
        insertCol(cell, header);
        insertCol(row, header);
        insertCol(col, header);
        insertCol(box, header);
        
        cellVal++;
        if(cellVal == 10){
            cellCount++;
            cellVal = 1;
            if((cellCount % 9) == 0){
                rowVal++;
            }
        }
    }
}

/**
 * @brief Can only be used on a complete dlink_t. 
 * Assumes that the node received is the cell node and has the row node as its next node
 * 
 * @param node 
 * @return sudoku_t* 
 */
sudoku_t* rowToSudoku(dlink_t *node){
    sudoku_t *square = malloc(sizeof(sudoku_t));
    square->cellNum = node->realCell;
    square->data = node->realVal;

    return square; 
}

dlink_t* findRow(sudoku_t *square, dlink_t *header){
    int cell = square->cellNum;
    int data = square->data;

    int row = ((cell - 1) * 9) + data;
    return dlinkFind(header, row, cell);
}
