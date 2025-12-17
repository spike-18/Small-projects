MPICXX = mpic++
MPICXXFLAGS = -Wall -O3 -Wextra -Werror=return-type -std=c++23

CXX = g++
CXXFLAGS = -Wall -O3 -fopenmp -Wextra -Werror=return-type -std=c++23

MPI_SOURCES := $(wildcard src/*mpi.cpp)
MPI_TARGETS := $(addprefix bin/, $(MPI_SOURCES:src/%.cpp=%))

SOURCES := $(filter-out $(MPI_SOURCES), $(wildcard src/*.cpp))
TARGETS := $(addprefix bin/, $(SOURCES:src/%.cpp=%))

.PHONY: clean all run

all: $(TARGETS) $(MPI_TARGETS)

run: plots/task_1.png plots/task_2.png plots/task_3.png results/main

plots/task_1.png: results/task_1.csv scripts/plot_1.plt
	./scripts/plot_1.plt

results/task_1.csv: scripts/task_1.sh bin/task_1_base bin/task_1_mpi
	./scripts/task_1.sh > results/task_1.csv

plots/task_2.png: results/task_2.csv scripts/plot_2.plt
	./scripts/plot_2.plt

results/task_2.csv: scripts/task_2.sh bin/task_2_base bin/task_2_omp
	./scripts/task_2.sh > results/task_2.csv

plots/task_3.png: results/task_3.csv scripts/plot_3.plt
	./scripts/plot_3.plt

results/task_3.csv: scripts/task_3.sh bin/task_3_base bin/task_3_omp
	./scripts/task_3.sh > results/task_3.csv

results/main: scripts/main.sh bin/main_base bin/main_mpi bin/main_omp

bin/%_mpi: src/%_mpi.cpp
	mkdir -p bin
	$(MPICXX) $(MPICXXFLAGS) -o $@ $<


bin/%: src/%.cpp
	mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f bin/*
