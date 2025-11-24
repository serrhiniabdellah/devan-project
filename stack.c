#include "structs.h"

// Create a new stack
Stack* create_stack(int initial_capacity) {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    stack->capacity = initial_capacity;
    stack->elements = (StackElement*)malloc(initial_capacity * sizeof(StackElement));
    stack->top = -1;
    return stack;
}

// Push (state, node) onto stack
void push(Stack* stack, int state, Node* node) {
    if (stack->top + 1 >= stack->capacity) {
        stack->capacity *= 2;
        stack->elements = (StackElement*)realloc(stack->elements, stack->capacity * sizeof(StackElement));
    }
    stack->top++;
    stack->elements[stack->top].state = state;
    stack->elements[stack->top].node = node;
}

// Pop from stack
StackElement pop(Stack* stack) {
    if (stack->top < 0) {
        fprintf(stderr, "Error: Stack underflow\n");
        exit(1);
    }
    return stack->elements[stack->top--];
}

// Peek at top of stack (state only)
int peek_state(Stack* stack) {
    if (stack->top < 0) {
        return -1;
    }
    return stack->elements[stack->top].state;
}

// Get element at position from top (0 = top, 1 = one below, etc.)
StackElement* get_element(Stack* stack, int offset) {
    if (stack->top - offset < 0) {
        return NULL;
    }
    return &stack->elements[stack->top - offset];
}

// Check if stack is empty
int is_empty(Stack* stack) {
    return stack->top < 0;
}

// Get stack size
int stack_size(Stack* stack) {
    return stack->top + 1;
}

// Print stack for debugging
void print_stack(Stack* stack) {
    printf("Stack: [");
    for (int i = 0; i <= stack->top; i++) {
        printf("%d", stack->elements[i].state);
        if (i < stack->top) printf(", ");
    }
    printf("]\n");
}

// Free stack memory
void free_stack(Stack* stack) {
    free(stack->elements);
    free(stack);
}
