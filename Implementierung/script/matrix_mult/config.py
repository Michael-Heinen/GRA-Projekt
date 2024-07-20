import os
import numpy as np
# Base directories
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BASE_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, '../..'))

FILES_DIR = os.path.join(BASE_DIR, 'files')
RESULTS_DIR = os.path.join(FILES_DIR, 'results')
TEST_MATRICES_DIR = os.path.join(FILES_DIR, 'test_matrices')
EDGE_MATRICES_DIR = os.path.join(FILES_DIR, 'edge_matrices')

# Ensure directories exist
os.makedirs(FILES_DIR, exist_ok=True)
os.makedirs(RESULTS_DIR, exist_ok=True)
os.makedirs(TEST_MATRICES_DIR, exist_ok=True)
os.makedirs(EDGE_MATRICES_DIR, exist_ok=True)

# Constants
EDGE_CASES = [
    ("empty_matrix", (0, 0)),  # Empty matrix
    ("single_element", (1, 1)),  # Single element matrix
    ("all_zeros", (4, 4, 0,0)),  # All elements are zero
    ("large_values", (4, 4, np.random.uniform(1e10, 1e12))),  # Very large values
    ("small_values", (4, 4, np.random.uniform(-1e12, -1e10))),  # Very small (negative) values
    ("rectangular", (4, 2)),  # Rectangular matrix
    ("sparse", (4, 4, 0,25))  # Sparse matrix with 25% density
]