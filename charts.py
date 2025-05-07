import pandas as pd
import matplotlib.pyplot as plt
import re
import numpy as np
import os
import math

df = pd.read_csv('test_results.csv')

max_columns_to_check = ['shape_metrics_time',
                        'shape_metrics_number_useless_bends',
                        'shape_metrics_number_added_cycles',
                        'ogdf_max_bends_per_edge',
                        'shape_metrics_max_bends_per_edge']

for col in max_columns_to_check:
    max_time_graph = df.loc[df[col].idxmax(), 'graph_name']
    print(f"graph with highest {col}: [{max_time_graph}] value [{df[col].max()}]")

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

def make_scatter_comparison(df, x_row_name, y_row_name, x_label, y_label, title, filename, output_dir, add_diagonal=True):
    plt.figure(figsize=(8, 6))

    # Get x and y values
    x = df[x_row_name]
    y = df[y_row_name]
    
    # Create scatter plot
    sc = plt.scatter(x, y,
                    c=df['density'], cmap='viridis_r', 
                    vmin=df['density'].min(), vmax=df['density'].max(),
                    edgecolor='k', s=35)
    plt.colorbar(sc, label='Density (edges/nodes)')
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.title(title)
    plt.grid(True, linestyle='--', alpha=0.7)
    
    if (add_diagonal): # Add diagonal line (bisector)
        min_val = min(x.min(), y.min())
        max_val = max(x.max(), y.max())
        plt.plot([min_val, max_val], [min_val, max_val], linestyle='--', color='gray', label='y = x')
    
    # Calculate and plot best-fit line
    mask = ~np.isnan(x) & ~np.isnan(y)  # Handle NaN values if any

    # Fit a 2nd-degree (quadratic) polynomial
    coeffs = np.polyfit(x[mask], y[mask], deg=2)
    poly = np.poly1d(coeffs)

    # Generate smooth curve points
    x_fit = np.linspace(min(x), max(x), 500)
    y_fit = poly(x_fit)

    # R² computation (manually for nonlinear)
    y_pred = poly(x[mask])
    ss_res = np.sum((y[mask] - y_pred) ** 2)
    ss_tot = np.sum((y[mask] - np.mean(y[mask])) ** 2)
    r2 = 1 - (ss_res / ss_tot)

    # Plot
    fit_label = f'Best fit: y = {coeffs[0]:.2f}x² + {coeffs[1]:.2f}x + {coeffs[2]:.2f}\n(R² = {r2:.2f})'
    plt.plot(x_fit, y_fit, 'r--', label=fit_label)

    plt.legend()
    plt.tight_layout()
    filepath = os.path.join(output_dir, filename)
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    plt.close()

def make_scatter_comparisons(df, output_dir):
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
    os.makedirs(output_dir, exist_ok=True)
    for metric, title in metrics:
        x_row_name = f'ogdf_{metric}'
        y_row_name = f'shape_metrics_{metric}'
        x_label = f'OGDF {metric.capitalize()}'
        y_label = f'Shape Metrics {metric.capitalize()}'
        filename = f"{metric}_comparison.png"
        make_scatter_comparison(df, x_row_name, y_row_name, x_label, y_label, title, filename, output_dir)
    print(f"All scatter plots have been saved to the '{output_dir}' directory.")

def make_histogram_comparison(x_row_name, y_row_name, x_label, y_label, title, filename, output_dir):
    # Histogram comparison for max_bends_per_edge
    plt.figure(figsize=(12, 6))

    # Get unique values and counts
    all_values = np.sort(pd.concat([
        df[x_row_name].dropna(),
        df[y_row_name].dropna()
    ]).unique())

    # Prepare counts for each value
    ogdf_counts = [(df[x_row_name] == val).sum() for val in all_values]
    shape_counts = [(df[y_row_name] == val).sum() for val in all_values]

    # Set up bar positions
    x_pos = np.arange(len(all_values))
    bar_width = 0.35

    # Plot side-by-side bars
    plt.bar(x_pos - bar_width/2, shape_counts, width=bar_width, 
            color='blue', label='Shape Metrics', edgecolor='black')
    plt.bar(x_pos + bar_width/2, ogdf_counts, width=bar_width, 
            color='red', label='OGDF', edgecolor='black')

    # Customize the plot
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.title(title)
    plt.xticks(x_pos, all_values)
    plt.grid(True, axis='y', linestyle='--', alpha=0.7)
    plt.legend()

    plt.tight_layout()

    # Save the plot
    filepath = os.path.join(output_dir, filename)
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    plt.close()

    print(f"Histogram plot saved to {filepath}")

