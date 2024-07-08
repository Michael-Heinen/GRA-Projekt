CC = gcc
CFLAGS = -Wall -Wextra -std=c99

<<<<<<< HEAD
TARGET_MAIN = main
TARGET_VERSION_1 = main_v1
TARGET_VERSION_2 = main_v2

all: $(TARGET_MAIN) $(TARGET_VERSION_1) $(TARGET_VERSION_2)
=======
# Source and include directories
SRC_DIR = src
INCLUDE_DIR = include

# Source files
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/matrix_io.c $(SRC_DIR)/matr_mult.c $(SRC_DIR)/matr_mult_V1.c $(SRC_DIR)/matr_mult_V2.c

# Compiler and flags
CC := $(shell command -v gcc || command -v clang)
CFLAGS := -Wall -Wextra -Wpedantic -std=c99 -I$(INCLUDE_DIR)

# Target executable
TARGET = main
>>>>>>> b2b9891 (modular project structure implementation)

$(TARGET_MAIN): main.c
	$(CC) $(CFLAGS) -o $(TARGET_MAIN) main.c

<<<<<<< HEAD
$(TARGET_VERSION_1): main.c
	$(CC) $(CFLAGS) -DVERSION=1 -o $(TARGET_VERSION_1) main.c

$(TARGET_VERSION_2): main.c
	$(CC) $(CFLAGS) -DVERSION=2 -o $(TARGET_VERSION_2) main.c

clean:
	rm -f $(TARGET_MAIN) $(TARGET_VERSION_1) $(TARGET_VERSION_2)

run: $(TARGET_MAIN)
	./$(TARGET_MAIN) -a input_a.txt -b input_b.txt -o output.txt

help:
	./$(TARGET_MAIN) --help
=======
# Check if compiler is available
ifeq ($(strip $(CC)),)
    $(error Weder clang noch gcc ist verfügbar. Beenden.)
endif

# Default target
all: release

# Debug build
debug: CFLAGS += -g
debug: $(TARGET)

# Release build
release: CFLAGS += -O2
release: $(TARGET)

# Recipe to build the target
$(TARGET):
	$(CC) $(CFLAGS) -o $@ $(SOURCES)

# Clean up
clean:
	rm -f $(TARGET)

.PHONY: all debug release clean
>>>>>>> b2b9891 (modular project structure implementation)
