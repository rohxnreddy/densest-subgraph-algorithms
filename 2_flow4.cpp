#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <limits>
#include <cstring>
#include <algorithm>
#include <set>

using namespace std;

string inputFile, outputFile;

int n, m;
vector<vector<int>> adj;

double density;
vector<int> nodes;

//Zero-based indexing for nodes
void read_input(string filename) {

    FILE* file = fopen(filename.c_str(), "r");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }

    int u, v;

    if (fscanf(file, "%d %d", &n, &m) != 2) {
        cout << "Invalid input format\n";
        exit(1);
    }

    set<pair<int,int>> edges;

    while (fscanf(file, "%d %d", &u, &v) == 2) {

        if (u == v) continue; 

        int a = min(u, v);
        int b = max(u, v);

        edges.insert({a, b});
    }

    fclose(file);

    m = edges.size();

    adj.assign(n, vector<int>());

    for (auto &e : edges) {
        adj[e.first].push_back(e.second);
        adj[e.second].push_back(e.first);
    }
}

void algorithm() {
    // Implement the Alogoithm here
    // Input graph is stored in adj and number of nodes and edges are stored in n and m respectively
    // output the nodes in the densest subgraph in the vector "nodes" and the density in the variable "density"
    // do density = 0.0; nodes.clear();before starting the algorithm to initialize the output variables
    // Update Algorithm name in output function as well
}

void print_output(string filename){
    FILE* file;

    if (filename == "stdout") {
        file = stdout;
    } else {
        file = fopen(filename.c_str(), "w");
        if (!file) {
            perror("Error opening output file");
            exit(1);
        }
    }

    sort(nodes.begin(), nodes.end());

    fprintf(file, "Algorithm: \n");
    fprintf(file, "Density: %.6f\n", density);
    fprintf(file, "Number of nodes: %d\n", (int)nodes.size());

    fprintf(file, "Nodes:\n");
    for (int v : nodes) {
        fprintf(file, "%d\n", v);
    }

    if (file != stdout) fclose(file);
}

int main(int argc, char* argv[]){
    inputFile = "inputs/input.txt";
    outputFile = "stdout";

    if (argc == 2){
        inputFile = argv[1];
    }

    if (argc == 3){
        inputFile = argv[1];
        outputFile = argv[2];
    }

    system("mkdir -p outputs");

    read_input(inputFile);

    algorithm();

    print_output(outputFile);

    return 0;
}