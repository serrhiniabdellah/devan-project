#include "structs.h"

// Function prototypes
int load_grammar_table(const char* filename, Grammar* grammar, Table* table);
void print_grammar(Grammar* grammar);
void print_table(Table* table);
int parse(Grammar* grammar, Table* table, const char* input, int trace);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <grammar_file> [input_string] [-v]\n", argv[0]);
        fprintf(stderr, "  -v : Enable verbose trace output\n");
        fprintf(stderr, "If no input_string is provided, it will be read from stdin\n");
        return 1;
    }
    
    char* filename = argv[1];
    char* input_string = NULL;
    int trace = 0;
    
    // Parse command line arguments
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            trace = 1;
        } else if (input_string == NULL) {
            input_string = argv[i];
        }
    }
    
    // Load grammar and table
    Grammar grammar;
    Table table;
    
    printf("Loading grammar from: %s\n", filename);
    if (!load_grammar_table(filename, &grammar, &table)) {
        fprintf(stderr, "Failed to load grammar and table\n");
        return 1;
    }
    
    printf("\n=== Grammar ===\n");
    print_grammar(&grammar);
    
    if (trace) {
        printf("\n=== Table Preview ===\n");
        print_table(&table);
    }
    
    // Get input string if not provided
    char input_buffer[1024];
    if (input_string == NULL) {
        printf("\nEnter input string: ");
        if (fgets(input_buffer, sizeof(input_buffer), stdin)) {
            // Remove newline
            input_buffer[strcspn(input_buffer, "\n")] = 0;
            input_string = input_buffer;
        } else {
            fprintf(stderr, "Failed to read input\n");
            return 1;
        }
    }
    
    printf("\n=== Parsing: %s ===\n", input_string);
    
    // Parse the input
    int result = parse(&grammar, &table, input_string, trace);
    
    if (result) {
        printf("\nResult: ACCEPT\n");
    } else {
        printf("\nResult: REJECT\n");
    }
    
    // Cleanup
    if (grammar.rules) {
        free(grammar.rules);
    }
    if (table.data) {
        free(table.data);
    }
    
    return result ? 0 : 1;
}
