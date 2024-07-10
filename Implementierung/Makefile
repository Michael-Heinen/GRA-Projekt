CC = gcc
CFLAGS = -Wall -Wextra -std=c99

TARGET_MAIN = main
TARGET_VERSION_1 = main_v1
TARGET_VERSION_2 = main_v2

all: $(TARGET_MAIN) $(TARGET_VERSION_1) $(TARGET_VERSION_2)

$(TARGET_MAIN): main.c
	$(CC) $(CFLAGS) -o $(TARGET_MAIN) main.c

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
