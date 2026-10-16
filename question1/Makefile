CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror
TARGET = event_logger
SOURCES = main.c logger.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = logger.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

main.o: main.c $(HEADERS)
	$(CC) $(CFLAGS) -c main.c -o main.o

logger.o: logger.c $(HEADERS)
	$(CC) $(CFLAGS) -c logger.c -o logger.o

clean:
	rm -f $(OBJECTS) $(TARGET)

