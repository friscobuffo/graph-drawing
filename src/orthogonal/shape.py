






def _model_solution_to_shape(graph: Graph, solution, is_edge_up_variable, is_edge_down_variable, is_edge_right_variable, is_edge_left_variable) -> Shape:
    variable_values = dict()
    for i in range(len(solution)):
        var = solution[i]
        if var > 0:
            variable_values[var] = True
        else:
            variable_values[-var] = False
    shape = Shape(graph)
    for i in range(graph.size()):
        for j in graph.get_neighbors(i):
            up = _get_variables(is_edge_up_variable, i, j)
            down = _get_variables(is_edge_down_variable, i, j)
            right = _get_variables(is_edge_right_variable, i, j)
            left = _get_variables(is_edge_left_variable, i, j)
            assert variable_values[up] + variable_values[down] + variable_values[right] + variable_values[left] == 1
            if variable_values[up]:
                shape[(i, j)] = "up"
            elif variable_values[down]:
                shape[(i, j)] = "down"
            elif variable_values[right]:
                shape[(i, j)] = "right"
            elif variable_values[left]:
                shape[(i, j)] = "left"
            else: assert False
    return shape

from time import perf_counter


