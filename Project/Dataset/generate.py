import random

def generate_graph(n, m, path_to_file):
    ''' 
        where n is the number of vertices and m is the number of edges:
    '''
    weight_min = 1
    weight_max = 100

    # Ensure edges are unique and there's no self-loop
    edges = set()
    while len(edges) < m:
        a = random.randint(0, n-1)
        b = random.randint(0, n-1)
        if a != b: # for ensuring no self loops
            edge = (a, b) 
            edges.add(edge)

    # Assign random weights to each edge
    graph = [(a, b, random.randint(weight_min, weight_max)) for a, b in edges]

    # Write to file
    with open(path_to_file, "w") as file:
        file.write(f"{n} {m}\n")
        for a, b, w in graph:
            file.write(f"{a} {b} {w}\n")


def generate_test_file():
    path_to_file = f"./Dataset/test.txt"
    n = 1000
    m = int(1e5)

    generate_graph(n, m, path_to_file)


pairs = [(4*1e3, 1e5), (8*1e3, 1e7), (3*1e4, 1e7)]
for i in range(3):
    path_to_file = f"./Dataset/in{i}.txt"
    n = int(pairs[i][0])
    m = int(pairs[i][1])
    
    generate_graph(n, m, path_to_file)
    print(f"Graph {i} generated successfully!")