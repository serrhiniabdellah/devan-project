#include "structs.h"

// Read and parse the grammar and table from file
int read_file(const char* filename, Grammar* grammar, Table* table) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return 0;
    }

    char line[1024];
    grammar->rules = NULL;
    grammar->num_rules = 0;
    int rules_capacity = 10;
    grammar->rules = (Rule*)malloc(rules_capacity * sizeof(Rule));
    
    int reading_grammar = 1;
    int first_rule = 1;
    
    // Read grammar rules
    while (fgets(line, sizeof(line), fp)) {
        // Skip empty lines
        if (line[0] == '\n' || line[0] == '\r') continue;
        
        // Check if we've reached the table header (line with just terminals/non-terminals)
        if (line[0] == '\t' || (line[0] != ' ' && strchr(line, '\t') && !strchr(line, ':'))) {
            reading_grammar = 0;
            break;
        }
        
        if (reading_grammar) {
            // Parse grammar rule: LHS:RHS
            char* colon = strchr(line, ':');
            if (!colon) continue;
            
            // Get LHS (non-terminal)
            char lhs_char = line[0];
            char lhs = MAKE_NONTERMINAL(lhs_char);
            
            // Set axiom from first rule
            if (first_rule) {
                grammar->axiom = lhs;
                first_rule = 0;
            }
            
            // Get RHS (production)
            char* rhs_start = colon + 1;
            int rhs_idx = 0;
            char rhs[MAX_PRODUCTION_LEN] = {0};
            
            for (char* p = rhs_start; *p && *p != '\n' && *p != '\r'; p++) {
                if (*p == '$') {
                    // Next character is a non-terminal
                    p++;
                    if (*p && *p != '\n' && *p != '\r') {
                        rhs[rhs_idx++] = MAKE_NONTERMINAL(*p);
                    }
                } else if (*p != ' ' && *p != '\t') {
                    // Terminal
                    rhs[rhs_idx++] = *p;
                }
            }
            
            // Expand rules array if needed
            if (grammar->num_rules >= rules_capacity) {
                rules_capacity *= 2;
                grammar->rules = (Rule*)realloc(grammar->rules, rules_capacity * sizeof(Rule));
            }
            
            // Add rule
            grammar->rules[grammar->num_rules].lhs = lhs;
            strcpy(grammar->rules[grammar->num_rules].rhs, rhs);
            grammar->rules[grammar->num_rules].rhs_len = rhs_idx;
            grammar->num_rules++;
        }
    }
    
    // Read table header (skip it, we know columns are indexed by character value)
    if (!reading_grammar) {
        // Already read the header line above
    } else {
        fgets(line, sizeof(line), fp);  // Read header
    }
    
    // Count states first
    long pos = ftell(fp);
    int num_states = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] >= '0' && line[0] <= '9') {
            num_states++;
        }
    }
    fseek(fp, pos, SEEK_SET);
    
    // Allocate table
    table->num_states = num_states;
    table->data = (short*)calloc(num_states * TABLE_COLS, sizeof(short));
    
    // Read table rows
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] < '0' || line[0] > '9') continue;
        
        int state = atoi(line);
        char* token = strtok(line, "\t");
        token = strtok(NULL, "\t");  // Skip state number
        
        int col_idx = 0;
        char headers[TABLE_COLS][10];
        
        // Re-read header to get column order
        long current_pos = ftell(fp);
        fseek(fp, pos - strlen(line) - 100, SEEK_SET);
        char header_line[1024];
        int found_header = 0;
        while (fgets(header_line, sizeof(header_line), fp) && ftell(fp) < pos) {
            if (header_line[0] == '\t' || (header_line[0] != ' ' && strchr(header_line, '\t'))) {
                found_header = 1;
                break;
            }
        }
        
        // Parse headers
        int num_cols = 0;
        char* htoken = strtok(header_line, "\t\n\r");
        while (htoken) {
            strcpy(headers[num_cols++], htoken);
            htoken = strtok(NULL, "\t\n\r");
        }
        
        fseek(fp, current_pos - strlen(line) - 1, SEEK_SET);
        fgets(line, sizeof(line), fp);
        
        // Parse state line
        token = strtok(line, "\t");
        token = strtok(NULL, "\t");
        col_idx = 0;
        
        while (token && col_idx < num_cols) {
            if (token[0] != '\n' && token[0] != '\r' && strlen(token) > 0) {
                // Determine symbol for this column
                char symbol;
                if (headers[col_idx][0] == '$') {
                    symbol = MAKE_NONTERMINAL(headers[col_idx][1]);
                } else {
                    symbol = headers[col_idx][0];
                }
                
                // Parse action
                short action = 0;
                if (token[0] == 'd') {
                    // Shift
                    action = (short)atoi(token + 1);
                } else if (token[0] == 'r') {
                    // Reduce
                    action = -(short)atoi(token + 1);
                } else if (token[0] == 'a') {
                    // Accept
                    action = -127;
                } else if (token[0] >= '0' && token[0] <= '9') {
                    // GOTO (just state number)
                    action = (short)atoi(token);
                }
                
                table->data[state * TABLE_COLS + (unsigned char)symbol] = action;
            }
            token = strtok(NULL, "\t");
            col_idx++;
        }
    }
    
    fclose(fp);
    return 1;
}

