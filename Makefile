# Makefile
#
# Build the example search
#
# Derrick Stolee - Oct 28 2009
#

CC=gcc
CXX=g++
OFLAGS=-O3
LFLAGS=-g -lm 
CFLAGS=-g -c -Wall -Werror
CXXFLAGS=-g -c -Wall -Werror
OBJECTS=SearchManager.o
TARGETS=example.exe

.SUFFIXES: .c .cpp .o .obj .exe 

all: $(OBJECTS) $(TARGETS)

# The default object compiler
.c.o: $<
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o: $<
	$(CXX) $(CXXFLAGS) -c $< -o $@

.cpp.exe: $< $(OBJECTS) 
	$(CXX) $(LFLAGS)						\
	$(OBJECTS)				    	   			\
	$< -o $@

.c.exe: $< $(COBJECTS)
	$(CC) 	$(LFLAGS)	$(DEBUG)		    						\			
	$< -o $@

clean:
	-@rm $(OBJECTS) $(TARGETS) $(TESTS)
	
cleanexe:
	-@rm $(TARGETS)

cleantest:
	-@rm $(TESTS)

clest:
	-@rm $(TESTS)

clexe:
	-@rm $(TARGETS)
