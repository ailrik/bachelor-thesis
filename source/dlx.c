/**
 * @file dlx.c
 * @author Leo Hall (dv20lhl@cs.umu.se)
 * @author Henrik Aili (c20hai@cs.umu.se)
 * @brief Algorithm X
 * @version 0.1
 * @date 2023-03-29
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "dlx.h"
#include "dlink.h"
#include "stack.h"
#include <bits/getopt_core.h>
#include <bits/time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <getopt.h>
#include <time.h>
#include <stdint.h>

long double calc_time_diff(struct timespec stop, struct timespec start) {
    return ((long double)(stop.tv_sec - start.tv_sec)) + ((long double) (stop.tv_nsec - start.tv_nsec))/1000000000;
}

/*
int main(int argc, char *argv[]) {
  int fileformat = 0;
  int multisudoku = 0;
  int opt = 0;
  FILE *fp;
  if (argc <= 2) {
    fprintf(stderr, "Too few arguments, usage:\n\t[optional] -d (file is in dot notation) \n\t[optional] -m (multiple sudokus in one file) \n\t[required] -f {file} \n");
    exit(EXIT_FAILURE);
  }

  while ((opt = getopt(argc, argv, "dmf:")) != -1) {
    switch (opt) {
      case 'd':
        fileformat = 1;
        break;

      case 'm':
        multisudoku = 1;
        break;
      
      case 'f':
        fp = fopen(optarg, "r");
        break;

      case '?':
        exit(EXIT_FAILURE);
        break;

      default:
        break;
    }
  }
  if (optind < argc) {
    fprintf(stderr, "Too many arguments \n"); 
    exit(EXIT_FAILURE);
  }



  char **arr = calloc(9, sizeof(char *));

  for (int i = 0; i < 9; i++) {
    arr[i] = calloc(9, sizeof(char));
  }

  dlink_t *header = dlinkEmpty();
  createRows(header);
  stack_t *chosenRows = stack_create(); 
  
  dlink_t* temp = header->next;
  while (temp!=header) {
    if (temp->numNodes != 9)
      fprintf(stderr, "num nodes = %d, row = %d, col = %d\n", temp->numNodes, temp->row, temp->col);
    temp = temp->next;
  }

  //stack_t *temp;
  int count = 0;
  if (multisudoku == 1) {
    multisudoku_loop(header, chosenRows, arr, fp, count);  
  }
  else {
    toArr(arr, fp);
    removeClues(header, arr, chosenRows);
    dlink_t* temp = header->next;
    while (temp!=header) {
      
        fprintf(stderr, "num nodes = %d, row = %d, col = %d\n", temp->numNodes, temp->row, temp->col);
      temp = temp->next;
    }
    if (dlx(header, chosenRows, count)) {
      dlink_t* temp2 = header->next;
      while (temp2!=header) {
        if (temp2->numNodes != 9)
          fprintf(stderr, "num nodes = %d, row = %d, col = %d\n", temp2->numNodes, temp2->row, temp2->col);
        temp2 = temp2->next;
      }
      fprintf(stderr, "you win\n");
      reset_cover(chosenRows);
    }
  }

  for (int i = 0; i < 9; i++) {
    free(arr[i]); 
  }
  free(arr);
  fclose(fp);
  dlinkKill(header);
  stack_kill(chosenRows);
  //stack_kill(temp);

  return 0;
}

*/

/**
 * @brief Removes clues on the sudoku board from the dlink structure
 *
 * @param header the header node
 * @param arr char matrix containing the sudoku board 
 * @param chosenRows stack of rows that has been covered
 */
