.PHONY: build clean

# Compiler settings
CC := gcc
CFLAGS := -Wall -Wextra $(OPTFLAGS)
LDFLAGS := $(OPTFLAGS)

# Optimization level, can be overridden by passing OPTFLAGS=-O2 etc. to make
OPTFLAGS := -O0

# Source and Object Directories
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

# Target binary name
TARGET := benchmark

# Automatically find source files & deduce object file names
SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Default rule
all: build

# Rule to create the bin and obj directories
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# Rule to compile: create object files from source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to link: create final binary from object files
$(BIN_DIR)/$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ -o $@

build: $(BIN_DIR)/$(TARGET)

clean:
	$(RM) -r $(OBJ_DIR) $(BIN_DIR)

