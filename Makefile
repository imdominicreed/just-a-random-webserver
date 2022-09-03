CXX		  := g++
CXX_FLAGS := -Wall -Wextra -std=c++17 -ggdb

BIN		:= bin
SRC		:= src/*.cc
INCLUDE	:= -Iinclude
EXECUTABLE	:= main

# compile an executable using all code in include/ and src/
all:
	$(CXX) $(SRC) $(CXX_FLAGS) $(INCLUDE) $^ -o ./$(BIN)/$(EXECUTABLE)

# remove the executable, compile and run it, clearing the
# terminal before running
run: clean all
	clear
	./$(BIN)/$(EXECUTABLE)

# remove the executable in the binary directory
clean:
	-rm $(BIN)/$(EXECUTABLE)
