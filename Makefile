# A template C++ Makefile for your SAT solver.

# Debugging flags
#FLAGS=-Wall -Wold-style-cast -Wformat=2 -ansi -pedantic -ggdb3 -DDEBUG

OBJ_DIR	=	objs
SRC_DIR = 	srcs

# Optimizing flags
FLAGS=-Wall -Wold-style-cast -Wformat=2 -ansi -pedantic -O3

# List all the .o files you need to build here
OBJS = $(OBJ_DIR)/parser.o $(OBJ_DIR)/main.o $(OBJ_DIR)/sat_solver.o

# This is the name of the executable file that gets built.  Please
# don't change it.
EXENAME=yasat

# Compile targets
# modigy -lz flag, because cygwin cannot recognize this flag
# g++ $(FLAGS) $(OBJS) -lz -o $(EXENAME)
all: $(OBJS)
	g++ $(FLAGS) $(OBJS) -o $(EXENAME)
$(OBJ_DIR)/main.o: $(SRC_DIR)/main.cpp
	g++ $(FLAGS) -c $(SRC_DIR)/main.cpp -o $@
$(OBJ_DIR)/sat_solver.o: $(SRC_DIR)/sat_solver.cpp $(SRC_DIR)/sat_solver.h
	g++ $(FLAGS) -c $(SRC_DIR)/sat_solver.cpp -o $@
$(OBJ_DIR)/parser.o: $(SRC_DIR)/parser.cpp $(SRC_DIR)/parser.h
	g++ $(FLAGS) -c $(SRC_DIR)/parser.cpp -o $@

# The "phony" `clean' compilation target.  Type `make clean' to remove
# your object files and your executable.
.PHONY: clean
clean:
	rm -rf $(OBJS) $(EXENAME)