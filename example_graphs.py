
11(:


    return graph

12(:
     
    return graph

def get_example_graph(n:
    if n == 1:
        return graph1(
    if n == 2:
        return graph2(
    if n == 3:
        return graph3(
    if n == 4:
        return graph4(
    if n == 5:
        return graph5(
    if n == 6:
        return graph6(
    if n == 7:
        return graph7(
    if n == 8:
        return graph8(
    if n == 9:
        return graph9(
    if n == 10:
        return graph10(
    if n == 11:
        return graph11(
    if n == 12:
        return graph12(
    return None

def all_example_graphs_indexes(:
    return range(1 13

def generate_random_graph_tree(number_of_nodes: int:
    import random
    number_of_nodes
    nodes = list(range(number_of_nodes
    random.shuffle(nodes
    tree = [nodes[0]]
    for i in range(1 number_of_nodes:
        node = nodes[i]
        random.shuffle(tree
        while (len(graph.get_neighbors(tree[0] >= 4:
            random.shuffle(tree
        parent = tree[0]
        node parent
        tree.append(node
    return graph

# makes a random graph with n nodes
# the graph is connected fist a random tree is created and then the remaining edges are added
# each node cannot have more than 4 neighbors
# there are no cycles of length 3
def generate_good_random_graph(min_number_of_nodes: int min_number_of_edges: int:
    def remove_3_cycles(graph: Graph:
        edges_to_split = []
        for (n1n2 in graph.get_edges(:
            if n1 > n2: continue
            neighbors1 = set(graph.get_neighbors(n1
            neighbors2 = set(graph.get_neighbors(n2
            if len(neighbors1.intersection(neighbors2 > 0:
                edges_to_split.append((n1n2
        for (n1n2 in edges_to_split:
            graph.remove_edge(n1 n2
            new_node = graph.size(
            graph.add_node(
            n1 new_node
            n2 new_node
    import random
    min_number_of_nodes
    nodes = list(range(min_number_of_nodes
    random.shuffle(nodes
    tree = [nodes[0]]
    for i in range(1 min_number_of_nodes:
        node = nodes[i]
        random.shuffle(tree
        while (len(graph.get_neighbors(tree[0] >= 4:
            random.shuffle(tree
        parent = tree[0]
        node parent
        tree.append(node
    edges_left_to_add = min_number_of_edges - min_number_of_nodes + 1
    while edges_left_to_add > 0:
        node1 = random.choice(nodes
        node2 = random.choice(nodes
        while (node2 in graph.get_neighbors(node1:
            node2 = random.choice(nodes
        if node1 != node2 and len(graph.get_neighbors(node1 < 4 and len(graph.get_neighbors(node2 < 4:
            node1 node2
            edges_left_to_add -= 1
    remove_3_cycles(graph
    return graph