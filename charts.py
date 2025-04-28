import pandas as pd
import matplotlib.pyplot as plt
import re
import numpy as np
from scipy import stats

# Read CSV data
df = pd.read_csv('test_results_2.csv')  # Replace with your actual file path

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
    ('total_edge_length', 'Total Edge Length Comparison'),
    ('time', 'Time Comparison')
]

for metric, title in metrics:
    plt.figure(figsize=(8, 6))
    
    # Get x and y values
    x = df[f'ogdf_{metric}']
    y = df[f'shape_metrics_{metric}']
    
    # Create scatter plot
    sc = plt.scatter(x, y,
                     c=df['density'], cmap='viridis_r', vmin=1, vmax=2,
                     edgecolor='k', s=35)
    
    plt.colorbar(sc, label='Density (edges/nodes)')
    plt.xlabel(f'OGDF {metric.capitalize()}')
    plt.ylabel(f'Shape Metrics {metric.capitalize()}')
    plt.title(title)
    plt.grid(True, linestyle='--', alpha=0.7)
    
    # Add diagonal line (bisector)
    min_val = min(x.min(), y.min())
    max_val = max(x.max(), y.max())
    plt.plot([min_val, max_val], [min_val, max_val], 'r--', label='y = x')
    
    # Calculate and plot best-fit line
    mask = ~np.isnan(x) & ~np.isnan(y)  # Handle NaN values if any
    slope, intercept, r_value, p_value, std_err = stats.linregress(x[mask], y[mask])
    fit_label = f'Best fit: y = {slope:.2f}x + {intercept:.2f}\n(R² = {r_value**2:.2f})'
    plt.plot(x, slope * x + intercept, 'b-', label=fit_label)
    
    plt.legend()
    plt.tight_layout()

plt.show()