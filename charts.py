import pandas as pd
import matplotlib.pyplot as plt
import re
import numpy as np
import os
import math
from concurrent.futures import ProcessPoolExecutor
import shutil
from collections import Counter
import seaborn as sns

df = pd.read_csv('test_results___.csv')

def parse_graph_name(name):
    n_match = re.search(r'n(\d+)', name)
    m_match = re.search(r'm(\d+)', name)
    nodes = int(n_match.group(1)) if n_match else 0
    edges = int(m_match.group(1)) if m_match else 0
    return nodes, edges

df['nodes'] = df['graph_name'].apply(lambda x: parse_graph_name(x)[0])
df['edges'] = df['graph_name'].apply(lambda x: parse_graph_name(x)[1])
df['density'] = df['edges'] / df['nodes']
df['shape_metrics_total_added_bends'] = df['shape_metrics_number_useless_bends'] + df['shape_metrics_bends']
df['shape_metrics_sat_invocations'] = df['shape_metrics_number_added_cycles'] + df['shape_metrics_total_added_bends'] + 1
# df['shape_metrics_ratio_useless_bends'] = df['shape_metrics_number_useless_bends'] / df['shape_metrics_total_added_bends']
# df['good_bends_ratio'] = df['shape_metrics_bends'] / (df['shape_metrics_bends'] + df['shape_metrics_number_useless_bends'])

if df.isnull().values.any():
    nan_columns = df.columns[df.isnull().any()].tolist()
    print(f"NaN values found in columns: {nan_columns}")
    raise ValueError("DataFrame contains NaN values. Please clean the data before proceeding.")

output_dir = 'plot_results'
if os.path.exists(output_dir):
    import shutil
    shutil.rmtree(output_dir, ignore_errors=False)

scatter_comparisons_dir = os.path.join(output_dir, 'scatter_comparisons')
# scatter_comparisons_less_1_5_dir = os.path.join(output_dir, 'scatter_comparisons_less_1_5')
# scatter_comparisons_more_1_5_dir = os.path.join(output_dir, 'scatter_comparisons_more_1_5')

scatter_percentage_dir = os.path.join(output_dir, 'percentage_difference')

# histograms_dir = os.path.join(output_dir, 'histograms')

cdf_dir = os.path.join(output_dir, 'cdf')

# shape_metrics_functions_dir = os.path.join(output_dir, 'shape_metrics_functions')

if os.path.exists(output_dir):
    shutil.rmtree(output_dir)

os.makedirs(output_dir)
# os.makedirs(scatter_comparisons_less_1_5_dir)
# os.makedirs(scatter_comparisons_more_1_5_dir)
os.makedirs(scatter_comparisons_dir)
os.makedirs(scatter_percentage_dir)
# os.makedirs(histograms_dir)
# os.makedirs(shape_metrics_functions_dir)
os.makedirs(cdf_dir)

def make_winner_comparison_scatter(df, metric_name, filename, output_dir):
    grouped = df.groupby(['nodes', 'density']).agg({
        f'ogdf_{metric_name}': 'mean',
        f'shape_metrics_{metric_name}': 'mean'
    }).reset_index()

    grouped['metric_diff'] = np.sign(grouped[f'ogdf_{metric_name}'] - grouped[f'shape_metrics_{metric_name}'])
    
    max_abs_diff = np.abs(grouped['metric_diff']).max()

    plt.figure(figsize=(10, 8))
    plt.scatter(
        grouped['nodes'], grouped['density'],
        c=grouped['metric_diff'],
        cmap='coolwarm_r',
        vmin=-max_abs_diff,
        vmax= max_abs_diff,
        edgecolor='white',
        s=150
    )

    plt.xlabel('Number of Vertices (n)')
    plt.ylabel('Density (m/n)')
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.tight_layout()

    filepath = os.path.join(output_dir, filename)
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    plt.close()

