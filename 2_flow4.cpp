#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <limits>
#include <cstring>
#include <algorithm>
#include <set>
#include <cmath>
#include <unordered_map>
#include <unordered_set>

using namespace std;

string inputFile, outputFile;

int n, m;
vector<vector<int>> adj;

double density;
vector<int> nodes;

struct Edge {
    int to;
    double cap, flow;
    int rev;
};

vector<vector<Edge>> adj_flow;
vector<int> level_arr, ptr_arr;

void add_edge(int from, int to, double cap) {
    adj_flow[from].push_back({to, cap, 0.0, (int)adj_flow[to].size()});
    adj_flow[to].push_back({from, 0.0, 0.0, (int)adj_flow[from].size() - 1});
}

bool bfs(int S, int T) {
    fill(level_arr.begin(), level_arr.end(), -1);
    level_arr[S] = 0;
    queue<int> q;
    q.push(S);
    while (!q.empty()) {
        int v = q.front(); 
        q.pop();
        for (auto& edge : adj_flow[v]) {
            if (edge.cap - edge.flow > 1e-7 && level_arr[edge.to] == -1) {
                level_arr[edge.to] = level_arr[v] + 1;
                q.push(edge.to);
            }
        }
    }
    return level_arr[T] != -1;
}

double dfs(int v, int T, double pushed) {
    if (pushed < 1e-7) return 0.0;
    if (v == T) return pushed;
    for (int& cid = ptr_arr[v]; cid < adj_flow[v].size(); ++cid) {
        auto& edge = adj_flow[v][cid];
        int tr = edge.to;
        if (level_arr[v] + 1 != level_arr[tr] || edge.cap - edge.flow < 1e-7) continue;
        double push = dfs(tr, T, min(pushed, edge.cap - edge.flow));
        if (push < 1e-7) continue;
        edge.flow += push;
        adj_flow[tr][edge.rev].flow -= push;
        return push;
    }
    return 0.0;
}

double dinic(int S, int T) {
    double flow = 0;
    while (bfs(S, T)) {
        fill(ptr_arr.begin(), ptr_arr.end(), 0);
        while (double pushed = dfs(S, T, 1e18)) {
            flow += pushed;
        }
    }
    return flow;
}

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
    vector<int> all_nodes(n);
    for (int i = 0; i < n; ++i) all_nodes[i] = i;
    evaluate_subset(all_nodes);
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

    while (!pq.empty()) {
        double current_rho = (V_res > 0) ? (double)E_res / V_res : 0.0;
        max_rho = max(max_rho, current_rho);

        int d = pq.begin()->first;
        int u = pq.begin()->second;
        pq.erase(pq.begin());

        core[u] = d;
        k_max = max(k_max, d);
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
    double u_bound = k_max;

    vector<int> k_double_prime_nodes;
    for (int i = 0; i < n; ++i) {
        if (core[i] >= k_double_prime) k_double_prime_nodes.push_back(i);
    }

    vector<vector<int>> C_list = get_ccs(k_double_prime_nodes);

    for (auto& initial_comp : C_list) {
        evaluate_subset(initial_comp);
        vector<int> comp = initial_comp;

        while (true) {
            if (comp.size() <= 1) break;
            int req_core = ceil(l);
            vector<int> next_comp;
            for (int v : comp) {
                if (core[v] >= req_core) next_comp.push_back(v);
            }
            comp = next_comp;

            if (comp.size() <= 1) break;

            int num_nodes = comp.size();
            unordered_map<int, int> to_local;
            for (int i = 0; i < num_nodes; ++i) to_local[comp[i]] = i + 1;

            int m_C = 0;
            vector<int> deg_C(num_nodes + 1, 0);
            vector<pair<int, int>> edges_C;

            for (int i = 0; i < num_nodes; ++i) {
                int u = comp[i];
                for (int v : adj[u]) {
                    if (u < v && to_local.count(v)) {
                        edges_C.push_back({to_local[u], to_local[v]});
                        deg_C[to_local[u]]++;
                        deg_C[to_local[v]]++;
                        m_C++;
                    }
                }
            }
            auto build_and_cut = [&](double alpha) -> vector<int> {
                int S = 0, T = num_nodes + 1;
                adj_flow.assign(num_nodes + 2, vector<Edge>());
                level_arr.assign(num_nodes + 2, -1);
                ptr_arr.assign(num_nodes + 2, 0);

                for (int i = 1; i <= num_nodes; ++i) {
                    add_edge(S, i, m_C);
                    add_edge(i, T, m_C + 2.0 * alpha - deg_C[i]);
                }
                for (auto e : edges_C) {
                    add_edge(e.first, e.second, 1.0);
                    add_edge(e.second, e.first, 1.0);
                }

                dinic(S, T);

                vector<bool> visited(num_nodes + 2, false);
                queue<int> q;
                q.push(S);
                visited[S] = true;
                while (!q.empty()) {
                    int v = q.front(); q.pop();
                    for (auto& edge : adj_flow[v]) {
                        if (edge.cap - edge.flow > 1e-7 && !visited[edge.to]) {
                            visited[edge.to] = true;
                            q.push(edge.to);
                        }
                    }
                }

                vector<int> U;
                for (int i = 1; i <= num_nodes; ++i) {
                    if (visited[i]) U.push_back(comp[i - 1]);
                }
                return U;
            };
            vector<int> U_l = build_and_cut(l);
            if (U_l.empty()) break; 

            double eps = 1.0 / ((double)num_nodes * (num_nodes - 1.0));
            bool shrunk = false;
            while (u_bound - l >= eps) {
                double alpha = (l + u_bound) / 2.0;
                vector<int> U = build_and_cut(alpha);
                
                if (U.empty()) {
                    u_bound = alpha; // S = {s}
                } else {
                    l = alpha;
                    evaluate_subset(U);
                    comp = U; 
                    break; 
                }
            }

            if (!shrunk) {
                break;
            }
        }
    }
}

void print_output(string filename) {
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
    fprintf(file, "Algorithm: CoreExact\n");
    fprintf(file, "Density: %.6f\n", density);
    fprintf(file, "Number of nodes: %d\n", (int)nodes.size());

    fprintf(file, "Nodes:\n");
    for (int v : nodes) {
        fprintf(file, "%d\n", v);
    }

    if (file != stdout) fclose(file);
}

int main(int argc, char* argv[]){
    inputFile = "testcases/Wiki-Vote.txt";
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