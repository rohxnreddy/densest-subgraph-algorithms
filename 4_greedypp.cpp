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

static void print_progress_bar(long long current, long long total, int width = 40)
{
    if (total <= 0)
        return;
    if (current < 0)
        current = 0;
    if (current > total)
        current = total;

    double ratio = (total == 0) ? 1.0 : (double)current / (double)total;
    int filled = (int)(ratio * width);
    if (filled < 0)
        filled = 0;
    if (filled > width)
        filled = width;

    cerr << '\r' << '[';
    for (int i = 0; i < width; ++i)
    {
        cerr << (i < filled ? '#' : '-');
    }
    int pct = (int)(ratio * 100.0);
    if (pct < 0)
        pct = 0;
    if (pct > 100)
        pct = 100;
    cerr << "] " << pct << "% (" << current << "/" << total << ')' << flush;

    if (current >= total)
        cerr << '\n';
}

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

    int T = 5; //20 or 50

    vector<int> load(n, 0);
    vector<int> best_nodes;
    double best_density = n > 0 ? (double)m / n : 0.0;

    for (int i = 0; i < n; i++)
        best_nodes.push_back(i);

    const long long total_steps = (long long)T * (long long)n;
    const long long update_every = max(1LL, total_steps / 200); // ~200 updates max
    long long completed_steps = 0;

    // Pre-allocate structures outside the loop to avoid redundant allocations
    vector<bool> alive(n);
    vector<int> degree(n);
    vector<int> key(n);
    vector<int> removal_order;
    removal_order.reserve(n);

    // Doubly linked list for O(1) bucket sort
    vector<int> head;
    vector<int> prev_node(n);
    vector<int> next_node(n);

    for (int iter = 0; iter < T; iter++)
    {
        int remaining_nodes = n;
        int remaining_edges = m;
        removal_order.clear();

        int iter_max_key = 0;
        for (int u = 0; u < n; u++)
        {
            alive[u] = true;
            degree[u] = adj[u].size();
            key[u] = load[u] + degree[u];
            if (key[u] > iter_max_key)
            {
                iter_max_key = key[u];
            }
        }

        if (head.size() <= iter_max_key)
        {
            head.resize(iter_max_key + 1, -1);
        }

        // Clear only the used portion of the bucket array
        fill(head.begin(), head.begin() + iter_max_key + 1, -1);

        for (int u = 0; u < n; u++)
        {
            int k = key[u];
            next_node[u] = head[k];
            if (head[k] != -1)
            {
                prev_node[head[k]] = u;
            }
            head[k] = u;
            prev_node[u] = -1;
        }

        int min_key = 0;
        double iter_best_density = -1.0;
        int iter_best_step = -1;

        while (remaining_nodes > 0)
        {
            // Find the minimum non-empty bucket
            while (min_key <= iter_max_key && head[min_key] == -1)
            {
                min_key++;
            }

            int u = head[min_key];

            // Pop u from the doubly linked list
            head[min_key] = next_node[u];
            if (head[min_key] != -1)
            {
                prev_node[head[min_key]] = -1;
            }

            alive[u] = false;
            load[u] += degree[u];
            removal_order.push_back(u);
            remaining_nodes--;
            completed_steps++;

            for (int v : adj[u])
            {
                if (alive[v])
                {
                    remaining_edges--;

                    int k = key[v];
                    // Remove v from its current bucket
                    if (prev_node[v] != -1)
                    {
                        next_node[prev_node[v]] = next_node[v];
                    }
                    else
                    {
                        head[k] = next_node[v];
                    }
                    if (next_node[v] != -1)
                    {
                        prev_node[next_node[v]] = prev_node[v];
                    }

                    key[v]--;
                    k--;
                    degree[v]--;

                    // Insert v into the new bucket
                    next_node[v] = head[k];
                    if (head[k] != -1)
                    {
                        prev_node[head[k]] = v;
                    }
                    head[k] = v;
                    prev_node[v] = -1;

                    // Update min_key if needed
                    if (k < min_key)
                    {
                        min_key = k;
                    }
                }
            }

            if (remaining_nodes > 0)
            {
                double curr_density = (double)remaining_edges / remaining_nodes;
                if (curr_density > iter_best_density)
                {
                    iter_best_density = curr_density;
                    iter_best_step = removal_order.size();
                }
            }

            if (total_steps >= 200 && (completed_steps == 1 || completed_steps % update_every == 0 || completed_steps == total_steps))
            {
                print_progress_bar(completed_steps, total_steps);
            }
        }

        if (iter_best_density > best_density)
        {
            best_density = iter_best_density;
            best_nodes.clear();
            if (iter_best_step != -1)
            {
                best_nodes.insert(best_nodes.end(), removal_order.begin() + iter_best_step, removal_order.end());
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

    fprintf(file, "Algorithm: Greedy++\n");
    fprintf(file, "Density: %.6f\n", density);
    fprintf(file, "Number of nodes: %d\n", (int)nodes.size());
    fprintf(file, "Number of edges: %lld\n", internal_edges); // New Line

    fprintf(file, "Nodes: ");
    for (int v : nodes) {
        fprintf(file, " %d ,", v);
    }
    fprintf(file, "\n");

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
