# Densest Subgraph Algorithms — Makefile

CXX ?= g++
CXXFLAGS ?= -O2 -std=c++17

BINARIES := flow1 flow4 greedy greedypp

.PHONY: all clean run

all: $(BINARIES)

flow1: 1_flow1.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

flow4: 2_flow4.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

greedy: 3_greedy.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

greedypp: 4_greedypp.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(BINARIES)

# Convenience target: generates input, builds, and runs all implementations
run: all
	bash runall.sh
