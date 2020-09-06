CC=g++

huever:
	$(CC) -O3 -o huever src/main.cpp

.PHONY: clean

clean:
	rm huever
