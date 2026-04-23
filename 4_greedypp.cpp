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

// Zero-based indexing for nodes
void read_input(string filename)
{

    FILE *file = fopen(filename.c_str(), "r");
    if (!file)
    {
        perror("Error opening file");
        exit(1);
    }

    char line[256];
    int u, v;
    int maxNode = -1;

    set<pair<int, int>> edges;

    while (fgets(line, sizeof(line), file))
    {

        if (line[0] == '#')
            continue;

        if (sscanf(line, "%d %d", &u, &v) == 2)
        {

            if (u == v)
                continue;

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

    for (auto &e : edges)
    {
        adj[e.first].push_back(e.second);
        adj[e.second].push_back(e.first);
    }
}

void algorithm()
{
    density = 0.0;
    nodes.clear();

    int T = 20; // better iteration count for Greedy++

    vector<int> load(n, 0);
    vector<int> best_nodes;
    double best_density = (double)m / n;

    // initialize best_nodes as all nodes (full graph)
    for (int i = 0; i < n; i++)
        best_nodes.push_back(i);

    for (int iter = 0; iter < T; iter++)
    {

        // Copy graph state
        vector<set<int>> H(n);
        vector<bool> alive(n, true);

        for (int u = 0; u < n; u++)
        {
            for (int v : adj[u])
            {
                H[u].insert(v);
            }
        }

        int remaining_nodes = n;
        int remaining_edges = m;

        // Min heap: (load + degree, node)
        using pii = pair<int, int>;
        priority_queue<pii, vector<pii>, greater<pii>> pq;

        vector<int> degree(n, 0);
        for (int u = 0; u < n; u++)
        {
            degree[u] = H[u].size();
            pq.push({load[u] + degree[u], u});
        }

        vector<int> current_nodes;
        for (int i = 0; i < n; i++)
            current_nodes.push_back(i);

        while (!pq.empty() && remaining_nodes > 0)
        {
            auto [val, u] = pq.top();
            pq.pop();

            if (!alive[u])
                continue;
            if (val != load[u] + degree[u])
                continue;

            // Update load
            load[u] += degree[u];

            // Remove u
            alive[u] = false;
            remaining_nodes--;

            for (int v : H[u])
            {
                if (alive[v])
                {
                    H[v].erase(u);
                    remaining_edges--;
                    degree[v]--;
                    pq.push({load[v] + degree[v], v});
                }
            }

            H[u].clear();

            // Build current node list
            current_nodes.clear();
            for (int i = 0; i < n; i++)
            {
                if (alive[i])
                    current_nodes.push_back(i);
            }

            if (remaining_nodes > 0)
            {
                int edge_count = 0;
                for (int i = 0; i < n; i++)
                {
                    if (alive[i])
                        edge_count += degree[i];
                }
                edge_count /= 2;

                double curr_density = (double)edge_count / remaining_nodes;
                if (curr_density > best_density)
                {
                    best_density = curr_density;
                    best_nodes = current_nodes;
                }
            }
        }
    }

    density = best_density;
    nodes = best_nodes;
}

void print_output(string filename)
{
    FILE *file;

    if (filename == "stdout")
    {
        file = stdout;
    }
    else
    {
        file = fopen(filename.c_str(), "w");
        if (!file)
        {
            perror("Error opening output file");
            exit(1);
        }
    }

    sort(nodes.begin(), nodes.end());

    fprintf(file, "Algorithm: Greedy++\n");
    fprintf(file, "Density: %.6f\n", density);
    fprintf(file, "Number of nodes: %d\n", (int)nodes.size());

    fprintf(file, "Nodes:\n");
    for (int v : nodes)
    {
        fprintf(file, "%d\n", v);
    }

    if (file != stdout)
        fclose(file);
}

int main(int argc, char *argv[])
{
    inputFile = "testcases/Wiki-Vote.txt";
    outputFile = "stdout";

    if (argc == 2)
    {
        inputFile = argv[1];
    }

    if (argc == 3)
    {
        inputFile = argv[1];
        outputFile = argv[2];
    }

    system("mkdir -p outputs");

    read_input(inputFile);

    algorithm();

    print_output(outputFile);

    return 0;
}