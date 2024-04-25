CC = gcc
CFLAGS = -g -Wall

CXX = g++
CPPFLAGS = -g -Wall

LDFLAGS= -lpq

EXEC = test

OBJS= example_noblock.o

INC_DIRS= -I /usr/include

$(EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
example_noblock.o: example_noblock.cpp
	$(CXX) $(CPPFLAGS) $(INC_DIRS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJS) $(EXEC)
