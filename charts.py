import pandas as pd
import matplotlib.pyplot as plt
import re
import numpy as np
from scipy import stats
import os

# Create a directory to save the plots if it doesn't exist
output_dir = 'plot_results'
os.makedirs(output_dir, exist_ok=True)

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
    ('total_edge_length', 'Total Edge Length Comparison'),
    ('time', 'Time Comparison'),
    ('max_edge_length', 'Max Edge Length Comparison'),
    ('max_bends_per_edge', 'Max Bends per Edge Comparison'),
    ('edge_length_stddev', 'Edge Length Std Dev Comparison'),
    ('bends_stddev', 'Bends Std Dev Comparison')
]

for metric, title in metrics:
    plt.figure(figsize=(8, 6))
    
    # Get x and y values
    x = df[f'ogdf_{metric}']
    y = df[f'shape_metrics_{metric}']
    
    # Create scatter plot
    sc = plt.scatter(x, y,
                     c=df['density'], cmap='viridis_r', 
                     vmin=df['density'].min(), vmax=df['density'].max(),
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
    
    # Save the figure instead of showing it
    filename = f"{metric}_comparison.png"
    filepath = os.path.join(output_dir, filename)
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    plt.close()  # Close the figure to free memory

print(f"All scatter plots have been saved to the '{output_dir}' directory.")

# Histogram comparison for max_bends_per_edge
plt.figure(figsize=(12, 6))

# Get unique values and counts
all_values = np.sort(pd.concat([
    df['ogdf_max_bends_per_edge'].dropna(),
    df['shape_metrics_max_bends_per_edge'].dropna()
]).unique())

# Prepare counts for each value
ogdf_counts = [(df['ogdf_max_bends_per_edge'] == val).sum() for val in all_values]
shape_counts = [(df['shape_metrics_max_bends_per_edge'] == val).sum() for val in all_values]

# Set up bar positions
x_pos = np.arange(len(all_values))
bar_width = 0.35

# Plot side-by-side bars
plt.bar(x_pos - bar_width/2, shape_counts, width=bar_width, 
        color='blue', label='Shape Metrics', edgecolor='black')
plt.bar(x_pos + bar_width/2, ogdf_counts, width=bar_width, 
        color='red', label='OGDF', edgecolor='black')

# Customize the plot
plt.xlabel('Max Bends per Edge')
plt.ylabel('Count')
plt.title('Side-by-Side Histogram of Max Bends per Edge Distribution')
plt.xticks(x_pos, all_values)
plt.grid(True, axis='y', linestyle='--', alpha=0.7)
plt.legend()

plt.tight_layout()

# Save the plot
filename = "max_bends_per_edge_histogram.png"
filepath = os.path.join(output_dir, filename)
plt.savefig(filepath, dpi=300, bbox_inches='tight')
plt.close()

print(f"Maxx bends per edge histogram plot saved to {filepath}")