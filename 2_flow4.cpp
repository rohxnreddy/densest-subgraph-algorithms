#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <limits>
#include <cstring>
#include <algorithm>
#include <set>
#include <cmath>
#include <fstream>
#include <sstream>

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

    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error opening file\n";
        exit(1);
    }

    string line;
    bool header_read = false;
    set<pair<int,int>> edges;
    int max_node = -1;

    while (getline(file, line)) {
        if (line.empty() || line[0] == '#' || line[0] == '%') continue;

        stringstream ss(line);
        int u, v;
        if (ss >> u >> v) {
            if (!header_read) {
                n = u;
                m = v;
                header_read = true;
            } else {
                if (u == v) continue; 
                int a = min(u, v);
                int b = max(u, v);
                edges.insert({a, b});
                max_node = max({max_node, a, b});
            }
        }
    }

    file.close();

    if (!header_read) {
        cout << "Invalid input format\n";
        exit(1);
    }

    // In case the file didn't have an explicit n, m header and the first line was actually an edge
    n = max(n, max_node + 1);
    m = edges.size();

    adj.assign(n, vector<int>());

    for (auto &e : edges) {
        adj[e.first].push_back(e.second);
        adj[e.second].push_back(e.first);
    }
}

// Structs for Max Flow (Dinic's Algorithm)
struct FlowEdge {
    int to;
    double cap, flow;
    int rev;
};

struct Dinic {
    int V;
    vector<vector<FlowEdge>> dinic_adj;
    vector<int> level, ptr;

    Dinic(int V) : V(V), dinic_adj(V), level(V), ptr(V) {}

    void add_edge(int from, int to, double cap) {
        dinic_adj[from].push_back({to, cap, 0.0, (int)dinic_adj[to].size()});
        dinic_adj[to].push_back({from, 0.0, 0.0, (int)dinic_adj[from].size() - 1});
    }

    bool bfs(int s, int t) {
        fill(level.begin(), level.end(), -1);
        level[s] = 0;
        queue<int> q;
        q.push(s);
        while (!q.empty()) {
            int v = q.front();
            q.pop();
            for (auto& edge : dinic_adj[v]) {
                if (edge.cap - edge.flow > 1e-9 && level[edge.to] == -1) {
                    level[edge.to] = level[v] + 1;
                    q.push(edge.to);
                }
            }
        }
        return level[t] != -1;
    }

    double dfs(int v, int t, double pushed) {
        if (pushed < 1e-9) return 0;
        if (v == t) return pushed;
        for (int& cid = ptr[v]; cid < dinic_adj[v].size(); ++cid) {
            auto& edge = dinic_adj[v][cid];
            int tr = edge.to;
            if (level[v] + 1 != level[tr] || edge.cap - edge.flow < 1e-9) continue;
            double push = dfs(tr, t, min(pushed, edge.cap - edge.flow));
            if (push < 1e-9) continue;
            edge.flow += push;
            dinic_adj[tr][edge.rev].flow -= push;
            return push;
        }
        return 0;
    }

    double max_flow(int s, int t) {
        double flow = 0;
        while (bfs(s, t)) {
            fill(ptr.begin(), ptr.end(), 0);
            while (double pushed = dfs(s, t, 1e15)) {
                flow += pushed;
            }
        }
        return flow;
    }

    vector<int> get_S(int s) {
        vector<int> S;
        vector<bool> vis(V, false);
        queue<int> q;
        q.push(s);
        vis[s] = true;
        while (!q.empty()) {
            int v = q.front();
            q.pop();
            S.push_back(v);
            for (auto& edge : dinic_adj[v]) {
                if (edge.cap - edge.flow > 1e-9 && !vis[edge.to]) {
                    vis[edge.to] = true;
                    q.push(edge.to);
                }
            }
        }
        return S;
    }
};

struct Triangle {
    int u, v, w;
    bool active;
};

