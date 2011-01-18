# Makefile
#
# Build the example search
#
# Derrick Stolee - Oct 28 2009
#

CC=gcc
CXX=g++
OFLAGS=-O7
# -O0 is for valgrind
LFLAGS=-g -lm 
CFLAGS=-g -c -Wall
CXXFLAGS=-g -c -Wall
OBJECTS=treesearch.o
TARGET=example
TARGET2=example2



all: $(TARGET) $(TARGET2)

treesearch.o: treesearch.h treesearch.cpp
	$(CXX) $(OFLAGS) $(CXXFLAGS) -o treesearch.o treesearch.cpp
	
SearchManager.o: SearchManager.hpp SearchManager.cpp
	$(CXX) $(OFLAGS) $(CXXFLAGS) -o SearchManager.o SearchManager.cpp
	
$(TARGET).o: $(TARGET).cpp treesearch.h 
	$(CXX) $(OFLAGS) $(CXXFLAGS) -o $(TARGET).o $(TARGET).cpp
		
$(TARGET): $(OBJECTS) $(TARGET).o
	$(CXX) $(OFLAGS) $(LFLAGS) -o $(TARGET) $(TARGET).o $(OBJECTS)
	
$(TARGET2): SearchManager.o $(TARGET2).o
	$(CXX) $(OFLAGS) $(LFLAGS) -o $(TARGET2) $(TARGET2).o SearchManager.o
	
clean: 
	rm $(TARGET) $(TARGET).o $(OBJECTS) $(TARGET2) $(TARGET2).o SearchManager.o
	
rebuild: clean all
