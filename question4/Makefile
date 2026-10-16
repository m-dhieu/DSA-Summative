CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic
TARGET = routing_program
OBJECTS = main.o routing.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

main.o: main.c routing.h
	$(CC) $(CFLAGS) -c main.c -o main.o

routing.o: routing.c routing.h
	$(CC) $(CFLAGS) -c routing.c -o routing.o

clean:
	rm -f $(OBJECTS) $(TARGET)
