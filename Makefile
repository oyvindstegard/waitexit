# Simple Makefile for GNU/Linux

CC = gcc
CFLAGS = -O2 -Wall -Wno-unused-result

waitexit: waitexit.c
	$(CC) -o $@ $< $(CFLAGS)

# Clean up
.PHONY: clean
clean:
	rm -f waitexit
