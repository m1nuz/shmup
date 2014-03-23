#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "stack.h"

struct STACK
{
    void        *elements;
    size_t      element_size;
    size_t      elements_count;
    int         top;
};


extern struct STACK *
new_stack(size_t element_size, size_t elements)
{
    assert(element_size > 0);
    assert(elements > 0);

    struct STACK *stack = malloc(sizeof(struct STACK));

    stack->elements = malloc(element_size * elements);
    stack->element_size = element_size;
    stack->elements_count = elements;
    stack->top = -1;

    return stack;
}

extern void
delete_stack(struct STACK *stack)
{
    free(stack->elements);
    free(stack);
}

extern void
push_stack(struct STACK *stack, void *element)
{
    assert(stack != NULL);

    stack->top++;

    memcpy(stack->elements + stack->top * stack->element_size, element, stack->element_size);
}

extern void*
pop_stack(struct STACK *stack)
{
    assert(stack != NULL);

    if(stack->top == -1)
        return NULL;

    void *element = stack->elements + stack->top * stack->element_size;

    stack->top--;

    return element;
}

extern void*
top_stack(struct STACK *stack)
{
    assert(stack != NULL);

    if(stack->top == -1)
        return NULL;

    return stack->elements + stack->top * stack->element_size;
}

extern int
is_stack_empty(struct STACK *stack)
{
    assert(stack != NULL);

    if(stack->top == -1)
        return 1;

    return 0;
}
