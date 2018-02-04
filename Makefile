OUTPUT_FILE = broking-example.out

CXX = g++
CXXFLAGS = -std=c++11 -g -Wall -pedantic -Wextra -pthread
INCLFLAGS = -Iinclude -Ilogging/include

SOURCES += $(wildcard src/broking/*.cpp)
SOURCES += main.cpp
SOURCES += $(wildcard logging/src/logging/*.cpp)

all: $(SOURCES)
	$(CXX) -o $(OUTPUT_FILE) $(CXXFLAGS) $(INCLFLAGS) $(SOURCES)
	
.PHONY: clean
clean:
	rm -f $(OUTPUT_FILE)