#ifndef DLINK_H
#define DLINK_H

/**
 * @file dlink.h
 * @author Henrik Aili (c20hai@cs.umu.se)
 * @brief doubly linked circular list 
 * @version 1.0
 * @date 2023-03-27
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
typedef struct dlink_t dlink_t ;

typedef struct dlink_t {
    int data;
    int col;
    int row;
    //column headers ONLY
    int numNodes;
    int realVal;
    int realCell;

    dlink_t* next;
    dlink_t* prev;
    dlink_t* up;
    dlink_t* down;
    dlink_t* colHeader;
} dlink_t;

/**
 * @brief Create a Node struct
 * 
 * @param row row number 
 * @param col col number
 * @return dlink_t* created node
 */
dlink_t* createNode(int row, int col, int data, int realVal, int realCell);

/**
 * @brief Creates empty dlink
 * 
 * @param dimension dimension of matrix 
 * @return dlink_t* an empty d-linked list
 */
dlink_t* dlinkEmpty();  

/**
 * @brief Kills the dlink from header
 * 
 * @param header the header of the dlink
 */
void dlinkKill(dlink_t *header);

/**
 * @brief finds the node with the given row and col 
 *
 * @param header header of dlink
 * @param row row number
 * @param col col number
 * @return dlink_t* node with the given row and col
 */
dlink_t* dlinkFind(dlink_t* header, int row, int col);

/**
 * @brief finds the header node of the col 
 * 
 * @param header header node 
 * @param col column to find
 * @return dlink_t* header node of column
 */
dlink_t* dlinkFindCol(dlink_t* header, int col);

#endif
