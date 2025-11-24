#!/bin/bash

# Test script for LR Parser

echo "========================================="
echo "LR Parser Test Suite"
echo "========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

passed=0
failed=0

# Function to run a test
run_test() {
    local grammar=$1
    local input=$2
    local expected=$3  # "accept" or "reject"
    
    echo -n "Testing $grammar with '$input': "
    
    output=$(./lr_parser "$grammar" "$input" 2>&1)
    
    if echo "$output" | grep -q "Result: ACCEPT"; then
        result="accept"
    else
        result="reject"
    fi
    
    if [ "$result" = "$expected" ]; then
        echo -e "${GREEN}PASS${NC}"
        ((passed++))
    else
        echo -e "${RED}FAIL${NC} (expected $expected, got $result)"
        ((failed++))
    fi
}

# Test grammar: S -> aSb | ε
echo "--- Test 1: Balanced a's and b's ---"
run_test "test" "" "accept"
run_test "test" "ab" "accept"
run_test "test" "aabb" "accept"
run_test "test" "aaabbb" "accept"
run_test "test" "aa" "reject"
run_test "test" "abb" "reject"
echo ""

# Test grammar: T -> (T)T | ε
echo "--- Test 2: Nested parentheses ---"
run_test "test2" "" "accept"
run_test "test2" "()" "accept"
run_test "test2" "(())" "accept"
run_test "test2" "()()" "accept"
run_test "test2" "((()))" "accept"
run_test "test2" "(" "reject"
run_test "test2" ")(" "reject"
echo ""

# Test grammar: E -> E+E | E*E | (E) | a
echo "--- Test 3: Arithmetic expressions ---"
run_test "test3" "a" "accept"
run_test "test3" "a+a" "accept"
run_test "test3" "a*a" "accept"
run_test "test3" "a+a*a" "accept"
run_test "test3" "(a+a)*a" "accept"
run_test "test3" "(a)" "accept"
run_test "test3" "+" "reject"
run_test "test3" "a+" "reject"
echo ""

# Test grammar: A -> AaB | B, B -> Bbc | c
echo "--- Test 4: Complex grammar ---"
run_test "test4" "c" "accept"
run_test "test4" "cbc" "accept"
run_test "test4" "cbcbc" "accept"
run_test "test4" "cacbc" "accept"
run_test "test4" "ab" "reject"
run_test "test4" "abc" "reject"
run_test "test4" "cabc" "reject"
echo ""

echo "========================================="
echo "Results: ${GREEN}$passed passed${NC}, ${RED}$failed failed${NC}"
echo "========================================="

if [ $failed -eq 0 ]; then
    exit 0
else
    exit 1
fi
