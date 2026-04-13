
# Densest Subgraph — Approximation & Flow-Based Algorithms (C++)

### Design and Analysis of Algorithms | BITS Pilani, Hyderabad Campus

C++ implementations / skeletons for multiple **densest subgraph** approaches on **unweighted, undirected** graphs.

## About

This repo contains four C++ programs (Flow Algo-1, Flow Algo-4, Greedy, Greedy++) that all:

- read the same edge-list input format,
- compute a candidate densest subgraph (set of nodes), and
- print the resulting node set and density.

A small Python generator is included to create random test graphs, and `runall.sh` compiles and runs all implementations on the generated input.

## Directory Structure

```
.
├── 1_flow1.cpp
├── 2_flow4.cpp
├── 3_greedy.cpp
├── 4_greedypp.cpp
├── generator.py
├── runall.sh
├── inputs/
│   └── input.txt
├── outputs/
│   └── output.txt
└── papers/
	├── dense-subgraph.pdf
	└── flowless.pdf
```

## Build

Compile the individual programs (creates binaries `flow1`, `flow4`, `greedy`, `greedypp`):

```bash
g++ -O2 1_flow1.cpp -o flow1
g++ -O2 2_flow4.cpp -o flow4
g++ -O2 3_greedy.cpp -o greedy
g++ -O2 4_greedypp.cpp -o greedypp
```

Or just run the provided script (it compiles for you):

```bash
bash runall.sh
```

## Run

### Run all implementations (recommended)

`runall.sh` will:

1) generate `inputs/input.txt` via `generator.py` (you can pass generator args through),
2) compile all four C++ programs, and
3) append all outputs into `outputs/output.txt`.

```bash
bash runall.sh
bash runall.sh -n 100 -m 500 --seed 42
```

### Run an individual implementation

Each C++ program accepts up to 2 CLI arguments:

```bash
./flow1   [input_file] [output_file]
./flow4   [input_file] [output_file]
./greedy  [input_file] [output_file]
./greedypp [input_file] [output_file]
```

- No args: reads `inputs/input.txt`, writes to stdout.
- One arg: `./flow1 path/to/graph.txt` writes to stdout.
- Two args: `./flow1 in.txt out.txt` writes exactly to `out.txt`.

Examples:

```bash
./greedy
./flow4 inputs/input.txt
./greedypp inputs/input.txt outputs/greedypp_output.txt
```

## Input format

Input is a plain-text undirected edge list.

The first line is:

```text
n m
```

where:

- `n` is the number of nodes (assumed 0-based IDs `0..n-1`),
- `m` is the (declared) number of edges.

Each subsequent line is an edge:

```text
u v
```

Notes (based on the current C++ parsing logic):

- self-loops (`u == v`) are ignored,
- duplicate edges are removed,
- edges are treated as undirected (`(u,v)` is the same as `(v,u)`).

## Output format

Each implementation prints a short report:

- `Algorithm:` (label)
- `Density:` (floating-point density)
- `Number of nodes:` (size of the returned node set)
- `Nodes:` list (one node ID per line)

When using `runall.sh`, outputs from all four programs are concatenated into `outputs/output.txt` with section headers.

## Generate a random graph

`generator.py` writes a random undirected graph to `inputs/input.txt`.

```bash
python3 generator.py
python3 generator.py -n 50 -m 200 --seed 7
```

Then run any implementation:

```bash
./flow1 inputs/input.txt
```

## Contributors

| GitHub ID | Name | ID Number |
| --------- | ---- | --------- |
| [Crisp79](https://github.com/Crisp79) | Akshith Vuppala | 2023A7PS0032H |
| [rohxnreddy](https://github.com/rohxnreddy) | Rohan Reddy Devarapalli | 2023A7PS0138H |
| [Parallax-Ace](https://github.com/Parallax-Ace) | Adithya Nama | 2023A7PS0171H |
| [venusai24](https://github.com/venusai24) | Venu Sai Yelesvarapu | 2023A7PS0149H |
| [satvik-A](https://github.com/satvik-A) | Satvik Aderla | 2023A7PS0167H |

## References

- Charikar, M. (2000). **Greedy approximation algorithms for finding dense components in a graph**.
- (See PDFs under `papers/` for additional background.)

