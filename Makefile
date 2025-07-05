# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -I./include # -g for debugging, -I./include to find headers in include/
LDFLAGS = -lm # Link math library for functions like sin, cos, sqrt, fabs

# Directories
SRC_DIR = src
INCLUDE_DIR = include
DEMO_DIR = demo
TESTS_DIR = tests
BUILD_DIR = build

# Source files for the library (now includes math3d)
LIB_SRCS = $(SRC_DIR)/canvas.c $(SRC_DIR)/math3d.c $(SRC_DIR)/renderer.c
# Object files for the library
LIB_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(LIB_SRCS))

# Demo source file (Task 1 demo)
DEMO_SRC = $(DEMO_DIR)/main2.c
# Demo object file
DEMO_OBJ = $(patsubst $(DEMO_DIR)/%.c,$(BUILD_DIR)/%.o,$(DEMO_SRC))
# Demo executable name
DEMO_EXEC = $(BUILD_DIR)/stage3_demo

# Test source files
TEST_MATH_SRC = $(TESTS_DIR)/test_math.c
TEST_MATH_OBJ = $(patsubst $(TESTS_DIR)/%.c,$(BUILD_DIR)/%.o,$(TEST_MATH_SRC))
TEST_MATH_EXEC = $(BUILD_DIR)/test_math

# Library archive (for later tasks)
LIB_ARCHIVE = $(BUILD_DIR)/libtiny3d.a

# Default target - build both demos
all: $(DEMO_EXEC) $(TEST_MATH_EXEC) $(LIB_ARCHIVE)

# Rule to build the Task 1 demo executable
$(DEMO_EXEC): $(DEMO_OBJ) $(LIB_OBJS)
	@mkdir -p $(@D) # Ensure build directory exists
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Task 1 demo executable built: $@"

# Rule to build the Task 2 test executable
$(TEST_MATH_EXEC): $(TEST_MATH_OBJ) $(LIB_OBJS)
	@mkdir -p $(@D) # Ensure build directory exists
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Task 2 math test executable built: $@"

# Rule to build the static library
$(LIB_ARCHIVE): $(LIB_OBJS)
	@mkdir -p $(@D) # Ensure build directory exists
	ar rcs $@ $^
	@echo "Static library built: $@"

# Rule to compile library source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D) # Ensure build directory exists
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled library object: $@"

# Rule to compile demo source file into an object file
$(BUILD_DIR)/%.o: $(DEMO_DIR)/%.c
	@mkdir -p $(@D) # Ensure build directory exists
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled demo object: $@"

# Rule to compile test source files into object files
$(BUILD_DIR)/%.o: $(TESTS_DIR)/%.c
	@mkdir -p $(@D) # Ensure build directory exists
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled test object: $@"

# Individual build targets
task1: $(DEMO_EXEC)
	@echo "Task 1 demo ready: $(DEMO_EXEC)"

task2: $(TEST_MATH_EXEC)
	@echo "Task 2 math test ready: $(TEST_MATH_EXEC)"

task3: $(DEMO_EXEC) # New target for Task 3
    @echo "Task 3 demo ready: $(DEMO_EXEC)"

library: $(LIB_ARCHIVE)
	@echo "Static library ready: $(LIB_ARCHIVE)"

# Run targets
run-task1: $(DEMO_EXEC)
	@echo "Running Task 3 demo..."
	mkdir -p frames #
	./$(DEMO_EXEC)

run-task2: $(TEST_MATH_EXEC)
	@echo "Running Task 2 math test..."
	./$(TEST_MATH_EXEC)

# Target to clean up build files
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR)/*
	@echo "Clean complete."

# Help target
help:
	@echo "Available targets:"
	@echo "  all        - Build everything (default)"
	@echo "  task1      - Build Task 1 demo only"
	@echo "  task2      - Build Task 2 math test only"
	@echo "  library    - Build static library only"
	@echo "  run-task1  - Build and run Task 1 demo"
	@echo "  run-task2  - Build and run Task 2 math test"
	@echo "  clean      - Remove all build files"
	@echo "  help       - Show this help message"

# Phony targets (targets that don't represent actual files)
.PHONY: all task1 task2 library run-task1 run-task2 clean help

# Usage examples for Task 2:
# make              	# Build everything
# make clean        	# Clean build files
# mkdir frames      	# Create a directory named frames in the current directory.
# ./build/test_math 	# Run the Task 2
# Animate created 60 .pgm files in frames dir.
