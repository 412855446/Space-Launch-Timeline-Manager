\
        CC = gcc
        CFLAGS = -std=c11 -Wall -Wextra -O0 -g
        TARGET = sltm
        SRCS = main.c
        OBJS = $(SRCS:.c=.o)

        all: $(TARGET)

        $(TARGET): $(OBJS)
        	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

        %.o: %.c event.h
        	$(CC) $(CFLAGS) -c $< -o $@

        clean:
        	rm -f $(OBJS) $(TARGET)

        .PHONY: all clean
