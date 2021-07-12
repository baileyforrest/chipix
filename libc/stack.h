#pragma once

#include <stddef.h>

typedef struct Stack {
  struct Stack* next;
} Stack;

static inline void stack_init(Stack* stack) { stack->next = NULL; }

static inline void stack_push(Stack** stack, Stack* node) {
  node->next = (*stack)->next;
  *stack = node;
}

static inline Stack* stack_pop(Stack** stack) {
  Stack* ret = (*stack)->next;
  if (ret == NULL) {
    return NULL;
  }

  *stack = ret->next;
  return ret;
}
