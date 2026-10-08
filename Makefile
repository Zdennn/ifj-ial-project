CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -Isrc -MMD -MP

SRC = $(wildcard src/*.c)
OBJS = $(SRC:.c=.o)
DEPS = $(SRC:.c=.d)

TARGET = compiler

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(TARGET) src/*.o src/*.d

.PHONY: all clean