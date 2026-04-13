import os
import random
import argparse

DEFAULT_NUM_NODES = 10
DEFAULT_NUM_EDGES = 20
DEFAULT_SEED = None
DEFAULT_INPUTS_DIR = "inputs"
DEFAULT_INPUT_FILENAME = "input.txt"

#Zero-based indexing for nodes
def generate_graph(n, m, seed=None):
    if seed is not None:
        random.seed(seed)

    edges = set()

    while len(edges) < m:
        u = random.randint(0, n - 1)
        v = random.randint(0, n - 1)

        if u == v:
            continue

        a = min(u, v)
        b = max(u, v)

        edges.add((a, b))

    return edges


def main():
    parser = argparse.ArgumentParser(description="Random Graph Generator")
    parser.add_argument("-n", type=int, default=DEFAULT_NUM_NODES, help="Number of nodes")
    parser.add_argument("-m", type=int, default=DEFAULT_NUM_EDGES, help="Number of edges")
    parser.add_argument("--seed", type=int, default=DEFAULT_SEED, help="Random seed")

    args = parser.parse_args()

    n = args.n
    m = args.m

    if m > n * (n - 1) // 2:
        print("Too many edges for given n!")
        return

    edges = generate_graph(n, m, args.seed)

    os.makedirs(DEFAULT_INPUTS_DIR, exist_ok=True)

    output_path = os.path.join(DEFAULT_INPUTS_DIR, DEFAULT_INPUT_FILENAME)

    with open(output_path, "w") as f:
        f.write(f"{n} {m}\n")
        for u, v in edges:
            f.write(f"{u} {v}\n")

    print(f"Graph generated with {n} nodes and {m} edges → {output_path}")


if __name__ == "__main__":
    main()