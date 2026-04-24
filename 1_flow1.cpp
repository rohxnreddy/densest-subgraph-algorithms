#include <algorithm>
#include <cstring>
#include <iostream>
#include <limits>
#include <cmath>
#include <vector>
#include <cstdio>

using namespace std;

// Global variables to store graph and results
string inputFile, outputFile;
int n, m;
double max_density = 0.0;
vector<int> densest_nodes;

struct EdgeInfo {
    int to;
    int id;
};
vector<vector<EdgeInfo>> adj_id;
vector<pair<int, int>> edge_list;

// --- Ultra-Fast I/O Parsing from Buffer ---
inline bool parse_two_ints(char*& ptr, int& u, int& v) {
    while (*ptr && isspace(*ptr)) ++ptr;
    if (!*ptr) return false;

    u = 0;
    while (*ptr >= '0' && *ptr <= '9') u = u * 10 + (*ptr++ - '0');

    while (*ptr && isspace(*ptr)) ++ptr;
    if (!*ptr) return false;

    v = 0;
    while (*ptr >= '0' && *ptr <= '9') v = v * 10 + (*ptr++ - '0');

    return true;
}

// --- Max Flow (Dinic's Algorithm) Implementation ---
struct Edge {
    int to;
    int rev;
    double capacity;
    double flow;
};

vector<vector<Edge>> flow_adj;
vector<int> level_arr;
vector<int> ptr_arr;
vector<int> flat_q;

void add_flow_edge(int from, int to, double cap) {
    flow_adj[from].push_back({to, (int)flow_adj[to].size(), cap, 0});
    flow_adj[to].push_back({from, (int)flow_adj[from].size() - 1, 0, 0});
}

bool bfs(int s, int t, int T_node) {
    fill(level_arr.begin(), level_arr.begin() + T_node + 1, -1);
    level_arr[s] = 0;

    int head = 0, tail = 0;
    flat_q[tail++] = s;

    while (head < tail) {
        int v = flat_q[head++];
        for (auto &edge : flow_adj[v]) {
            if (edge.capacity - edge.flow > 1e-9 && level_arr[edge.to] == -1) {
                level_arr[edge.to] = level_arr[v] + 1;
                flat_q[tail++] = edge.to;
            }
        }
    }
    return level_arr[t] != -1;
}

double dfs(int v, int t, double pushed) {
    if (pushed == 0 || v == t) return pushed;
    for (int &cid = ptr_arr[v]; cid < (int)flow_adj[v].size(); ++cid) {
        auto &edge = flow_adj[v][cid];
        int tr = edge.to;
        if (level_arr[v] + 1 != level_arr[tr] || edge.capacity - edge.flow < 1e-9)
            continue;
        double tr_pushed = dfs(tr, t, min(pushed, edge.capacity - edge.flow));
        if (tr_pushed == 0) continue;
        edge.flow += tr_pushed;
        flow_adj[tr][edge.rev].flow -= tr_pushed;
        return tr_pushed;
    }
    return 0;
}

double dinic(int s, int t, int T_node) {
    double flow = 0;
    while (bfs(s, t, T_node)) {
        fill(ptr_arr.begin(), ptr_arr.begin() + T_node + 1, 0);
        while (double pushed = dfs(s, t, numeric_limits<double>::infinity())) {
            flow += pushed;
        }
    }
    return flow;
}

struct SinkEdgeRef {
    int u;
    int idx;
};

