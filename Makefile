CXX		  := g++
CXX_FLAGS := -Wall -Wextra -std=c++17 -ggdb -l sqlite3

BIN		:= bin
SRC		:= src/*.cc
TESTING := testing/*.cc
INCLUDE	:= -Iinclude
EXECUTABLE	:= main
DB_TEST := interactive_db

# compile an executable using all code in include/ and src/
all:
	$(CXX) $(SRC) $(CXX_FLAGS) $(INCLUDE) $^ -o ./$(BIN)/$(EXECUTABLE)

test: 
	$(CXX) $(TESTING) $(CXX_FLAGS) $(INCLUDE) $^ -o ./$(BIN)/$(DB_TEST)

run_test: clean clean_db test
	./$(BIN)/$(DB_TEST) < testing/commands.in > testing/commands.out

# remove the executable, compile and run it, clearing the
# terminal before running
run: clean all
	clear
	./$(BIN)/$(EXECUTABLE)

# remove the executable in the binary directory
clean:
	-rm $(BIN)/$(EXECUTABLE)
	-rm $(BIN)/${DB_TEST}

clean_db:
	-rm ./$(BIN)/*.db
