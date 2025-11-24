CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99
TARGET = lr_parser
OBJS = main.o loader.o tree.o stack.o engine.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c structs.h
	$(CC) $(CFLAGS) -c main.c

loader.o: loader.c structs.h
	$(CC) $(CFLAGS) -c loader.c

tree.o: tree.c structs.h
	$(CC) $(CFLAGS) -c tree.c

stack.o: stack.c structs.h
	$(CC) $(CFLAGS) -c stack.c

engine.o: engine.c structs.h
	$(CC) $(CFLAGS) -c engine.c

clean:
	rm -f $(OBJS) $(TARGET)

test: $(TARGET)
	@echo "=== Testing with test file ==="
	./$(TARGET) test aa -v
	@echo ""
	@echo "=== Testing with test4 file ==="
	./$(TARGET) test4 aabc -v

.PHONY: all clean test
