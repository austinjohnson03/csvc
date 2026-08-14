CC 				= gcc
CFLAGS 		= -std=c11 -Wall -Wextra -Wpedantic -Wsign-compare -Wshadow -g
CPPFLAGS 	= -Iinclude

LIB 			= ./build/libcsv.a
TARGET		= ./build/csv-test

LIB_SRC		= src/csv.c
LIB_OBJ		=	./build/csv.o

APP_SRC		= ./test/main.c
APP_OBJ		= ./build/main.o

.PHONY: all test clean

all: $(LIB)

test: $(TARGET)

$(LIB): $(LIB_OBJ) | build
	ar rcs $@ $^

$(TARGET): $(APP_OBJ) $(LIB) | build
	$(CC) $(CFLAGS) -o $@ $(APP_OBJ) $(LIB)

./build/csv.o: src/csv.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

./build/main.o: test/main.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	$(RM) -r build
