# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -I./include # -g for debugging, -I./include to find headers in include/
LDFLAGS = -lm # Link math library for functions like sin, cos, sqrt, fabs

# Directories
SRC_DIR = src
INCLUDE_DIR = include
DEMO_DIR = demo
BUILD_DIR = build

# Source files for the library
LIB_SRCS = $(SRC_DIR)/canvas.c
# Object files for the library
LIB_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(LIB_SRCS))

# Demo source file
DEMO_SRC = $(DEMO_DIR)/main.c
# Demo object file
DEMO_OBJ = $(patsubst $(DEMO_DIR)/%.c,$(BUILD_DIR)/%.o,$(DEMO_SRC))
# Demo executable name
DEMO_EXEC = $(BUILD_DIR)/stage1_demo

# Default target
all: $(DEMO_EXEC)

# Rule to build the demo executable
$(DEMO_EXEC): $(DEMO_OBJ) $(LIB_OBJS)
	@mkdir -p $(@D) # Ensure build directory exists
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Demo executable built: $@"

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

# Target to clean up build files
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR)/*
	@echo "Clean complete."

# Phony targets (targets that don't represent actual files)
.PHONY: all clean

# To build, navigate to the libtiny3d directory in your terminal and run:
# make
#
# To run the demo after building:
# ./build/stage1_demo
#
# To clean up:
# make clean