// Simpler version - parse the exact format from the test files
int load_grammar_table(const char* filename, Grammar* grammar, Table* table) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return 0;
    }

    char line[1024];
    grammar->rules = (Rule*)malloc(MAX_RULES * sizeof(Rule));
    grammar->num_rules = 0;
    int first_rule = 1;
    
    // Phase 1: Read grammar rules
    while (fgets(line, sizeof(line), fp)) {
        // Empty line or start of table
        if (line[0] == '\t' || line[0] == '\n' || line[0] == '\r') break;
        if (line[0] < 'A' || line[0] > 'Z') break;
        
        char* colon = strchr(line, ':');
        if (!colon) break;
        
        // Parse LHS
        char lhs = MAKE_NONTERMINAL(line[0]);
        if (first_rule) {
            grammar->axiom = lhs;
            first_rule = 0;
        }
        
        // Parse RHS
        char rhs[MAX_PRODUCTION_LEN] = {0};
        int rhs_len = 0;
        char* p = colon + 1;
        
        while (*p && *p != '\n' && *p != '\r') {
            if (*p == '$') {
                p++;
                if (*p && *p != '\n' && *p != '\r' && *p != '\t' && *p != ' ') {
                    rhs[rhs_len++] = MAKE_NONTERMINAL(*p);
                }
            } else if (*p != ' ' && *p != '\t') {
                rhs[rhs_len++] = *p;
            }
            p++;
        }
        
        // rhs_len can be 0 for epsilon production
        grammar->rules[grammar->num_rules].lhs = lhs;
        if (rhs_len > 0) {
            memcpy(grammar->rules[grammar->num_rules].rhs, rhs, rhs_len);
        }
        grammar->rules[grammar->num_rules].rhs[rhs_len] = '\0';
        grammar->rules[grammar->num_rules].rhs_len = rhs_len;
        grammar->num_rules++;
    }
    
    // Phase 2: Read table header (line should already be in buffer)
    char symbols[TABLE_COLS];
    int num_symbols = 0;
    
    // Parse header line (currently in line buffer)
    char* token = strtok(line, "\t\n\r");
    while (token && num_symbols < TABLE_COLS) {
        // Skip empty tokens
        while (token && (strlen(token) == 0 || (strlen(token) == 1 && token[0] == ' '))) {
            token = strtok(NULL, "\t\n\r");
        }
        if (!token) break;
        
        // In the table header:
        // - Uppercase single letters (A-Z) are non-terminals
        // - Everything else (including $, a, b, c, etc.) are terminals
        if (strlen(token) == 1 && token[0] >= 'A' && token[0] <= 'Z') {
            symbols[num_symbols++] = MAKE_NONTERMINAL(token[0]);
        } else if (strlen(token) >= 1) {
            // Terminal - take the first character
            symbols[num_symbols++] = token[0];
        }
        token = strtok(NULL, "\t\n\r");
    }
    
    // Phase 3: Count states
    long header_pos = ftell(fp);
    int max_state = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] >= '0' && line[0] <= '9') {
            int state = atoi(line);
            if (state > max_state) max_state = state;
        }
    }
    
    // Allocate table
    table->num_states = max_state + 1;
    table->data = (short*)calloc(table->num_states * TABLE_COLS, sizeof(short));
    
    // Re-read table rows
    fseek(fp, header_pos, SEEK_SET);
    
    while (fgets(line, sizeof(line), fp)) {
        // Remove CR if present (Windows line endings)
        char* cr = strchr(line, '\r');
        if (cr) *cr = '\0';
        char* nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        
        if (line[0] < '0' || line[0] > '9') continue;
        
        // Parse state number
        int state = atoi(line);
        
        // Find first tab
        char* p = strchr(line, '\t');
        if (!p) continue;
        p++;  // Move past the tab
        
        // Parse each column manually to handle empty cells
        for (int col = 0; col < num_symbols; col++) {
            // Find the extent of this cell (until next tab or end)
            char* tab_pos = strchr(p, '\t');
            char cell[100] = {0};
            
            if (tab_pos) {
                int len = tab_pos - p;
                if (len > 0 && len < 99) {
                    strncpy(cell, p, len);
                    cell[len] = '\0';
                }
                p = tab_pos + 1;  // Move to next cell
            } else {
                // Last cell in row
                strncpy(cell, p, 99);
                cell[99] = '\0';
            }
            
            // Trim whitespace
            int start = 0;
            while (cell[start] == ' ') start++;
            int end = strlen(cell) - 1;
            while (end >= start && (cell[end] == ' ' || cell[end] == '\r' || cell[end] == '\n')) {
                cell[end--] = '\0';
            }
            
            // Parse action if cell is not empty
            if (start <= end && cell[start] != '\0') {
                short action = 0;
                
                if (cell[start] == 'd') {
                    // Shift
                    action = (short)atoi(cell + start + 1);
                } else if (cell[start] == 'r') {
                    // Reduce
                    action = -(short)atoi(cell + start + 1);
                } else if (cell[start] == 'a') {
                    // Accept
                    action = -127;
                } else if (cell[start] >= '0' && cell[start] <= '9') {
                    // GOTO state
                    action = (short)atoi(cell + start);
                }
                
                if (action != 0) {
                    table->data[state * TABLE_COLS + (unsigned char)symbols[col]] = action;
                }
            }
            
            if (!tab_pos) break;  // No more cells
        }
    }
    
    fclose(fp);
    return 1;
}

