#include <algorithm>
#include <cstring>
#include <iostream>
#include <limits>
#include <queue>
#include <set>
#include <stack>
#include <vector>

using namespace std;

string inputFile, outputFile;

int n, m;
vector<vector<int>> adj;

double density;
vector<int> nodes;

// --- Max Flow (Dinic's Algorithm) Implementation ---
struct Edge {
    int to;
    double capacity;
    double flow;
    int rev;
};

vector<vector<Edge>> flow_adj;
vector<int> level;
vector<int> ptr;

void add_flow_edge(int from, int to, double cap) {
    flow_adj[from].push_back({to, cap, 0, (int)flow_adj[to].size()});
    flow_adj[to].push_back({from, 0, 0, (int)flow_adj[from].size() - 1});
}

bool bfs(int s, int t) {
    fill(level.begin(), level.end(), -1);
    level[s] = 0;
    queue<int> q;
    q.push(s);
    while (!q.empty()) {
        int v = q.front();
        q.pop();
        for (auto &edge : flow_adj[v]) {
            if (edge.capacity - edge.flow > 1e-9 && level[edge.to] == -1) {
                level[edge.to] = level[v] + 1;
                q.push(edge.to);
            }
        }
    }
    return level[t] != -1;
}

double dfs(int v, int t, double pushed) {
    if (pushed == 0)
        return 0;
    if (v == t)
        return pushed;
    for (int &cid = ptr[v]; cid < flow_adj[v].size(); ++cid) {
        auto &edge = flow_adj[v][cid];
        int tr = edge.to;
        if (level[v] + 1 != level[tr] || edge.capacity - edge.flow < 1e-9)
            continue;
        double tr_pushed = dfs(tr, t, min(pushed, edge.capacity - edge.flow));
        if (tr_pushed == 0)
            continue;
        edge.flow += tr_pushed;
        flow_adj[tr][edge.rev].flow -= tr_pushed;
        return tr_pushed;
    }
    return 0;
}

double dinic(int s, int t) {
    double flow = 0;
    while (bfs(s, t)) {
        fill(ptr.begin(), ptr.end(), 0);
        while (double pushed = dfs(s, t, numeric_limits<double>::infinity())) {
            flow += pushed;
        }
    }
    return flow;
}

// Zero-based indexing for nodes
void read_input(string filename) {

    FILE *file = fopen(filename.c_str(), "r");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }

    int u, v;

    if (fscanf(file, "%d %d", &n, &m) != 2) {
        cout << "Invalid input format\n";
        exit(1);
    }

    set<pair<int, int>> edges;

    while (fscanf(file, "%d %d", &u, &v) == 2) {

        if (u == v)
            continue;

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

    // For h=2 (standard density), cliques Lambda are just the edges
    // For general h, you would need a clique-finding preprocessing step here
    double l = 0;
    double u = m;
    double EPS = 1.0 / (n * (n - 1)); // Theoretical precision bound

    while (u - l > EPS) {
        double alpha = (l + u) / 2.0;
        int s = 0, t = n + 1;
        flow_adj.assign(n + 2, vector<Edge>());
        level.resize(n + 2);
        ptr.resize(n + 2);

        for (int i = 1; i <= n; ++i) {
            // Source to vertex: capacity is degree
            add_flow_edge(s, i, (double)adj[i - 1].size());
            // Vertex to sink: capacity is 2 * alpha
            add_flow_edge(i, t, 2.0 * alpha);

            // Edges between vertices: capacity 1
            for (int neighbor : adj[i - 1]) {
                add_flow_edge(i, neighbor + 1, 1.0);
            }
        }

        dinic(s, t);

        // Find reachable nodes from s in the residual graph (Min-Cut S set)
        vector<int> current_S;
        vector<bool> visited(n + 2, false);
        queue<int> q;
        q.push(s);
        visited[s] = true;
        while (!q.empty()) {
            int v = q.front();
            q.pop();
            if (v >= 1 && v <= n)
                current_S.push_back(v - 1);
            for (auto &edge : flow_adj[v]) {
                if (edge.capacity - edge.flow > 1e-9 && !visited[edge.to]) {
                    visited[edge.to] = true;
                    q.push(edge.to);
                }
            }
        }

        if (current_S.empty()) {
            u = alpha;
        } else {
            l = alpha;
            density = alpha;
            nodes = current_S;
        }
    }
}

void print_output(string filename) {
    FILE *file;

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

    if (file != stdout)
        fclose(file);
}

int main(int argc, char *argv[]) {
    inputFile = "inputs/input.txt";
    outputFile = "stdout";

    if (argc == 2) {
        inputFile = argv[1];
    }

    if (argc == 3) {
        inputFile = argv[1];
        outputFile = argv[2];
    }

    system("mkdir -p outputs");

    read_input(inputFile);

    algorithm();

    print_output(outputFile);

    return 0;
}
