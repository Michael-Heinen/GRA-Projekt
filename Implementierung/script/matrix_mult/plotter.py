import matplotlib.pyplot as plt
import os
import json
from .config import *

# Function to plot performance results from JSON file
def plot_performance_results(json_filename, densities, matrix_sizes, implementations):
    with open(json_filename, 'r') as f:
        performance_results = json.load(f)

    num_densities = len(densities)
    fig, axes = plt.subplots(num_densities, 1, figsize=(12, 8), sharex=True, sharey=True)

    if num_densities == 1:
        axes = [axes]

    for i, density in enumerate(densities):
        ax = axes[i]
        for impl in implementations:
            # Filter out None values for plotting
            sizes = []
            times = []
            for size, time in zip(matrix_sizes, performance_results[str(density)][impl]):
                if time is not None:
                    sizes.append(size)
                    times.append(time)
            if sizes and times:  # Ensure there's data to plot
                ax.plot(sizes, times, marker='o', label=f"V:{impl}")

        ax.set_ylabel('Execution Time (s)')
        ax.set_title(f'Density: {density}')
        ax.grid(True)
        ax.legend()

    axes[-1].set_xlabel('Matrix Size ((NxM)/2)')
    plt.suptitle('Performance Comparison of Matrix Multiplication Implementations')
    plt.savefig(os.path.join(BASE_DIR, f'performance_comparison.png'))