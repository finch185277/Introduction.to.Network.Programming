CXX = clang++
ARG = -O3 -std=c++17 -g -o
RAW_FILES = main.cpp
EXEC_FILE = main

all: $(RAW_FILES)
	$(CXX) $(ARG) $(EXEC_FILE) $^

.PHONY: clean
clean:
	rm -rf $(EXEC_FILE)