def make_scatter_comparison(df, x_row_name, y_row_name, x_label, y_label, filename, output_dir, add_diagonal=True):
    plt.figure(figsize=(7, 6))

    # Get x and y values
    x = df[x_row_name]
    y = df[y_row_name]
    
    # Create scatter plot
    plt.scatter(x, y, edgecolor='k', s=15, color='royalblue', alpha=0.7)
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.grid(True, linestyle='--', alpha=0.7)
    
    if (add_diagonal): # Add diagonal line (bisector)
        min_val = min(x.min(), y.min())
        max_val = max(x.max(), y.max())
        plt.plot([min_val, max_val], [min_val, max_val], linestyle='--', color='gray', label='y = x')
    
    # Fit a 2nd-degree (quadratic) polynomial
    coeffs = np.polyfit(x, y, deg=2)
    poly = np.poly1d(coeffs)

    # Generate smooth curve points
    x_fit = np.linspace(min(x), max(x), 500)
    y_fit = poly(x_fit)

    # R² computation (manually for nonlinear)
    y_pred = poly(x)
    ss_res = np.sum((y - y_pred) ** 2)
    ss_tot = np.sum((y - np.mean(y)) ** 2)
    r2 = 1 - (ss_res / ss_tot)

    # Plot
    fit_label = f'Best fit: y = {coeffs[0]:.4f}x² + {coeffs[1]:.4f}x + {coeffs[2]:.4f}\n(R² = {r2:.2f})'
    plt.plot(x_fit, y_fit, 'r--', label=fit_label)

    plt.legend()
    plt.tight_layout()
    filepath = os.path.join(output_dir, filename)
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    plt.close()

def make_scatter_comparison_special(df, x_row_name, y_row_name, x_label, y_label, filename, output_dir, add_diagonal=True):
    plt.figure(figsize=(7, 6))

    x = df[x_row_name]
    y = df[y_row_name]
    
    # Remove NaNs
    mask = ~np.isnan(x) & ~np.isnan(y)
    x = x[mask]
    y = y[mask]

    # Count frequency of each (x, y) pair
    coords = list(zip(x, y))
    counter = Counter(coords)

    # Get unique (x, y) pairs and their counts
    unique_coords = np.array(list(counter.keys()))
    counts = np.array(list(counter.values()))

    x_unique = unique_coords[:, 0]
    y_unique = unique_coords[:, 1]

    def get_circle_size(counts):
        return 50 * np.sqrt(counts / 3.1415926535)
    
    size = get_circle_size(counts)

    # Create scatter plot with sizes based on count
    plt.scatter(x_unique, y_unique, edgecolor='k', s=size, alpha=0.7, color='royalblue')
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.grid(True, linestyle='--', alpha=0.7)

    # Diagonal line
    if add_diagonal:
        min_val = min(x.min(), y.min())
        max_val = max(x.max(), y.max())
        plt.plot([min_val, max_val], [min_val, max_val], linestyle='--', color='gray', label='y = x')

    # Fit quadratic polynomial to raw data
    coeffs = np.polyfit(x, y, deg=2)
    poly = np.poly1d(coeffs)

    # R² computation
    y_pred = poly(x)
    ss_res = np.sum((y - y_pred) ** 2)
    ss_tot = np.sum((y - np.mean(y)) ** 2)
    r2 = 1 - (ss_res / ss_tot)

    # Plot best-fit curve
    x_fit = np.linspace(x.min(), x.max(), 500)
    y_fit = poly(x_fit)
    fit_label = f'Best fit: y = {coeffs[0]:.4f}x² + {coeffs[1]:.4f}x + {coeffs[2]:.4f}\n(R² = {r2:.2f})'
    plt.plot(x_fit, y_fit, 'r--', label=fit_label)

    # Add legend for circle sizes
    legend_counts = [1, 10, 100]
    handles = []
    for count in legend_counts:
        size = get_circle_size(count)
        handles.append(plt.scatter([], [], s=size, edgecolor='k', facecolor='royalblue', alpha=0.7, label=f'{count}'))
    plt.legend(handles=handles, loc='upper right', frameon=True)
    plt.legend()
    plt.tight_layout()
    filepath = os.path.join(output_dir, filename)
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    plt.close()

