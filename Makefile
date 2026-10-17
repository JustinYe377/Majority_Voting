CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2

.PHONY: all run clean
all: lab6

lab6: lab6.c

run: lab6
	./lab6

clean:
	$(RM) lab6