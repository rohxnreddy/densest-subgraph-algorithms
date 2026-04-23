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

// Global variables to store graph and results
string inputFile, outputFile;
int n, m;
vector<vector<int>> adj;
double density = 0.0;
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
    if (pushed == 0 || v == t) return pushed;
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

struct Triangle {
    int u, v, w;
};

void algorithm() {
    if (n == 0) return;

    // 1. Find all triangles
    vector<Triangle> triangles;
    if (n > 0) {
        vector<int> mark(n, 0);
        int stamp = 1;
        for (int u_node = 0; u_node < n; ++u_node) {
            for (int v : adj[u_node]) mark[v] = stamp;
            for (int v_node : adj[u_node]) {
                if (v_node <= u_node) continue;
                for (int w_node : adj[v_node]) {
                    if (w_node <= v_node) continue;
                    if (mark[w_node] == stamp) {
                        triangles.push_back({u_node, v_node, w_node});
                    }
                }
            }
            ++stamp;
            if (stamp == 0x3fffffff) {
                fill(mark.begin(), mark.end(), 0);
                stamp = 1;
            }
        }
    }

    double l = 0, u = triangles.size();
    double EPS = 1.0 / (1.0 * n * (n + 1));

    // Calculate total iterations for progress bar
    // Since it's binary search: log2((u-l)/EPS)
    int total_iters = (u <= l) ? 1 : (int)ceil(log2((u - l) / EPS));
    int current_iter = 0;
    int update_every = max(1, total_iters / 20); // Update every 5%

    while (u - l > EPS) {
        current_iter++;
        double alpha = (l + u) / 2.0;
        int S_node = 0;
        int T_node = triangles.size() + n + 1;

        flow_adj.assign(T_node + 1, vector<Edge>());
        level_arr.assign(T_node + 1, -1);
        ptr_arr.assign(T_node + 1, 0);

        for (int i = 0; i < (int)triangles.size(); ++i) {
            add_flow_edge(S_node, i + 1, 1.0);
            add_flow_edge(i + 1, triangles[i].u + (int)triangles.size() + 1, 1e15);
            add_flow_edge(i + 1, triangles[i].v + (int)triangles.size() + 1, 1e15);
            add_flow_edge(i + 1, triangles[i].w + (int)triangles.size() + 1, 1e15);
        }

        for (int i = 0; i < n; ++i) {
            add_flow_edge(i + (int)triangles.size() + 1, T_node, alpha);
        }

        dinic(S_node, T_node);

        vector<int> current_S;
        vector<char> visited(T_node + 1, 0);
        queue<int> q;
        q.push(S_node);
        visited[S_node] = 1;

        while (!q.empty()) {
            int curr = q.front();
            q.pop();
            if (curr > (int)triangles.size() && curr < T_node) {
                current_S.push_back(curr - (int)triangles.size() - 1);
            }
            for (auto &edge : flow_adj[curr]) {
                if (edge.capacity - edge.flow > 1e-9 && !visited[edge.to]) {
                    visited[edge.to] = true;
                    q.push(edge.to);
                }
            }
        }

        if (!current_S.empty()) {
            l = alpha;
            density = alpha;
            nodes = current_S;
        } else {
            u = alpha;
        }

        // Fixed progress bar call
        if (current_iter == 1 || current_iter % update_every == 0 || u - l <= EPS) {
            print_progress_bar(current_iter, total_iters);
        }
    }
}

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

    fprintf(file, "Algorithm: Exact\n");
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