def make_histogram_comparison(x_row_name, y_row_name, x_label, y_label, title, filename, output_dir):
    # Histogram comparison for max_bends_per_edge
    plt.figure(figsize=(12, 6))

    # Get unique values and counts
    all_values = np.sort(pd.concat([
        df[x_row_name],
        df[y_row_name]
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

def make_histogram_bucket(df, col_name, cutoff, bin_size, output_dir, x_label, y_label):
    column = df[col_name]
    # Compute 95th percentile cutoff
    value_cutoff = column.quantile(cutoff)
    # Filter values under or equal to the cutoff
    column_clipped = column[column <= value_cutoff]
    mean_value = column.mean()
    # Define bin edges up to the cutoff
    max_value_display = math.ceil(value_cutoff)
    bins = np.arange(0, max_value_display + bin_size, bin_size)

    # Plot histogram
    plt.figure(figsize=(8, 5))
    plt.hist(column_clipped, bins=bins, edgecolor='black')
    plt.title(f'Histogram of {col_name} ({bin_size} buckets size, {cutoff*100}% Cutoff)')
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.grid(axis='y', linestyle='--', alpha=0.7)

    # Draw average time line
    plt.axvline(mean_value, color='red', linestyle='--', linewidth=1.5, label=f'Average {x_label} ≈ {mean_value:.2f}s')

    # Set custom ticks every 10s
    xticks = [b for b in bins if b % bin_size == 0]
    plt.xticks(xticks)

    # Add legend
    plt.legend()

    # Save the histogram
    filename = f"{col_name}_histogram_buckets.png"
    filepath = os.path.join(output_dir, filename)
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    plt.close()

def make_cdf(column, output_path, use_log_scale=False, xlabel='Value'):
    sns.ecdfplot(column)
    if (use_log_scale):
        plt.xscale('log')
    plt.xlabel(xlabel)
    plt.ylabel('CDF')
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    plt.close()

def make_double_cdf(column1, column2, output_path, label1, label2, xlabel='Value'):
    sns.ecdfplot(column1, color='blue', label=label1)
    sns.ecdfplot(column2, color='red', label=label2)
    plt.xlabel(xlabel)
    plt.ylabel('CDF')
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    plt.close()

def print_percentages_comparison(df, metric, x, y):
    ogdf_higher_count = (df[x] > df[y]).sum()
    shape_higher_count = (df[y] > df[x]).sum()
    equal_count = (df[x] == df[y]).sum()
    total_count = len(df)
    print(f"{metric}:")
    print(f"OGDF > Shape Metrics: {ogdf_higher_count} times ({ogdf_higher_count/total_count*100:.2f}%)")
    print(f"Shape Metrics > OGDF: {shape_higher_count} times ({shape_higher_count/total_count*100:.2f}%)")
    print(f"Shape Metrics = OGDF: {equal_count} times ({equal_count/total_count*100:.2f}%)")
    print()

def generate_all_plots_parallel(df):
    # df_less = df[df['density'] < 1.5]
    # df_more = df[df['density'] >= 1.5]

    metrics_scatter_comparison = [
        'crossings',
        'bends',
        'area',
        'total_edge_length',
        'time',
        'max_edge_length',
        'edge_length_stddev',
        'bends_stddev',
    ]

    metric_scatter_comparison_special = [
        'max_bends_per_edge',
    ]

    metrics_percentage_difference = [
        ('crossings', 'crossings_diff_scatter.pdf'),
        ('bends', 'bends_diff_scatter.pdf'),
        ('area', 'area_diff_scatter.pdf'),
        ('total_edge_length', 'total_edge_length_diff_scatter.pdf'),
        ('max_edge_length', 'max_edge_length_diff_scatter.pdf'),
        ('max_bends_per_edge', 'max_bends_per_edge_diff_scatter.pdf'),
        ('edge_length_stddev', 'edge_length_stddev_diff_scatter.pdf'),
        ('bends_stddev', 'bends_stddev_diff_scatter.pdf')
    ]
    
    with ProcessPoolExecutor() as executor:
        futures = []

        for metric in metrics_scatter_comparison:
            x = f'ogdf_{metric}'
            y = f'shape_metrics_{metric}'
            f = f"{metric}_comparison.pdf"
            print_percentages_comparison(df, metric, x, y)
            futures.append(executor.submit(make_scatter_comparison, df, x, y, 'OGDF', 'DOMUS', f, scatter_comparisons_dir))

            # futures.append(executor.submit(make_scatter_comparison, df_less, x, y, 'OGDF', 'DOMUS', f"{metric}_less.pdf", scatter_comparisons_less_1_5_dir))
            # futures.append(executor.submit(make_scatter_comparison, df_more, x, y, 'OGDF', 'DOMUS', f"{metric}_more.pdf", scatter_comparisons_more_1_5_dir))

        for metric in metric_scatter_comparison_special:
            x = f'ogdf_{metric}'
            y = f'shape_metrics_{metric}'
            f = f"{metric}_comparison_special.pdf"
            print_percentages_comparison(df, metric, x, y)
            futures.append(executor.submit(make_scatter_comparison_special, df, x, y, 'OGDF', 'DOMUS', f, scatter_comparisons_dir))

            # futures.append(executor.submit(make_scatter_comparison_special, df_less, x, y, 'OGDF', 'DOMUS', f"{metric}_less.pdf", scatter_comparisons_less_1_5_dir))
            # futures.append(executor.submit(make_scatter_comparison_special, df_more, x, y, 'OGDF', 'DOMUS', f"{metric}_more.pdf", scatter_comparisons_more_1_5_dir))

        
        for metric, filename in metrics_percentage_difference:
            futures.append(executor.submit(make_winner_comparison_scatter, df, metric, filename, scatter_percentage_dir))

        futures += [
            # executor.submit(make_scatter_comparison, df, 'nodes', 'shape_metrics_time', 'Number of Nodes', 'Shape Metrics Time', 'shape_metrics_time_nodes.pdf', shape_metrics_functions_dir, False),
            # executor.submit(make_scatter_comparison, df, 'density', 'shape_metrics_time', 'Density', 'Shape Metrics Time', 'shape_metrics_time_density.pdf', shape_metrics_functions_dir, False),
            # executor.submit(make_scatter_comparison, df, 'nodes', 'shape_metrics_number_added_cycles', 'Nodes', 'Added Cycles', 'added_cycles_nodes.pdf', shape_metrics_functions_dir, False),
            # executor.submit(make_scatter_comparison, df, 'density', 'shape_metrics_number_added_cycles', 'Density', 'Added Cycles', 'added_cycles_density.pdf', shape_metrics_functions_dir, False),
            # executor.submit(make_scatter_comparison, df, 'nodes', 'shape_metrics_bends', 'Nodes', 'Bends', 'bends_nodes.pdf', shape_metrics_functions_dir, False),
            # executor.submit(make_scatter_comparison, df, 'density', 'shape_metrics_bends', 'Density', 'Bends', 'bends_density.pdf', shape_metrics_functions_dir, False),
            # executor.submit(make_histogram_comparison, 'ogdf_max_bends_per_edge', 'shape_metrics_max_bends_per_edge', 'Max Bends per Edge', 'Count', '', 'max_bends_hist.pdf', histograms_dir),
            # executor.submit(make_histogram_bucket, df, 'shape_metrics_time', 0.925, 5, histograms_dir, 'Time (s)', 'Graphs'),
            # executor.submit(make_histogram_bucket, df, 'shape_metrics_number_added_cycles', 1.00, 10, histograms_dir, 'Added Cycles', 'Graphs'),
            executor.submit(make_cdf, df['shape_metrics_time'], os.path.join(cdf_dir, 'shape_metrics_time_cdf.pdf'), True, 'Time (s)'),
            executor.submit(make_double_cdf, df['shape_metrics_total_added_bends'], df['shape_metrics_bends'], os.path.join(cdf_dir, 'shape_metrics_bends_cdf.pdf'), 'Fictitious Vertices', 'Bends'),
            executor.submit(make_cdf, df['shape_metrics_number_added_cycles'], os.path.join(cdf_dir, 'shape_metrics_added_cycles_cdf.pdf')),
            executor.submit(make_cdf, df['shape_metrics_sat_invocations'], os.path.join(cdf_dir, 'shape_metrics_sat_invocations_cdf.pdf')),
        ]

        for fut in futures:
            fut.result()

if __name__ == '__main__':
    generate_all_plots_parallel(df)