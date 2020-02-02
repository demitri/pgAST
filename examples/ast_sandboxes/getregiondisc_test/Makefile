
CC = gcc
AST = /usr/local/ast

INCLUDES = -I$(AST)/include
CFLAGS = -g -Wall $(INCLUDES)
LDFLAGS = -L/$(AST)/lib

all: getregiondisc_test

getregiondisc_test: getregiondisc_test.o
	$(CC) getregiondisc_test.o $(LDFLAGS) -last -o getregiondisc_test
	
getregiondisc_test.o: getregiondisc_test.c
	$(CC) $(CFLAGS) -c getregiondisc_test.c

clean:
	rm *.o getregiondisc_test
	