void algorithm() {
    if (n == 0 || m == 0) return;

    int S_node = 0;
    // Nodes: S (1) + Vertices (n) + Edges (m) + T (1)
    int T_node = n + m + 1;
    int max_nodes = T_node + 1;

    flow_adj.resize(max_nodes);
    level_arr.resize(max_nodes);
    ptr_arr.resize(max_nodes);
    flat_q.resize(max_nodes);

    vector<int> t_count(n, 0);

    // 1. Find Triangles and Build V -> Lambda edges
    for (int u = 0; u < n; ++u) {
        for (auto& edge1 : adj_id[u]) {
            int v = edge1.to;
            int e_uv = edge1.id;
            if (v <= u) continue;

            for (auto& edge2 : adj_id[v]) {
                int w = edge2.to;
                int e_vw = edge2.id;
                if (w <= v) continue;

                // Binary search for w in u's neighborhood
                auto it = lower_bound(adj_id[u].begin(), adj_id[u].end(), w,
                    [](const EdgeInfo& e, int val) { return e.to < val; });

                if (it != adj_id[u].end() && it->to == w) {
                    int e_uw = it->id;

                    t_count[u]++;
                    t_count[v]++;
                    t_count[w]++;

                    // Add an edge v -> \psi with capacity 1
                    add_flow_edge(w + 1, e_uv + n + 1, 1.0);
                    add_flow_edge(u + 1, e_vw + n + 1, 1.0);
                    add_flow_edge(v + 1, e_uw + n + 1, 1.0);
                }
            }
        }
    }

    int max_degree = 0;
    for (int i = 0; i < n; ++i) {
        if (t_count[i] > max_degree) max_degree = t_count[i];
    }

    if (max_degree == 0) return; // No motifs found

    // 2. Add Source -> V edges
    for (int i = 0; i < n; ++i) {
        if (t_count[i] > 0) {
            add_flow_edge(S_node, i + 1, t_count[i]);
        }
    }

    // 3. Add Lambda -> V edges (capacity +infinity)
    for (int i = 0; i < m; ++i) {
        int e_node = i + n + 1;
        int u_node = edge_list[i].first + 1;
        int v_node = edge_list[i].second + 1;
        add_flow_edge(e_node, u_node, 1e15);
        add_flow_edge(e_node, v_node, 1e15);
    }

    // 4. Add V -> Sink edges
    vector<SinkEdgeRef> sink_edges;
    sink_edges.reserve(n);
    for (int i = 0; i < n; ++i) {
        if (t_count[i] > 0) {
            int v_node = i + 1;
            add_flow_edge(v_node, T_node, 0.0); // Temporary capacity
            sink_edges.push_back({v_node, (int)flow_adj[v_node].size() - 1});
        }
    }

    // Binary Search Initialization
    double l = 0;
    double u_bound = max_degree;
    double EPS = 1.0 / ((double)n * (n - 1));

    int max_iters = 1;
    if (u_bound - l >= EPS) {
        max_iters = (int)ceil(log2((u_bound - l) / EPS)) + 1;
    }
    const int bar_width = 40;

    // Hard cap at 60 to prevent max_iters from getting absurdly high
    if (max_iters > 60) max_iters = 60;

    int iter = 1;
    for (; iter <= max_iters; ++iter) {
        double alpha = (l + u_bound) / 2.0;

        // Failsafe: break instantly if floating-point precision stagnates
        if (alpha == l || alpha == u_bound) break;

        // Reset flows
        for (int i = 0; i <= T_node; ++i) {
            for (auto &edge : flow_adj[i]) edge.flow = 0;
        }

        // Update capacities for V -> Sink. For 3-cliques, |V_psi| = 3.
        for (const auto &ref : sink_edges) {
            flow_adj[ref.u][ref.idx].capacity = 3.0 * alpha;
        }

        dinic(S_node, T_node, T_node);

        // Extract minimum st-cut reachable from S
        vector<int> current_S;
        vector<char> visited(T_node + 1, 0);

        int head = 0, tail = 0;
        flat_q[tail++] = S_node;
        visited[S_node] = 1;

        while (head < tail) {
            int curr = flat_q[head++];
            if (curr > 0 && curr <= n) {
                current_S.push_back(curr - 1);
            }
            for (auto &edge : flow_adj[curr]) {
                if (edge.capacity - edge.flow > 1e-9 && !visited[edge.to]) {
                    visited[edge.to] = 1;
                    flat_q[tail++] = edge.to;
                }
            }
        }

        if (!current_S.empty()) {
            l = alpha;
            max_density = alpha;
            densest_nodes = current_S;
        } else {
            u_bound = alpha;
        }

        // Render progress
        double frac = (double)iter / (double)max_iters;
        if (frac < 0.0) frac = 0.0;
        if (frac > 1.0) frac = 1.0;
        int pos = (int)round(frac * bar_width);

        cerr << '\r' << "[";
        for (int i = 0; i < bar_width; ++i) cerr << (i < pos ? '#' : '-');
        cerr << "] " << (int)round(frac * 100.0) << "% (" << iter << "/" << max_iters << ")" << flush;
    }

    // Correctly format the final progress bar line
    int final_iters = iter > max_iters ? max_iters : iter - 1;
    cerr << '\r' << "[";
    for (int i = 0; i < bar_width; ++i) cerr << '#';
    cerr << "] 100% (" << final_iters << "/" << max_iters << ")" << flush << '\n';
}

