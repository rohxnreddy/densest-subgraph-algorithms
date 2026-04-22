
# Densest Subgraph — Approximation & Flow-Based Algorithms (C++)

### Design and Analysis of Algorithms | BITS Pilani, Hyderabad Campus

C++ implementations / skeletons for multiple **densest subgraph** approaches on **unweighted, undirected** graphs.

## About

This repo contains four C++ programs (Flow Algo-1, Flow Algo-4, Greedy, Greedy++) that all:

- read the same edge-list input format,
- compute a candidate densest subgraph (set of nodes), and
- print the resulting node set and density.

`runall.sh` compiles and runs all implementations on one or more input files.

## Directory Structure

```
.
├── 1_flow1.cpp
├── 2_flow4.cpp
├── 3_greedy.cpp
├── 4_greedypp.cpp
├── Makefile
├── runall.sh
├── outputs/
│   ├── as-skitter_output.txt
│   ├── email-Enron_output.txt
│   └── wiki-Vote_output.txt
├── testcases/
│   ├── wiki-Vote.txt
│   ├── email-Enron.txt
│   └── as-skitter.txt.gz
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

## Testcases 

Testcases are under `testcases/`.

### Skitter: unzip before running

The Skitter edge list is large, so the plain-text file `testcases/as-skitter.txt` is **not committed** (it is listed in `.gitignore`).
Instead, the repo includes the compressed file:

- `testcases/as-skitter.txt.gz`

To generate the ignored text file locally, unzip it:

```bash
gunzip -k testcases/as-skitter.txt.gz
```

## Run

### Run all implementations (recommended)

`runall.sh` will compile all four C++ programs and run them on one or more input files.

- With no args, it runs on the testcases listed inside the script (by default `testcases/wiki-Vote.txt`).
- With args, it treats each arg as an input edge-list file.

For each input file, it writes a separate output report under `outputs/` named `<testcase>_output.txt`.

```bash
bash runall.sh
bash runall.sh testcases/wiki-Vote.txt
```

### Run an individual implementation

Each C++ program accepts up to 2 CLI arguments:

```bash
./flow1   [input_file] [output_file]
./flow4   [input_file] [output_file]
./greedy  [input_file] [output_file]
./greedypp [input_file] [output_file]
```

- No args: reads `testcases/wiki-Vote.txt` (or pass an explicit input file), writes to stdout.
- One arg: `./flow1 path/to/graph.txt` writes to stdout.
- Two args: `./flow1 in.txt out.txt` writes exactly to `out.txt`.

Examples:

```bash
./greedy
./flow4 testcases/wiki-Vote.txt
./greedypp testcases/wiki-Vote.txt outputs/greedypp_output.txt
```

## Output format

Each implementation prints a short report:

- `Algorithm:` (label)
- `Density:` (floating-point density)
- `Number of nodes:` (size of the returned node set)
- `Nodes:` list (one node ID per line)

When using `runall.sh`, each testcase gets its own report file under `outputs/` named `<testcase>_output.txt`, with all four algorithms appended with section headers.

## Contributors

| GitHub ID | Name | ID Number |
| --------- | ---- | --------- |
| [Crisp79](https://github.com/Crisp79) | Akshith Vuppala | 2023A7PS0032H |
| [rohxnreddy](https://github.com/rohxnreddy) | Rohan Reddy Devarapalli | 2023A7PS0138H |
| [Parallax-Ace](https://github.com/Parallax-Ace) | Adithya Nama | 2023A7PS0171H |
| [venusai24](https://github.com/venusai24) | Venu Sai Yelesvarapu | 2023A7PS0149H |
| [satvik-A](https://github.com/satvik-A) | Satvik Aderla | 2023A7PS0167H |

## References
- See PDFs under `papers/` for additional background.

