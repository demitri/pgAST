
CC = gcc
AST = /usr/local/ast

INCLUDES = -I$(AST)/include
CFLAGS = -g -Wall $(INCLUDES)
LDFLAGS = -L/$(AST)/lib

all: wcs_test

wcs_test: wcs_test.o
	$(CC) wcs_test.o $(LDFLAGS) -last -o wcs_test
	
wcs_test.o: wcs_test.c
	$(CC) $(CFLAGS) -c wcs_test.c

clean:
	rm *.o wcs_test
	