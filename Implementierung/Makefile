CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -O2 -I$(INC_DIR) -mavx

BUILD_DIR = obj
SRC_DIR = src
INC_DIR = include

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

TARGET = main


all: $(TARGET)

#Bulid executable -> all object files
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

run: $(TARGET)
	./$(TARGET) -a files/test_matrices/matrixA_4x4_D0.2.txt -b files/test_matrices/matrixB_4x4_D0.2.txt -o files/results/result_4x4_D0.2.txt

help:
	./$(TARGET) --help

valgrind: CFLAGS += -g
valgrind: $(TARGET)
	valgrind --leak-check=full ./$(TARGET) -a files/test_matrices/matrixA_4x4_D0.2.txt -b files/test_matrices/matrixB_4x4_D0.2.txt -o files/results/result_4x4_D0.2.txt

.PHONY: all clean run help valgrind
