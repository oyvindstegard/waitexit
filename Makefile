# Simple Makefile for GNU/Linux

CC = gcc
CFLAGS = -O2 -Wall -Wno-unused-result

waitexit: waitexit.c
	$(CC) -o $@ $< $(CFLAGS)

tags:
	etags *.[ch]

.PHONY: clean tags
clean:
	rm -f waitexit
