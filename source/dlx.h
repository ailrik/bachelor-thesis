#ifndef DLX_H
#define DLX_H

#include "dlink.h"
#include "parser.h"
#include "stack.h"

void removeClues(dlink_t *header, char **arr, stack_t *chosenRows);

dlink_t* chooseColumn(dlink_t *header);

bool dlx(dlink_t *header, stack_t *chosenRows, int count);

void cover(dlink_t *target);

void uncover(dlink_t *target);

void multisudoku_loop(dlink_t* header, stack_t* chosenRows, char** arr, FILE* fp, int count);

void reset_cover(stack_t* chosenRows);

void printSudoku(stack_t *stack);


#endif
