CXX = g++
CXXFLAGS = -Wall -O3 -fopenmp -Wextra -Werror=return-type -std=c++23

SOURCES := $(wildcard src/*.cpp)
TARGETS := $(addprefix bin/, $(SOURCES:src/%.cpp=%))

.PHONY: clean all

all: $(TARGETS) $(MPI_TARGETS)

bin/%: src/%.cpp
	mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f bin/*
