#include "structs.h"

// Function prototypes from other modules
Node* create_node(char symbol);
void add_child(Node* parent, Node* child);
void print_tree(Node* root);
void free_tree(Node* node);

Stack* create_stack(int initial_capacity);
void push(Stack* stack, int state, Node* node);
StackElement pop(Stack* stack);
int peek_state(Stack* stack);
StackElement* get_element(Stack* stack, int offset);
int is_empty(Stack* stack);
int stack_size(Stack* stack);
void free_stack(Stack* stack);

// Print current parsing state (for trace output)
void print_trace(const char* input, int input_pos, Stack* stack) {
    // Print remaining input
    printf("Flot: ");
    for (int i = input_pos; input[i]; i++) {
        printf("%c", input[i]);
    }
    if (!input[input_pos]) {
        printf("$");
    }
    
    // Print stack states
    printf(" | Pile: ");
    for (int i = 0; i <= stack->top; i++) {
        printf("%d ", stack->elements[i].state);
    }
    printf("\n");
}

// Main LR parsing engine
int parse(Grammar* grammar, Table* table, const char* input, int trace) {
    Stack* stack = create_stack(100);
    
    // Initialize: push state 0 with null node
    push(stack, 0, NULL);
    
    int input_pos = 0;
    int input_len = strlen(input);
    
    if (trace) {
        printf("\n=== Parsing Trace ===\n");
        printf("Input: %s\n\n", input);
    }
    
    while (1) {
        int current_state = peek_state(stack);
        
        // Get current input symbol (or $ for end)
        char current_char;
        if (input_pos >= input_len) {
            current_char = '$';
        } else {
            current_char = input[input_pos];
        }
        
        if (trace) {
            print_trace(input, input_pos, stack);
        }
        
        // Look up action in table
        short action = table->data[current_state * TABLE_COLS + (unsigned char)current_char];
        
        if (action == 0) {
            // Error
            if (trace) {
                printf("\nERROR: No action for state %d, symbol '%c'\n", current_state, current_char);
            }
            printf("REJECT\n");
            free_stack(stack);
            return 0;
            
        } else if (action == -127) {
            // Accept
            if (trace) {
                printf("\nACCEPT\n");
            }
            
            // The parse tree is at the top of the stack
            if (stack->top >= 0 && stack->elements[stack->top].node) {
                printf("\nParse Tree:\n");
                print_tree(stack->elements[stack->top].node);
            }
            
            free_stack(stack);
            return 1;
            
        } else if (action > 0) {
            // Shift to state 'action'
            if (trace) {
                printf("Action: SHIFT %d\n", action);
            }
            
            // Create leaf node for this terminal
            Node* leaf = create_node(current_char);
            
            // Push new state and node
            push(stack, action, leaf);
            
            // Consume input character
            input_pos++;
            
        } else {
            // Reduce by rule abs(action)
            int rule_num = -action - 1;  // Convert to 0-based index
            
            if (rule_num < 0 || rule_num >= grammar->num_rules) {
                fprintf(stderr, "Error: Invalid rule number %d\n", rule_num + 1);
                free_stack(stack);
                return 0;
            }
            
            Rule* rule = &grammar->rules[rule_num];
            
            if (trace) {
                printf("Action: REDUCE by rule %d: %c ->", rule_num + 1, GET_CHAR(rule->lhs));
                for (int i = 0; i < rule->rhs_len; i++) {
                    if (IS_NONTERMINAL(rule->rhs[i])) {
                        printf(" $%c", GET_CHAR(rule->rhs[i]));
                    } else {
                        printf(" %c", rule->rhs[i]);
                    }
                }
                printf("\n");
            }
            
            // Create new node for LHS
            Node* new_node = create_node(rule->lhs);
            
            // Pop L elements (where L = length of RHS)
            // and attach them as children (in reverse order to maintain left-to-right)
            int rhs_len = rule->rhs_len;
            Node** children = NULL;
            
            if (rhs_len > 0) {
                children = (Node**)malloc(rhs_len * sizeof(Node*));
                
                // Pop in reverse order, so we can add children left-to-right
                for (int i = rhs_len - 1; i >= 0; i--) {
                    StackElement elem = pop(stack);
                    children[i] = elem.node;
                }
                
                // Add children to new node
                for (int i = 0; i < rhs_len; i++) {
                    add_child(new_node, children[i]);
                }
                
                free(children);
            }
            
            // GOTO: Look at new top state (after popping RHS elements)
            int prev_state = peek_state(stack);
            unsigned char lhs_symbol = (unsigned char)rule->lhs;
            short goto_state = table->data[prev_state * TABLE_COLS + lhs_symbol];
            
            if (trace) {
                printf("Current state after pop: %d, Looking for GOTO on %c (0x%02x)\n", 
                       prev_state, GET_CHAR(rule->lhs), lhs_symbol);
                printf("Table entry: %d\n", goto_state);
            }
            
            if (goto_state <= 0) {
                fprintf(stderr, "Error: No GOTO for state %d, non-terminal %c (index %d)\n", 
                        prev_state, GET_CHAR(rule->lhs), lhs_symbol);
                fprintf(stderr, "Table value at [%d][%d] = %d\n", 
                        prev_state, lhs_symbol, goto_state);
                free_stack(stack);
                return 0;
            }
            
            if (trace) {
                printf("GOTO: state %d\n", goto_state);
            }
            
            // Push new state with new node
            push(stack, goto_state, new_node);
        }
    }
    
    free_stack(stack);
    return 0;
}