void removeClues(dlink_t *header, char **arr, stack_t *chosenRows) {
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      if (arr[i][j] != '0' && arr[i][j] != '.') {
        sudoku_t toAdd;
        toAdd.data = arr[i][j] - '0';
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

/**
 * @brief Choses a column with least count for Algorithm X
 *
 * @param header header node
 * @return dlink_t* pointer to column with least count
 */
dlink_t *chooseColumn(dlink_t *header) {
  dlink_t *temp = header->next->next;
  dlink_t *ret = header->next;
  while (temp->next != header->next) {
    if (temp->numNodes < ret->numNodes && (temp->numNodes > 0)) {
      ret = temp;
    }
    temp = temp->next;
  }
  return ret;
}

bool dlx(dlink_t *header,
         stack_t *chosenRows, int count) { 
  if (header->next == header) {
    fprintf(stderr, "Found solution at depth %d\n", count);
    return true;
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

    if (dlx(header, chosenRows, count+1)) {
      return true;
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

void cover(dlink_t *target) {
  dlink_t *row, *nextRight;

  dlink_t *col = target->colHeader;

  col->prev->next = col->next;
  col->next->prev = col->prev;

  for (row = col->down; row != col; row = row->down) {
    for (nextRight = row->next; row != nextRight;
         nextRight = nextRight->next) {
      nextRight->up->down = nextRight->down;
      nextRight->down->up = nextRight->up;
      nextRight->colHeader->numNodes--;
    }
  }
}

void uncover(dlink_t *target) {
  dlink_t *row, *nextLeft;

  dlink_t *col = target->colHeader;

  for (row = col->up; row != col; row = row->up) {
    for (nextLeft = row->prev; row != nextLeft;
         nextLeft = nextLeft->prev) {
      nextLeft->up->down = nextLeft;
      nextLeft->down->up = nextLeft;
      nextLeft->colHeader->numNodes++;
    }
  }

  col->prev->next = col;
  col->next->prev = col;

}

void multisudoku_loop(dlink_t* header, stack_t* chosenRows, char** arr, FILE* fp, int count) {
  struct timespec wall_time;
  struct timespec stop_walltime;
  long double totwalltime;
  clock_gettime(CLOCK_MONOTONIC, &wall_time);

  while (!feof(fp)) {
    toArrDot(arr, fp);
    if(!feof(fp)) {
      fseek(fp, 1, SEEK_CUR);
    }
    removeClues(header, arr, chosenRows);


    
    if (dlx(header, chosenRows, count) == false) {
      fprintf(stderr, "DLX: failed to solve a sudoku\n");
    }
    reset_cover(chosenRows);
  }

  //timing stuff
  clock_gettime(CLOCK_MONOTONIC, &stop_walltime);
  totwalltime = calc_time_diff(stop_walltime, wall_time);
  fprintf(stderr, "Total time taken in seconds: \n");
  fprintf(stdout, "%LF\n", (totwalltime));
}

void reset_cover(stack_t *chosenRows) {
  dlink_t *nextLeft;
  while (!stack_empty(chosenRows)) {
    dlink_t *r = stack_inspect(chosenRows);
    stack_pop(chosenRows);
    
    for (nextLeft = r->prev; r != nextLeft;
         nextLeft = nextLeft->prev) {
      uncover(nextLeft);
    }
    uncover(r->colHeader);
  }
}

void printSudoku(stack_t *stack) {
  int sudoku[81];
  dlink_t *temp = stack_inspect(stack);
  while (stack_empty(stack) == false) {
    sudoku[temp->realCell] = temp->realVal;
    stack_pop(stack);
    temp = stack_inspect(stack);
  }
  setlocale(LC_ALL, "");
  printf("╔═══╤═══╤═══╦═══╤═══╤═══╦═══╤═══╤═══╗\n");
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      switch (j%3) {
      case 0:
        printf("║");
        break;
      default:
        printf("│");
        break;
      }
      printf(" %d ", sudoku[(i * 9) + j]);
      
    }
    printf("║\n");
    if (i != 8) {
      switch (i%3) {
        case 2:
          printf("╠═══╪═══╪═══╬═══╪═══╪═══╬═══╪═══╪═══╣\n");
          break;
        default:
          printf("╟───┼───┼───╫───┼───┼───╫───┼───┼───╢\n"); 
          break;
      }
    }
  }
  printf("╚═══╧═══╧═══╩═══╧═══╧═══╩═══╧═══╧═══╝\n");
}
