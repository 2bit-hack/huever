CC=g++

huever:
	$(CC) -o huever src/main.cpp

.PHONY: clean

clean:
	rm huever