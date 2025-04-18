import pandas as pd
import matplotlib.pyplot as plt
import re

# Read CSV data
df = pd.read_csv('test_results.csv')  # Replace with your actual file path

# Extract nodes and edges from graph names
def parse_graph_name(name):
    n_match = re.search(r'n(\d+)', name)
    m_match = re.search(r'm(\d+)', name)
    nodes = int(n_match.group(1)) if n_match else 0
    edges = int(m_match.group(1)) if m_match else 0
    return nodes, edges

df['nodes'] = df['graph_name'].apply(lambda x: parse_graph_name(x)[0])
df['edges'] = df['graph_name'].apply(lambda x: parse_graph_name(x)[1])
df['density'] = df['edges'] / df['nodes']

# Create comparison plots for different metrics
metrics = [
    ('crossings', 'Crossings Comparison'),
    ('bends', 'Bends Comparison'),
    ('area', 'Area Comparison'),
    ('time', 'Time Comparison')
]

for metric, title in metrics:
    plt.figure(figsize=(8, 6))
    sc = plt.scatter(df[f'ogdf_{metric}'], df[f'shape_metrics_{metric}'],
                     c=df['density'], cmap='viridis', vmin=1, vmax=2,
                     edgecolor='k', s=100)
    
    plt.colorbar(sc, label='Density (edges/nodes)')
    plt.xlabel(f'OGDF {metric.capitalize()}')
    plt.ylabel(f'Shape Metrics {metric.capitalize()}')
    plt.title(title)
    plt.grid(True, linestyle='--', alpha=0.7)
    
    # Add diagonal line
    min_val = min(df[f'ogdf_{metric}'].min(), df[f'shape_metrics_{metric}'].min())
    max_val = max(df[f'ogdf_{metric}'].max(), df[f'shape_metrics_{metric}'].max())
    plt.plot([min_val, max_val], [min_val, max_val], 'r--', label='y = x')
    plt.legend()
    
    plt.tight_layout()

plt.show()