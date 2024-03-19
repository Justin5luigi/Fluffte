CC = g++
COMPILE_FLAGS = -g -pedantic-errors -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion
SRC_FILES = fluffyte.cpp

LINKERS = -lSDL2 -lSDL2_ttf
OBJ_NAME = fluffte


build:
	$(CC) $(COMPILE_FLAGS) $(SRC_FILES) $(LINKERS) -o $(OBJ_NAME)

run:
	./$(OBJ_NAME)

debug:
	gdb $(OBJ_NAME)

clean:
	rm $(OBJNAME)
