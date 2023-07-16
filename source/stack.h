#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

/**
 * @file linkedstack.h
 * @author Henrik Aili (c20hai@cs.umu.se)
 * @brief linkedstack header-file
 * @version 1.0
 * @date 2023-03-28
 */

typedef struct stack_t stack_t;

stack_t* stack_create();

void* stack_inspect(stack_t *stack);

void stack_pop(stack_t *stack);

void stack_push(stack_t *stack, void *data);

int stack_count(stack_t *stack);

bool stack_dup(stack_t *stack, void *data);

bool stack_empty(stack_t *stack);

void stack_kill(stack_t *stack);

#endif 