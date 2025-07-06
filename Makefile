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

# Source files for the library (now includes math3d, lighting, and animation)
LIB_SRCS = $(SRC_DIR)/canvas.c $(SRC_DIR)/math3d.c $(SRC_DIR)/renderer.c $(SRC_DIR)/lighting.c $(SRC_DIR)/animation.c

# Object files for the library
LIB_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(LIB_SRCS))

# Demo source file (Task 1 demo - now serves as the Task 3 animation demo)
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

# Default target - build everything
all: $(DEMO_EXEC) $(TEST_MATH_EXEC) $(LIB_ARCHIVE)

# Rule to build the demo executable (now primarily for animation)
$(DEMO_EXEC): $(DEMO_OBJ) $(LIB_OBJS)
	@mkdir -p $(@D) # Ensure build directory exists
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Demo executable built: $@"

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
	@echo "Task 2 math test ready: $(TEST_EXEC)"

task3: $(DEMO_EXEC) # Alias for the animation demo
	@echo "Task 3 animation demo ready: $(DEMO_EXEC)"

library: $(LIB_ARCHIVE)
	@echo "Static library ready: $(LIB_ARCHIVE)"

# Run targets
run-task1: $(DEMO_EXEC)
	@echo "Running Task 3 Animation demo..."
	mkdir -p frames # Ensure frames directory exists
	./$(DEMO_EXEC)

run-task2: $(TEST_MATH_EXEC)
	@echo "Running Task 2 math test..."
	./$(TEST_MATH_EXEC)

# Target to clean up build files
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR)/*
	rm -rf frames # Also clean up the generated animation frames
	@echo "Clean complete."

# Help target
help:
	@echo "Available targets:"
	@echo "  all          - Build everything (default)"
	@echo "  task1        - Build Task 1 demo (now the animation demo)"
	@echo "  task2        - Build Task 2 math test only"
	@echo "  task3        - Build Task 3 animation demo (alias for task1)"
	@echo "  library      - Build static library only"
	@echo "  run-task1    - Build and run Task 3 animation demo"
	@echo "  run-task2    - Build and run Task 2 math test"
	@echo "  clean        - Remove all build files and animation frames"
	@echo "  help         - Show this help message"

# Phony targets (targets that don't represent actual files)
.PHONY: all task1 task2 task3 library run-task1 run-task2 clean help

# Usage examples:
# make                  # Build everything
# make clean            # Clean build files and frames
# make run-task1        # Build and run the animation demo
# make run-task2        # Build and run the math test
# Animate created .pgm files in frames dir.