void read_input(string filename) {
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    vector<char> buffer(fileSize + 1);
    if (fread(buffer.data(), 1, fileSize, file) != (size_t)fileSize) {
        // Read fallback handling if needed
    }
    buffer[fileSize] = '\0';
    fclose(file);

    char* ptr = buffer.data();
    int u, v;
    int maxNode = -1;
    vector<pair<int,int>> edges;

    while (*ptr) {
        if (*ptr == '#') {
            while (*ptr && *ptr != '\n') ++ptr;
            continue;
        }

        if (!isspace(*ptr) && *ptr >= '0' && *ptr <= '9') {
            if (parse_two_ints(ptr, u, v)) {
                if (u != v) {
                    int a = min(u, v);
                    int b = max(u, v);
                    edges.emplace_back(a, b);
                    if (a > maxNode) maxNode = a;
                    if (b > maxNode) maxNode = b;
                }
            }
        } else {
            ++ptr;
        }
    }

    n = maxNode + 1;
    if (n <= 0) {
        adj_id.clear();
        return;
    }

    sort(edges.begin(), edges.end());
    edges.erase(unique(edges.begin(), edges.end()), edges.end());

    m = edges.size();
    edge_list = edges;
    adj_id.assign(n, vector<EdgeInfo>());

    for (int i = 0; i < m; ++i) {
        int eu = edges[i].first;
        int ev = edges[i].second;
        adj_id[eu].push_back({ev, i});
        adj_id[ev].push_back({eu, i});
    }

    for (int i = 0; i < n; ++i) {
        sort(adj_id[i].begin(), adj_id[i].end(), [](const EdgeInfo& a, const EdgeInfo& b){
            return a.to < b.to;
        });
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

    sort(densest_nodes.begin(), densest_nodes.end());

    long long internal_edges = 0;
    vector<bool> in_subgraph(n, false);
    for (int v : densest_nodes) in_subgraph[v] = true;

    for (int u : densest_nodes) {
        for (auto& edge : adj_id[u]) {
            if (in_subgraph[edge.to]) {
                internal_edges++;
            }
        }
    }
    internal_edges /= 2;

    fprintf(file, "Algorithm: Exact (h=3)\n");
    fprintf(file, "Density: %.6f\n", max_density);
    fprintf(file, "Number of nodes: %d\n", (int)densest_nodes.size());
    fprintf(file, "Number of edges: %lld\n", internal_edges);

    fprintf(file, "Nodes: ");
    for (int v : densest_nodes) {
        fprintf(file, " %d ,", v);
    }
    fprintf(file, "\n");

    if (file != stdout) fclose(file);
}

int main(int argc, char *argv[]) {
    inputFile = "testcases/Wiki-Vote.txt";
    outputFile = "stdout";

    if (argc >= 2) inputFile = argv[1];
    if (argc >= 3) outputFile = argv[2];

    read_input(inputFile);
    algorithm();
    print_output(outputFile);

    return 0;
}
