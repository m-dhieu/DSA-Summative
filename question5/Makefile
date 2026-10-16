CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic -O2

TARGET = huffman_utility
OBJECTS = main.o huffman_utility.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

main.o: main.c huffman_utility.h
	$(CC) $(CFLAGS) -c main.c -o main.o

huffman_utility.o: huffman_utility.c huffman_utility.h
	$(CC) $(CFLAGS) -c huffman_utility.c -o huffman_utility.o

clean:
	rm -f $(TARGET) $(OBJECTS) device_log.huff device_log_restored.txt