void algorithm() {
    density = 0.0;
    nodes.clear();

    if (n < 3) return;

    // 1. Efficient Triangle Enumeration
    vector<Triangle> triangles;
    vector<vector<int>> node_tris(n);

    vector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    sort(order.begin(), order.end(), [&](int a, int b) {
        return adj[a].size() < adj[b].size() || (adj[a].size() == adj[b].size() && a < b);
    });

    vector<int> rank(n);
    for (int i = 0; i < n; ++i) rank[order[i]] = i;

    vector<vector<int>> dir_adj(n);
    for (int u = 0; u < n; ++u) {
        for (int v : adj[u]) {
            if (rank[u] < rank[v]) dir_adj[u].push_back(v);
        }
    }

    vector<int> mark(n, -1);
    for (int u = 0; u < n; ++u) {
        for (int v : dir_adj[u]) mark[v] = u;
        for (int v : dir_adj[u]) {
            for (int w : dir_adj[v]) {
                if (mark[w] == u) {
                    int t_id = triangles.size();
                    triangles.push_back({u, v, w, true});
                    node_tris[u].push_back(t_id);
                    node_tris[v].push_back(t_id);
                    node_tris[w].push_back(t_id);
                }
            }
        }
    }

    // 2. Core Decomposition (Algorithm 3)
    vector<int> tri_deg(n);
    for (int i = 0; i < n; ++i) tri_deg[i] = node_tris[i].size();

    set<pair<int, int>> pq;
    for (int i = 0; i < n; ++i) pq.insert({tri_deg[i], i});

    vector<bool> active_node(n, true);
    int curr_n = n;
    int curr_tri = triangles.size();

    double rho_prime = curr_n > 0 ? (double)curr_tri / curr_n : 0.0;
    int max_rho_iter = 0;
    int current_iter = 0;
    vector<int> peel_order;
    vector<int> core(n, 0);
    int k_max = 0;

    const int total_steps = n;
    const int update_every = max(1, total_steps / 200); // ~200 updates max

    while (!pq.empty()) {
        double current_rho = curr_n > 0 ? (double)curr_tri / curr_n : 0.0;
        if (current_rho > rho_prime) {
            rho_prime = current_rho;
            max_rho_iter = current_iter;
        }

        auto it = pq.begin();
        int d = it->first;
        int u = it->second;
        pq.erase(it);

        active_node[u] = false;
        curr_n--;
        k_max = max(k_max, d);
        core[u] = k_max;
        peel_order.push_back(u);
        current_iter++;

        if (total_steps >= 200 && (current_iter == 1 || current_iter % update_every == 0 || current_iter == total_steps)) {
            print_progress_bar(current_iter, total_steps);
        }

        for (int t_id : node_tris[u]) {
            if (triangles[t_id].active) {
                triangles[t_id].active = false;
                curr_tri--;
                int v1 = triangles[t_id].u, v2 = triangles[t_id].v, v3 = triangles[t_id].w;
                int others[2];
                int idx = 0;
                if (v1 != u) others[idx++] = v1;
                if (v2 != u) others[idx++] = v2;
                if (v3 != u) others[idx++] = v3;

                for (int i = 0; i < 2; ++i) {
                    int w = others[i];
                    if (active_node[w]) {
                        pq.erase({tri_deg[w], w});
                        tri_deg[w]--;
                        pq.insert({tri_deg[w], w});
                    }
                }
            }
        }
    }

    // Set best baseline from core decomposition
    vector<bool> in_best(n, true);
    for (int i = 0; i < max_rho_iter; ++i) in_best[peel_order[i]] = false;
    for (int i = 0; i < n; ++i) if (in_best[i]) nodes.push_back(i);
    density = rho_prime;

    auto density_of = [&](const vector<int>& U) -> double {
        if (U.empty()) return 0.0;
        vector<bool> in_U(n, false);
        for (int v : U) in_U[v] = true;
        int tris = 0;
        for (int u : U) {
            for (int t_id : node_tris[u]) {
                int v1 = triangles[t_id].u, v2 = triangles[t_id].v, v3 = triangles[t_id].w;
                if (u == min({v1, v2, v3}) && in_U[v1] && in_U[v2] && in_U[v3]) tris++;
            }
        }
        return (double)tris / U.size();
    };

    // 3. Extract Core Components (k_prime_prime = ceil(rho_prime))
    int k_double_prime = ceil(rho_prime);
    vector<bool> in_core(n, false);
    vector<int> V_core;
    for (int i = 0; i < n; ++i) {
        if (core[i] >= k_double_prime) {
            in_core[i] = true;
            V_core.push_back(i);
        }
    }

    vector<bool> visited(n, false);
    vector<vector<int>> components;
    for (int v : V_core) {
        if (!visited[v]) {
            vector<int> comp;
            queue<int> q;
            q.push(v);
            visited[v] = true;
            while (!q.empty()) {
                int curr = q.front();
                q.pop();
                comp.push_back(curr);
                for (int nxt : adj[curr]) {
                    if (in_core[nxt] && !visited[nxt]) {
                        visited[nxt] = true;
                        q.push(nxt);
                    }
                }
            }
            components.push_back(comp);
        }
    }

    // 4. Algorithm 4 CoreExact Flow execution
    double global_l = rho_prime;
    double global_u = k_max;

    for (auto& C : components) {
        double l = global_l;
        double u = global_u;

        double epsilon = max(1e-9, 1.0 / ((double)C.size() * (C.size() - 1)));

        while (u - l >= epsilon) {
            int required_core = ceil(l);
            vector<int> curr_C;
            for (int v : C) {
                if (core[v] >= required_core) curr_C.push_back(v);
            }

            if (curr_C.empty()) break;

            int num_nodes = curr_C.size();
            vector<bool> in_C(n, false);
            vector<int> C_idx(n, -1);
            for (int i = 0; i < num_nodes; ++i) {
                in_C[curr_C[i]] = true;
                C_idx[curr_C[i]] = i + 1; // 1-based indexing for Dinic graph
            }

            vector<pair<int, int>> C_edges;
            for (int u : curr_C) {
                for (int v : adj[u]) {
                    if (u < v && in_C[v]) C_edges.push_back({u, v});
                }
            }
            sort(C_edges.begin(), C_edges.end());
            int num_edges = C_edges.size();

            auto get_edge_idx = [&](int x, int y) {
                if (x > y) swap(x, y);
                auto it = lower_bound(C_edges.begin(), C_edges.end(), make_pair(x, y));
                return (int)distance(C_edges.begin(), it);
            };

            vector<int> local_deg(n, 0);
            struct TriEdgeInfo { int e1, e2, e3, v1, v2, v3; };
            vector<TriEdgeInfo> C_tris_edges;

            for (int u : curr_C) {
                for (int t_id : node_tris[u]) {
                    int v1 = triangles[t_id].u, v2 = triangles[t_id].v, v3 = triangles[t_id].w;
                    if (u == min({v1, v2, v3}) && in_C[v1] && in_C[v2] && in_C[v3]) {
                        local_deg[v1]++; local_deg[v2]++; local_deg[v3]++;
                        int e1 = get_edge_idx(v1, v2);
                        int e2 = get_edge_idx(v2, v3);
                        int e3 = get_edge_idx(v1, v3);
                        C_tris_edges.push_back({e1, e2, e3, v1, v2, v3});
                    }
                }
            }

            double alpha = (l + u) / 2.0;
            
            // source = 0, node layer = 1..num_nodes, edge layer = num_nodes+1..num_nodes+num_edges, sink = end
            int s = 0, t = num_nodes + num_edges + 1;
            Dinic dinic(t + 1);

            for (int i = 0; i < num_nodes; ++i) {
                int u_node = curr_C[i];
                if (local_deg[u_node] > 0)
                    dinic.add_edge(s, i + 1, local_deg[u_node]);
                // edge to sink based on density alpha * size of triangle (h=3) -> 3.0 * alpha
                dinic.add_edge(i + 1, t, 3.0 * alpha);
            }

            const double INF = 1e12;
            for (int j = 0; j < num_edges; ++j) {
                int u_node = C_edges[j].first;
                int v_node = C_edges[j].second;
                int psi_node = num_nodes + j + 1;
                // capacity infinite from edge node to its vertex nodes
                dinic.add_edge(psi_node, C_idx[u_node], INF);
                dinic.add_edge(psi_node, C_idx[v_node], INF);
            }

            for (auto& tri : C_tris_edges) {
                // capacity 1 from vertex to opposite edge
                dinic.add_edge(C_idx[tri.v1], num_nodes + tri.e2 + 1, 1.0);
                dinic.add_edge(C_idx[tri.v2], num_nodes + tri.e3 + 1, 1.0);
                dinic.add_edge(C_idx[tri.v3], num_nodes + tri.e1 + 1, 1.0);
            }

            dinic.max_flow(s, t);
            vector<int> S = dinic.get_S(s);

            if (S.size() == 1) { // S == {s}
                u = alpha;
            } else {
                l = alpha;
                global_l = max(global_l, alpha);
                
                vector<int> U;
                for (int id : S) {
                    if (id > 0 && id <= num_nodes) {
                        U.push_back(curr_C[id - 1]);
                    }
                }
                
                double curr_rho = density_of(U);
                if (curr_rho > density) {
                    density = curr_rho;
                    nodes = U;
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

    fprintf(file, "Algorithm: CoreExact\n");
    fprintf(file, "Density: %.6f\n", density);
    fprintf(file, "Number of nodes: %d\n", (int)nodes.size());

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