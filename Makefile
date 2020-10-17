# proj2.c
# IOS - Projekt 2
# Date: 21. 4. 2020
# Author: David Hurta, FIT
# Compiled: gcc 7.5.0
# Makefile for compiling proj2

CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic
LIBS = -lpthread -lrt -lm

all: proj2

proj2: proj2.o imm.o judge.o
	$(CC) $(CFLAGS) proj2.o imm.o judge.o -o proj2 $(LIBS)

proj2.o: proj2.c
	$(CC) $(CFLAGS) -c proj2.c -o proj2.o

imm.o: imm.c imm.h
	$(CC) $(CFLAGS) -c imm.c -o imm.o

judge.o: judge.c judge.h
	$(CC) $(CFLAGS) -c judge.c -o judge.o

clean:
	rm *.o proj2