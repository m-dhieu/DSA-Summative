CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic -g
TARGET = net_analyser
OBJECTS = main.o net_analyser.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

main.o: main.c net_analyser.h
	$(CC) $(CFLAGS) -c main.c -o main.o

net_analyser.o: net_analyser.c net_analyser.h
	$(CC) $(CFLAGS) -c net_analyser.c -o net_analyser.o

clean:
	rm -f $(OBJECTS) $(TARGET) net_analyser_san
