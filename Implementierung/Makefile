CFLAGS =
CC = gcc

all: program

program: main.o
	$(CC) $(CFLAGS) main.o -o program

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm main.o program