output_dir = 'plot_results'
make_scatter_comparisons(df, output_dir)

df_density_less_1_5 = df.copy()
df_density_less_1_5 = df_density_less_1_5[df_density_less_1_5['density'] < 1.5]
df_density_more_1_5 = df.copy()
df_density_more_1_5 = df_density_more_1_5[df_density_more_1_5['density'] >= 1.5]
make_scatter_comparisons(df_density_less_1_5, 'plot_results_less_1_5')
make_scatter_comparisons(df_density_more_1_5, 'plot_results_more_1_5')

make_scatter_comparison(df, 'nodes', 'shape_metrics_time',
             'Number of Nodes', 'Shape Metrics Time',
             'Time function per Number of Nodes',
             'shape_metrics_time_function_nodes.png', output_dir,
             add_diagonal=False)

make_histogram_comparison('ogdf_max_bends_per_edge', 'shape_metrics_max_bends_per_edge',
             'Max Bends per Edge', 'Count',
             'Side-by-Side Histogram of Max Bends per Edge Distribution',
             "max_bends_per_edge_histogram.png", output_dir)

make_scatter_comparison(df, 'density', 'shape_metrics_time',
             'Density (edges/nodes)', 'Shape Metrics Time',
             'Time function per Density', 
             'shape_metrics_time_function_density.png', output_dir,
             add_diagonal=False)

make_scatter_comparison(df, 'nodes', 'shape_metrics_number_added_cycles',
             'Number of Nodes', 'Shape Metrics Number Added Cycles',
             'Number of Added Cycles function per Number of Nodes',
             'shape_metrics_number_added_cycles_function_nodes.png', output_dir,
             add_diagonal=False)

make_scatter_comparison(df, 'density', 'shape_metrics_number_added_cycles',
             'Density (edges/nodes)', 'Shape Metrics Number Added Cycles',
             'Number of Added Cycles function per Density',
             'shape_metrics_number_added_cycles_function_density.png', output_dir,
             add_diagonal=False)

make_scatter_comparison(df, 'nodes', 'shape_metrics_number_useless_bends',
             'Number of Nodes', 'Shape Metrics Number Useless Bends',
             'Number of Useless Bends function per Number of Nodes',
             'shape_metrics_number_useless_bends_function_nodes.png', output_dir,
             add_diagonal=False)

make_scatter_comparison(df, 'density', 'shape_metrics_number_useless_bends',
             'Density (edges/nodes)', 'Shape Metrics Number Useless Bends',
             'Number of Useless Bends function per Density',
             'shape_metrics_number_useless_bends_function_density.png', output_dir,
             add_diagonal=False)

make_scatter_comparison(df, 'nodes', 'shape_metrics_bends',
             'Number of Nodes', 'Shape Metrics Bends',
             'Bends function per Number of Nodes',
             'shape_metrics_bends_function_nodes.png', output_dir,
             add_diagonal=False)

make_scatter_comparison(df, 'density', 'shape_metrics_bends',
             'Density (edges/nodes)', 'Shape Metrics Bends',
             'Bends function per Density',
             'shape_metrics_bends_function_density.png', output_dir,
             add_diagonal=False)

# Extract times
times = df['shape_metrics_time']

# Compute 95th percentile cutoff
cutoff = 0.975
time_cutoff = times.quantile(cutoff)

# Filter values under or equal to the cutoff
times_clipped = times[times <= time_cutoff]

# Compute mean (average) of full data
mean_time = times.mean()

# Define bin edges up to the cutoff
max_time_display = math.ceil(time_cutoff)
bins = list(range(0, ((max_time_display // 5) + 2) * 5, 5))

# Plot histogram
plt.figure(figsize=(8, 5))
plt.hist(times_clipped, bins=bins, edgecolor='black')
plt.title(f'Histogram of shape_metrics_time (5s Buckets, {cutoff*100}% Cutoff)')
plt.xlabel('Time (seconds)')
plt.ylabel('Number of Graphs')
plt.grid(axis='y', linestyle='--', alpha=0.7)

# Draw average time line
plt.axvline(mean_time, color='red', linestyle='--', linewidth=1.5, label=f'Average Time ≈ {mean_time:.2f}s')

# Set custom ticks every 10s
xticks = [b for b in bins if b % 10 == 0]
plt.xticks(xticks)

# Add legend
plt.legend()

# Save the histogram
filename = "shape_metrics_time_histogram.png"
filepath = os.path.join(output_dir, filename)
plt.savefig(filepath, dpi=300, bbox_inches='tight')
plt.close()
print(f"Shape metrics time histogram plot saved to {filepath}")