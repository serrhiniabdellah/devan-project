#include "structs.h"

// Create a new tree node
Node* create_node(char symbol) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->symbol = symbol;
    node->children = NULL;
    node->num_children = 0;
    node->capacity = 0;
    return node;
}

// Add a child to a node
void add_child(Node* parent, Node* child) {
    if (parent->num_children >= parent->capacity) {
        parent->capacity = (parent->capacity == 0) ? 4 : parent->capacity * 2;
        parent->children = (Node**)realloc(parent->children, parent->capacity * sizeof(Node*));
    }
    parent->children[parent->num_children++] = child;
}

// Print tree in the format S(a()...) recursively
void print_tree_recursive(Node* node) {
    if (!node) return;
    
    // Print the symbol
    if (IS_NONTERMINAL(node->symbol)) {
        printf("%c", GET_CHAR(node->symbol));
    } else {
        printf("%c", node->symbol);
    }
    
    // If this node has children, print them in parentheses
    if (node->num_children > 0) {
        printf("(");
        for (int i = 0; i < node->num_children; i++) {
            print_tree_recursive(node->children[i]);
        }
        printf(")");
    } else {
        // Leaf node - print empty parentheses for terminals
        if (IS_TERMINAL(node->symbol)) {
            printf("()");
        }
    }
}

// Print the entire tree
void print_tree(Node* root) {
    if (!root) {
        printf("Empty tree\n");
        return;
    }
    print_tree_recursive(root);
    printf("\n");
}

// Free tree memory recursively
void free_tree(Node* node) {
    if (!node) return;
    
    for (int i = 0; i < node->num_children; i++) {
        free_tree(node->children[i]);
    }
    
    if (node->children) {
        free(node->children);
    }
    free(node);
}
