CC = g++
CFLAGS = $(shell pkg-config --cflags raylib)
LDFLAGS = $(shell pkg-config --libs raylib)

# Automatically find all .cpp files in current directory and src/
SOURCES = $(wildcard *.cpp src/*.cpp)

hello: $(SOURCES)
	$(CC) $(SOURCES) -o hello $(CFLAGS) $(LDFLAGS)

clean:
	rm -f hello

.PHONY: clean