/**
 * @file linkedstack.c
 * @author Henrik Aili (c20hai@cs.umu.se)
 * @brief linked stack
 * @version 1.0
 * @date 2023-03-28
 */

#include "stack.h"
#include "dlink.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct snode_t snode_t;

struct stack_t {
    int count;
    snode_t *next;
};

typedef struct snode_t {
    void *data;
    snode_t *next;
} snode_t;

stack_t* stack_create() {
    stack_t *stack = (stack_t *)malloc(sizeof(stack_t));
    stack->count = 0;
    stack->next = NULL;
    return stack;
}

void stack_push(stack_t *stack, void *data) {
    assert(stack != NULL);
    
    snode_t* temp;

    temp = (snode_t *)malloc(sizeof(snode_t));
    temp->data = data;
    temp->next = stack->next;

    stack->next = temp;
    stack->count++;
}

void stack_push_prio(stack_t *stack, void *data);


void* stack_inspect(stack_t *stack) {
    if (stack->next != NULL) {
        return stack->next->data;
    }
    return NULL;
}

void stack_pop(stack_t *stack) {
    snode_t *temp;
    if (stack->next == NULL) {
        return;
    }
    temp = stack->next;
    stack->next = stack->next->next; 
    free(temp);
    stack->count--;
}

int stack_count(stack_t *stack) {
    return stack->count;
}

bool stack_dup(stack_t *stack, void *data) {
    dlink_t *check = (dlink_t*) data;
    dlink_t *toCheck;
    snode_t *temp = stack->next;
    while (temp != NULL) {
        toCheck = (dlink_t*)temp->data;
        if ((toCheck->col == check->col) && (toCheck->row == check->row))
            return true;
        temp = temp->next;
    }
    return false;
}

bool stack_empty(stack_t *stack) {
    return stack->count <= 0;
}

void stack_kill(stack_t *stack) {
    while (stack->next != NULL) {
       stack_pop(stack);
    }
    free(stack);
}