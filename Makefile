.phony: clean

all: run 

build:
	gcc main.c -o bnet -Wall

run: build
	./bnet

memtest: build
	valgrind --leak-check=full --show-leak-kinds=all ./bnet

clean:
	rm bnet *.out
