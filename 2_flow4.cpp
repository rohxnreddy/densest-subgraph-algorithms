#include <algorithm>
#include <cstring>
#include <iostream>
#include <limits>
#include <queue>
#include <set>
#include <stack>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cmath>

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
vector<int> level_arr;
vector<int> ptr_arr;

void add_flow_edge(int from, int to, double cap) {
    flow_adj[from].push_back({to, cap, 0, (int)flow_adj[to].size()});
    flow_adj[to].push_back({from, 0, 0, (int)flow_adj[from].size() - 1});
}

bool bfs(int s, int t) {
    fill(level_arr.begin(), level_arr.end(), -1);
    level_arr[s] = 0;
    queue<int> q;
    q.push(s);
    while (!q.empty()) {
        int v = q.front();
        q.pop();
        for (auto &edge : flow_adj[v]) {
            if (edge.capacity - edge.flow > 1e-9 && level_arr[edge.to] == -1) {
                level_arr[edge.to] = level_arr[v] + 1;
                q.push(edge.to);
            }
        }
    }
    return level_arr[t] != -1;
}

double dfs(int v, int t, double pushed) {
    if (pushed == 0) return 0;
    if (v == t) return pushed;
    for (int &cid = ptr_arr[v]; cid < flow_adj[v].size(); ++cid) {
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

double dinic(int s, int t) {
    double flow = 0;
    while (bfs(s, t)) {
        fill(ptr_arr.begin(), ptr_arr.end(), 0);
        while (double pushed = dfs(s, t, numeric_limits<double>::infinity())) {
            flow += pushed;
        }
    }
    return flow;
}

// Zero-based indexing for nodes
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

    // Helper to evaluate and globally track the best subset
    auto evaluate_subset = [&](const vector<int>& subset) {
        if (subset.empty()) return;
        unordered_set<int> in_sub(subset.begin(), subset.end());
        int e_count = 0;
        for (int u : subset) {
            for (int v : adj[u]) {
                if (u < v && in_sub.count(v)) e_count++;
            }
        }
        double cur_rho = (double)e_count / subset.size();
        if (cur_rho > density) {
            density = cur_rho;
            nodes = subset;
        }
    };

    // Step 1: Core decomposition (Corrected Batagelj & Zaveršnik)
    vector<int> deg(n);
    for (int i = 0; i < n; ++i) deg[i] = adj[i].size();

    set<pair<int, int>> pq;
    for (int i = 0; i < n; ++i) pq.insert({deg[i], i});

    vector<int> core(n);
    vector<bool> removed(n, false);
    int V_res = n;
    int E_res = m;
    double max_rho = 0.0;
    int k_max = 0;
    int current_core = 0; // Tracks the peeling level

    while (!pq.empty()) {
        double current_rho = (V_res > 0) ? (double)E_res / V_res : 0.0;
        max_rho = max(max_rho, current_rho);

        int d = pq.begin()->first;
        int u = pq.begin()->second;
        pq.erase(pq.begin());

        // FIX: A node's core number cannot be lower than the graph's current peel level
        current_core = max(current_core, d);
        core[u] = current_core;
        k_max = max(k_max, current_core);
        removed[u] = true;
        V_res--;

        for (int v : adj[u]) {
            if (!removed[v]) {
                pq.erase({deg[v], v});
                deg[v]--;
                E_res--; 
                pq.insert({deg[v], v});
            }
        }
    }

    // Helper to get connected components
    auto get_ccs = [&](const vector<int>& valid_nodes) {
        vector<vector<int>> ccs;
        unordered_set<int> valid_set(valid_nodes.begin(), valid_nodes.end());
        unordered_set<int> vis;
        for (int u : valid_nodes) {
            if (!vis.count(u)) {
                vector<int> comp;
                queue<int> q;
                q.push(u);
                vis.insert(u);
                while (!q.empty()) {
                    int curr = q.front(); 
                    q.pop();
                    comp.push_back(curr);
                    for (int v : adj[curr]) {
                        if (valid_set.count(v) && !vis.count(v)) {
                            vis.insert(v);
                            q.push(v);
                        }
                    }
                }
                ccs.push_back(comp);
            }
        }
        return ccs;
    };

    // Pruning: Locate k'-core
    int k_prime = ceil(max_rho);
    vector<int> k_prime_nodes;
    for (int i = 0; i < n; ++i) {
        if (core[i] >= k_prime) k_prime_nodes.push_back(i);
    }
    
    vector<vector<int>> ccs_prime = get_ccs(k_prime_nodes);
    double max_rho_double_prime = 0.0;
    for (auto& comp : ccs_prime) {
        int comp_edges = 0;
        unordered_set<int> comp_set(comp.begin(), comp.end());
        for (int u : comp) {
            for (int v : adj[u]) {
                if (u < v && comp_set.count(v)) comp_edges++;
            }
        }
        double r = (double)comp_edges / comp.size();
        max_rho_double_prime = max(max_rho_double_prime, r);
    }

    int k_double_prime = ceil(max_rho_double_prime);
    double l = max_rho_double_prime;

    vector<int> k_double_prime_nodes;
    for (int i = 0; i < n; ++i) {
        if (core[i] >= k_double_prime) k_double_prime_nodes.push_back(i);
    }

    vector<vector<int>> C_list = get_ccs(k_double_prime_nodes);

    // Flow Network Constructor
    auto build_and_cut = [&](const vector<int>& current_comp, double alpha) -> vector<int> {
        int num_nodes = current_comp.size();
        if (num_nodes == 0) return {};
        
        int S_node = 0, T_node = num_nodes + 1;
        flow_adj.assign(num_nodes + 2, vector<Edge>());
        level_arr.assign(num_nodes + 2, -1);
        ptr_arr.assign(num_nodes + 2, 0);

        unordered_map<int, int> to_local;
        for (int i = 0; i < num_nodes; ++i) {
            to_local[current_comp[i]] = i + 1;
        }

        vector<int> deg_C(num_nodes + 1, 0);
        vector<pair<int, int>> edges_C;
        
        for (int i = 0; i < num_nodes; ++i) {
            int u = current_comp[i];
            for (int v : adj[u]) {
                if (u < v && to_local.count(v)) {
                    edges_C.push_back({to_local[u], to_local[v]});
                    deg_C[to_local[u]]++;
                    deg_C[to_local[v]]++;
                }
            }
        }

        for (int i = 1; i <= num_nodes; ++i) {
            add_flow_edge(S_node, i, (double)deg_C[i]);
            add_flow_edge(i, T_node, 2.0 * alpha);
        }
        for (auto e : edges_C) {
            add_flow_edge(e.first, e.second, 1.0);
            add_flow_edge(e.second, e.first, 1.0);
        }

        dinic(S_node, T_node);

        vector<bool> visited(num_nodes + 2, false);
        queue<int> q;
        q.push(S_node);
        visited[S_node] = true;
        vector<int> U;
        
        while (!q.empty()) {
            int v = q.front(); q.pop();
            if (v >= 1 && v <= num_nodes) U.push_back(current_comp[v - 1]);
            
            for (auto& edge : flow_adj[v]) {
                if (edge.capacity - edge.flow > 1e-9 && !visited[edge.to]) {
                    visited[edge.to] = true;
                    q.push(edge.to);
                }
            }
        }
        return U;
    };

    // Iterating components
    for (auto& initial_comp : C_list) {
        vector<int> comp = initial_comp;
        evaluate_subset(comp);
        
        int req_core = ceil(l);
        vector<int> next_comp;
        for (int v : comp) {
            if (core[v] >= req_core) next_comp.push_back(v);
        }
        comp = next_comp;
        
        if (comp.size() <= 1) continue;

        vector<int> init_U = build_and_cut(comp, l);
        if (init_U.empty()) continue; 

        double u_bound = k_max; 
        double eps = 1.0 / ((double)comp.size() * (comp.size() - 1.0));

        // Binary Search Process
        while (u_bound - l >= eps) {
            double alpha = (l + u_bound) / 2.0;
            vector<int> U = build_and_cut(comp, alpha);
            
            if (U.empty()) {
                u_bound = alpha; 
            } else {
                if (alpha > ceil(l)) {
                    int current_ceil = ceil(alpha);
                    vector<int> filtered_comp;
                    for (int v : comp) {
                        if (core[v] >= current_ceil) filtered_comp.push_back(v);
                    }
                    comp = filtered_comp;
                    if (comp.size() > 1) {
                        eps = 1.0 / ((double)comp.size() * (comp.size() - 1.0));
                    }
                }
                l = alpha;
                evaluate_subset(U);
            }
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

    fprintf(file, "Algorithm: CoreExact\n");
    fprintf(file, "Density: %.6f\n", density);
    fprintf(file, "Number of nodes: %d\n", (int)nodes.size());

    fprintf(file, "Nodes:\n");
    for (int v : nodes) {
        fprintf(file, "%d\n", v);
    }

    if (file != stdout) fclose(file);
}

int main(int argc, char *argv[]) {
    inputFile = "testcases/Wiki-Vote.txt";
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