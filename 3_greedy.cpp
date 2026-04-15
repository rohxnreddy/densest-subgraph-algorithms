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
    density = 0.0;
    nodes.clear();

    if (n == 0) return;

    vector<int> current_degrees(n);
    set<pair<int, int>> min_degree_heap;
    vector<bool> in_h(n, true);
    vector<int> current_nodes;

    for (int i = 0; i < n; ++i) {
        current_degrees[i] = adj[i].size();
        min_degree_heap.insert({current_degrees[i], i});
        current_nodes.push_back(i);
    }

    long long current_edge_count = m;
    int current_node_count = n;
    density = (double)current_edge_count / current_node_count;
    nodes = current_nodes;

    while (!min_degree_heap.empty()) {
        int u = min_degree_heap.begin()->second;
        min_degree_heap.erase(min_degree_heap.begin());
        in_h[u] = false;

        for (int v : adj[u]) {
            if (in_h[v]) {
                min_degree_heap.erase({current_degrees[v], v});
                current_degrees[v]--;
                min_degree_heap.insert({current_degrees[v], v});
                current_edge_count--;
            }
        }
        current_node_count--;
        if (current_node_count > 0) {
            double current_density = (double)current_edge_count / current_node_count;
            if(current_density > density) {
                density = current_density;
                nodes.clear();
                for (int i = 0; i < n; ++i) {
                    if (in_h[i]) nodes.push_back(i);
                }
            }
        }
    }
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

    fprintf(file, "Algorithm: Greedy \n");
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