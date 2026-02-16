CC = gcc
CFLAGS = -Wall -Wextra -O2 -D_GNU_SOURCE -std=gnu99
LDFLAGS = 

TARGET = file_server
OBJS = main.o file_handler.o signal_handler.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

main.o: main.c file_handler.h signal_handler.h
	$(CC) $(CFLAGS) -c main.c

file_handler.o: file_handler.c file_handler.h
	$(CC) $(CFLAGS) -c file_handler.c

signal_handler.o: signal_handler.c signal_handler.h
	$(CC) $(CFLAGS) -c signal_handler.c

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean