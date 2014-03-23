#pragma once

#include <stddef.h>

struct STACK;

struct STACK * new_stack(size_t element_size, size_t elements);
void delete_stack(struct STACK *stack);
void push_stack(struct STACK *stack, void *element);
void *pop_stack(struct STACK *stack);
void *top_stack(struct STACK *stack);
int is_stack_empty(struct STACK *stack);
