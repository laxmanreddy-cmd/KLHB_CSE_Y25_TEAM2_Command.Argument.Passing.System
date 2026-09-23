CC = gcc
CFLAGS = -Wall -Wextra -std=c11

.PHONY: all clean

all: arg_executor arg_printer

arg_executor: arg_executor.c
	$(CC) $(CFLAGS) -o $@ $<

arg_printer: arg_printer.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f arg_executor arg_printer
