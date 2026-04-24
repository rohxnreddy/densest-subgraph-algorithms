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

static void print_progress_bar(long long current, long long total, int width = 40) {
    if (total <= 0) return;
    if (current < 0) current = 0;
    if (current > total) current = total;

    double ratio = (total == 0) ? 1.0 : (double)current / (double)total;
    int filled = (int)(ratio * width);
    if (filled < 0) filled = 0;
    if (filled > width) filled = width;

    cerr << '\r' << '[';
    for (int i = 0; i < width; ++i) {
        cerr << (i < filled ? '#' : '-');
    }
    int pct = (int)(ratio * 100.0);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    cerr << "] " << pct << "% (" << current << "/" << total << ')' << flush;

    if (current >= total) cerr << '\n';
}

//Zero-based indexing for nodes
void read_input(string filename) {

    FILE* file = fopen(filename.c_str(), "r");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }

    char line[256];
    int u, v;
    int maxNode = -1;

    set<pair<int,int>> edges;

    while (fgets(line, sizeof(line), file)) {

        if (line[0] == '#') continue;

        if (sscanf(line, "%d %d", &u, &v) == 2) {

            if (u == v) continue;

            int a = min(u, v);
            int b = max(u, v);

            edges.insert({a, b});

            maxNode = max(maxNode, max(u, v));
        }
    }

    fclose(file);

    n = maxNode + 1;
    m = edges.size();

    adj.assign(n, vector<int>());
    density = 0.0;
    nodes.clear();

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
    using HeapIt = set<pair<int, int>>::iterator;
    vector<HeapIt> heap_it(n, min_degree_heap.end());
    vector<bool> in_h(n, true);
    vector<int> removed_at(n, 0);
    int removed_count = 0;

    for (int i = 0; i < n; ++i) {
        current_degrees[i] = adj[i].size();
        heap_it[i] = min_degree_heap.insert({current_degrees[i], i}).first;
    }

    long long current_edge_count = m;
    int current_node_count = n;
    double best_density = (double)current_edge_count / current_node_count;
    int best_removed_count = 0; // 0 means keep all nodes

    const int total_steps = n;
    const int update_every = max(1, total_steps / 200); // ~200 updates max

    while (!min_degree_heap.empty()) {
        int u = min_degree_heap.begin()->second;
        min_degree_heap.erase(min_degree_heap.begin());
        heap_it[u] = min_degree_heap.end();
        in_h[u] = false;
        removed_at[u] = ++removed_count;

        for (int v : adj[u]) {
            if (in_h[v]) {
                if (heap_it[v] != min_degree_heap.end()) {
                    min_degree_heap.erase(heap_it[v]);
                }
                current_degrees[v]--;
                heap_it[v] = min_degree_heap.insert({current_degrees[v], v}).first;
                current_edge_count--;
            }
        }
        current_node_count--;
        if (current_node_count > 0) {
            double current_density = (double)current_edge_count / current_node_count;
            if (current_density > best_density) {
                best_density = current_density;
                best_removed_count = removed_count;
            }
        }

        if (total_steps >= 200 && (removed_count == 1 || removed_count % update_every == 0 || removed_count == total_steps)) {
            print_progress_bar(removed_count, total_steps);
        }
    }

    density = best_density;
    nodes.clear();
    nodes.reserve(max(0, n - best_removed_count));
    for (int v = 0; v < n; ++v) {
        if (removed_at[v] > best_removed_count) nodes.push_back(v);
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

    // --- Add this block to count internal edges ---
    long long internal_edges = 0;
    vector<bool> in_subgraph(n, false);
    for (int v : nodes) in_subgraph[v] = true;

    for (int u : nodes) {
        for (int v : adj[u]) {
            if (in_subgraph[v]) {
                internal_edges++;
            }
        }
    }
    internal_edges /= 2;
    // ----------------------------------------------

    fprintf(file, "Algorithm: Greedy\n");
    fprintf(file, "Density: %.6f\n", density);
    fprintf(file, "Number of nodes: %d\n", (int)nodes.size());
    fprintf(file, "Number of edges: %lld\n", internal_edges); // New Line

    fprintf(file, "Nodes: ");
    for (int v : nodes) {
        fprintf(file, " %d ,", v);
    }
    fprintf(file, "\n");

    if (file != stdout) fclose(file);
}

int main(int argc, char* argv[]){
    inputFile = "testcases/wiki-Vote.txt";
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
