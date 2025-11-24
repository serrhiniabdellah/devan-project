#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// MSB encoding for non-terminals
#define IS_TERMINAL(c) (((unsigned char)(c)) < 128)
#define IS_NONTERMINAL(c) (((unsigned char)(c)) >= 128)
#define MAKE_NONTERMINAL(c) ((char)((c) | 0x80))
#define GET_CHAR(c) ((c) & 0x7F)

// Maximum limits
#define MAX_RULES 100
#define MAX_PRODUCTION_LEN 100
#define MAX_STATES 100
#define TABLE_COLS 256

// Grammar rule structure
typedef struct {
    char lhs;           // Left-hand side (non-terminal with MSB set)
    char rhs[MAX_PRODUCTION_LEN];  // Right-hand side (production)
    int rhs_len;        // Length of production
} Rule;

// Grammar structure
typedef struct {
    char axiom;         // Start symbol (non-terminal with MSB set)
    Rule* rules;        // Array of rules
    int num_rules;      // Number of rules
} Grammar;

// LR parsing table
typedef struct {
    short* data;        // Linearized 2D array [states][256]
    int num_states;     // Number of states
} Table;

// Tree node for parse tree
typedef struct Node {
    char symbol;        // Symbol (terminal or non-terminal)
    struct Node** children;  // Array of child nodes
    int num_children;   // Number of children
    int capacity;       // Allocated capacity for children
} Node;

// Stack element for LR parser
typedef struct {
    int state;          // State number
    Node* node;         // Parse tree node
} StackElement;

// Stack structure
typedef struct {
    StackElement* elements;
    int top;
    int capacity;
} Stack;

#endif // STRUCTS_H
