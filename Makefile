INCLUDE_DIR = include
OUTPUT_FILE = broking-example.out

CXX = g++
CXXFLAGS = -std=c++11 -g -Wall -pedantic -Wextra
INCLFLAGS = -I $(INCLUDE_DIR) 

SOURCES += $(wildcard src/broking/*.cpp)
SOURCES += main.cpp

all: $(SOURCES)
	$(CXX) -o $(OUTPUT_FILE) $(CXXFLAGS) $(INCLFLAGS) $(SOURCES)
	
.PHONY: clean
clean:
	rm -f $(OUTPUT_FILE)