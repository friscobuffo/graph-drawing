import pandas as pd
import numpy as np

df_1 = pd.read_csv('test_results1.csv')
df_2 = pd.read_csv('test_results2.csv')

metrics = [
    'shape_metrics_crossings',
    'shape_metrics_bends',
    'shape_metrics_area',
    'shape_metrics_total_edge_length',
    'shape_metrics_max_edge_length',
    'shape_metrics_max_bends_per_edge',
]

df_1_graph_name_to_index = {name: i for i, name in enumerate(df_1['graph_name'])}
df_2_graph_name_to_index = {name: i for i, name in enumerate(df_2['graph_name'])}

all_graph_names = set(df_1['graph_name']).union(set(df_2['graph_name']))

for metric in metrics:
    total_mismatches = 0
    total_comparisons = 0

    for graph_name in all_graph_names:
        if graph_name not in df_1_graph_name_to_index or graph_name not in df_2_graph_name_to_index:
            continue
        row_1 = df_1.iloc[df_1_graph_name_to_index[graph_name]]
        row_2 = df_2.iloc[df_2_graph_name_to_index[graph_name]]
        if row_1[metric] != row_2[metric]:
            total_mismatches += 1
        total_comparisons += 1
    print(f"Metric: {metric}")
    print(f"Mismatch ratio: {total_mismatches/total_comparisons}")

time_diffs = []
for _, row in df_1.iterrows():
    graph_name = row['graph_name']
    val1 = row['shape_metrics_time']
    val2_series = df_2.loc[df_2['graph_name'] == graph_name, 'shape_metrics_time']
    if not val2_series.empty:
        val2 = val2_series.values[0]
        diff = abs(val1 - val2)
        time_diffs.append(diff)

if time_diffs:
    mean_diff = np.mean(time_diffs)
    max_diff = np.max(time_diffs)
    print(f"Mean absolute difference for 'shape_metrics_time': {mean_diff}")
    print(f"Max absolute difference for 'shape_metrics_time': {max_diff}")
else:
    print("No matching 'graph_name' entries found for 'shape_metrics_time'")