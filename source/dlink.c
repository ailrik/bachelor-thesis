/**
 * @file dlink.c
 * @author Henrik Aili (c20hai@cs.umu.se)
 * @brief 
 * @version 0.1
 * @date 2023-03-27
 */

#include "dlink.h"
#include <stdio.h>
#include <stdlib.h>

#define ROWS 729
#define COLS 324

/**
 * @brief Create a Node struct
 * 
 * @param row row number 
 * @param col col number
 * @return dlink_t* created node
 */
dlink_t* createNode(int row, int col, int data, int realVal, int realCell) {
    dlink_t* node = malloc(sizeof(dlink_t));

    node->data = data;
    node->row = row;
    node->col = col;
    node->next = NULL;
    node->prev = NULL;
    node->up = NULL;
    node->down = NULL;
    node->numNodes = 0;
    node->realVal = realVal;
    node->realCell = realCell;
    node->colHeader = node;

    return node;
}

/**
 * @brief Creates an empty dlink 
 * 
 * @param dimension dimensions
 * @return dlink_t* dlink
 */
dlink_t* dlinkEmpty() {
    dlink_t* temp = createNode(-1, -1, -1, -1, -1);
    dlink_t* prev = createNode(0, 1, -1, -1, -1);
    dlink_t* next; 

    temp->colHeader = temp;
    temp->next = prev;
    prev->prev = temp;
    prev->up = prev;
    prev->down = prev;
    prev->colHeader = prev;

    for (int i = 2; i <= COLS; i++) {
        next = createNode(0, i, -1, -1, -1);
        next->up = next;
        next->down = next;
        next->prev = prev;
        next->colHeader = next;
        prev->next = next;
        prev = next;
    }
    next->next = temp;
    temp->prev = next;

    return temp;
}

/**
 * @brief finds the node with the given row and col 
 *
 * @param header header of dlink
 * @param row row number
 * @param col col number
 * @return dlink_t* node with the given row and col
 */
dlink_t* dlinkFind(dlink_t* header, int row, int col) {
    dlink_t* temp = header->next;
    dlink_t* check = temp;

    while (temp->col != col) {
        temp = temp->next;
    }
    while (temp->row != row) {
        temp = temp->down;
        if(temp->row == check->row){
            return NULL;
        }
    }

    return temp;
}

dlink_t* dlinkFindCol(dlink_t* header, int col) {
    dlink_t* temp = header->next;
    while (temp->col != col) {
        temp = temp->next;
    }

    return temp;
}

/**
 * @brief kills the dlink
 * 
 * @param header header of dlink 
 */
void dlinkKill(dlink_t *header) {
    while(header->next != header) {
        dlink_t* temp = header->next;
        dlink_t* temp2 = temp->down;
        
        while(temp2 != temp) {
            dlink_t* temp3 = temp2->down;
            free(temp2);
            temp2 = temp3;
        }
        header->next = temp->next;
        free(temp);
    }
    free(header);
}
