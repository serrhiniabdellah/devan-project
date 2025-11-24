# LR Parser - Generic LR Parsing Engine in C

This is a generic LR parser implementation in C that can parse any grammar given an LR parsing table.

## Project Structure

- `structs.h` - Data structure definitions (Grammar, Rule, Table, Node, Stack)
- `loader.c` - Grammar and parsing table file loader
- `tree.c` - N-ary tree management for parse trees
- `stack.c` - Stack operations for the LR parser
- `engine.c` - Main LR parsing algorithm
- `main.c` - Entry point and command-line interface
- `Makefile` - Build configuration

## Key Features

### MSB Encoding for Symbols
- **Terminals**: ASCII characters (0-127), bit 7 = 0
- **Non-terminals**: ASCII characters with bit 7 = 1 (values 128-255)
- This allows storing both types in a single `char` and indexing a 256-column table

### Grammar File Format

Grammar rules are specified with `:` separator:
```
S:a$Sb
S:
```

Where:
- Left side: Non-terminal (uppercase letter)
- Right side: Production ($ prefix marks non-terminals)
- Empty right side = epsilon production

### Parsing Table Format

Table header shows column symbols:
```
	a	b	$	S
```

Where:
- Lowercase/symbols = terminals
- Uppercase letters = non-terminals
- $ = end-of-input marker

Table rows specify actions per state:
```
0	d2	r2	r2	1
1			a
2	d2	r2	r2	3
```

Where:
- `dN` = Shift to state N
- `rN` = Reduce by rule N
- `a` = Accept
- `N` (number only) = GOTO state N

## Building

```bash
make
```

This creates the `lr_parser` executable.

## Usage

```bash
# Basic usage
./lr_parser <grammar_file> <input_string>

# Interactive mode (prompts for input)
./lr_parser <grammar_file>

# Verbose mode (shows parsing trace)
./lr_parser <grammar_file> <input_string> -v
```

### Examples

```bash
# Parse "ab" with test grammar
./lr_parser test ab

# Parse arithmetic expression with verbose trace
./lr_parser test3 "a+a*a" -v

# Interactive mode
./lr_parser test4
```

## Example Grammars

### test - Balanced parentheses
Grammar: `S -> aSb | ε`

Valid inputs: `""`, `ab`, `aabb`, `aaabbb`

### test2 - Nested parentheses  
Grammar: `T -> (T)T | ε`

Valid inputs: `""`, `()`, `(())`, `()()`

### test3 - Arithmetic expressions
Grammar:
```
E -> E + E
E -> E * E  
E -> ( E )
E -> a
```

Valid inputs: `a`, `a+a`, `a*a`, `a+a*a`, `(a+a)*a`

### test4 - Complex grammar
Grammar:
```
A -> AaB
A -> B
B -> Bbc
B -> c
```

Valid inputs: `c`, `cbc`, `cbcbc`

## Algorithm Overview

The LR parser uses a **stack-based automaton**:

1. **Stack**: Stores (state, tree-node) pairs
2. **Input**: String to parse + $ end marker
3. **Table**: Defines actions per (state, symbol)

### Parsing Actions

- **Shift N**: Push input symbol and goto state N
- **Reduce N**: Apply rule N, pop RHS elements, push LHS with GOTO
- **Accept**: Parsing succeeds, display tree
- **Error**: No valid action, reject input

### Parse Tree Construction

- **Leaf nodes**: Created on Shift (terminals)
- **Internal nodes**: Created on Reduce (non-terminals)
- Children attached left-to-right from RHS symbols

Output format: `S(a()S(b())c())`

## Implementation Details

### Critical Points

1. **MSB encoding**: Non-terminals have bit 7 set (`char | 0x80`)
2. **Rule indexing**: Table uses 1-based rules, array uses 0-based
3. **Empty cells**: Tab-separated format requires careful parsing
4. **Epsilon productions**: RHS length = 0, no stack pops
5. **GOTO transitions**: After reduce, look up non-terminal column

### File Parsing Challenges

- Windows line endings (`\r\n`)
- Empty table cells (consecutive tabs)
- Non-terminal encoding ($ in grammar, uppercase in table header)

## Testing

```bash
# Run all tests
make test

# Manual testing
./lr_parser test ab -v
./lr_parser test2 "(())" -v
./lr_parser test3 "a+a*a" -v
./lr_parser test4 "cbc" -v
```

## Output Examples

### Success (ACCEPT)
```
=== Parsing: ab ===
Parse Tree:
S(a()Sb())

Result: ACCEPT
```

### Failure (REJECT)
```
=== Parsing: aa ===
ERROR: No action for state 3, symbol '$'
REJECT

Result: REJECT
```

### Verbose Trace
```
=== Parsing Trace ===
Input: a+a*a

Flot: a+a*a | Pile: 0 
Action: SHIFT 3
Flot: +a*a | Pile: 0 3 
Action: REDUCE by rule 4: E -> a
...
```

## Memory Management

- Grammar rules: Dynamically allocated array
- Parse table: Flat array [states × 256]
- Parse tree: Recursive node structure
- Stack: Dynamic array with auto-resizing

All memory is properly freed on exit.

## Author

LR Parser implementation for Compilation Project 2025
