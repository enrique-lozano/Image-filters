CC=g++
CFLAGS=-std=c++17 -Wall -Wextra -Wno-deprecated -Werror -pedantic -pedantic-errors -O3 -DNDEBUG 
PARFLAGS=-fopenmp

img-par: img-par.cpp
	$(CC) $(CFLAGS) $(PARFLAGS) $< -o $@