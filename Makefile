# Makefile for Dungeon Adventure Game

CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = dungeon_game
SRC = main.c
OBJ = $(SRC:.c=.o)

# Default target
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile the source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the build files
clean:
	rm -f $(OBJ) $(TARGET)

# Phony targets
.PHONY: all clean