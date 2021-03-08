CC = g++
CFLAGS = -Wall -O2
SOURCES = $(wildcard *.cc)
OBJECTS = $(SOURCES:%.cc=%.o)

.PHONY: all clean

all: dragonshell

clean:
	rm *.o dragonshell

compile: $(OBJECTS)

%.o: %.cc
	$(CC) $(CFLAGS) -c $^ -o $@
	
compress: dragonshell
	zip -q -r dragonshell.zip *


dragonshell: dragonshell.o
	$(CC) -o dragonshell $(OBJECTS)