// Print grammar for debugging
void print_grammar(Grammar* grammar) {
    printf("Axiom: %c\n", GET_CHAR(grammar->axiom));
    printf("Rules:\n");
    for (int i = 0; i < grammar->num_rules; i++) {
        printf("  %d: %c ->", i + 1, GET_CHAR(grammar->rules[i].lhs));
        for (int j = 0; j < grammar->rules[i].rhs_len; j++) {
            char c = grammar->rules[i].rhs[j];
            if (IS_NONTERMINAL(c)) {
                printf(" $%c", GET_CHAR(c));
            } else {
                printf(" %c", c);
            }
        }
        printf("\n");
    }
}

// Print table for debugging
void print_table(Table* table) {
    printf("Table (%d states):\n", table->num_states);
    // Print a subset for debugging
    for (int i = 0; i < table->num_states && i < 10; i++) {
        printf("State %d: ", i);
        int count = 0;
        for (int j = 0; j < TABLE_COLS && count < 10; j++) {
            short action = table->data[i * TABLE_COLS + j];
            if (action != 0) {
                printf("[%c:", j);
                if (action > 0) printf("s%d", action);
                else if (action == -127) printf("acc");
                else printf("r%d", -action);
                printf("] ");
                count++;
            }
        }
        printf("\n");
    }
}
