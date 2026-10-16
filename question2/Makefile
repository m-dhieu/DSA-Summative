CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror
TARGET = command_registry
SOURCES = main.c registry.c
HEADERS = registry.h

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) commands.tmp

