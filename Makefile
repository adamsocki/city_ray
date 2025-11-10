CC = g++

# Automatically find all .cpp files in current directory and src/
SOURCES = $(wildcard *.cpp src/*.cpp src/*/*.cpp)

# Detect OS and set target name and flags
ifeq ($(OS),Windows_NT)
	TARGET = hello_pc.exe
	# Try pkg-config first, fallback to manual paths
	CFLAGS = $(shell pkg-config --cflags raylib 2>nul || echo -IC:/msys64/mingw64/include)
	LDFLAGS = $(shell pkg-config --libs raylib 2>nul || echo -LC:/msys64/mingw64/lib -lraylib -lopengl32 -lgdi32 -lwinmm)
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		TARGET = hello_mac
		CFLAGS = $(shell pkg-config --cflags raylib)
		LDFLAGS = $(shell pkg-config --libs raylib)
	else
		TARGET = hello_linux
		CFLAGS = $(shell pkg-config --cflags raylib)
		LDFLAGS = $(shell pkg-config --libs raylib)
	endif
endif

CFLAGS += -I.

$(TARGET): $(SOURCES)
	$(CC) $(SOURCES) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f hello hello.exe hello_pc.exe hello_mac hello_linux

.PHONY: clean