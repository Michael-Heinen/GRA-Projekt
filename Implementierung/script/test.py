import os
import subprocess
import time
import numpy as np
import matplotlib.pyplot as plt
import argparse

# Base directories
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BASE_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, '..'))

FILES_DIR = os.path.join(BASE_DIR, 'files')
RESULTS_DIR = os.path.join(FILES_DIR, 'results')
TEST_MATRICES_DIR = os.path.join(FILES_DIR, 'test_matrices')
EDGE_MATRICES_DIR = os.path.join(FILES_DIR, 'edge_matrices')
EXPECTED_DIR = os.path.join(FILES_DIR, 'expected')

# Ensure directories exist
os.makedirs(FILES_DIR, exist_ok=True)
os.makedirs(RESULTS_DIR, exist_ok=True)
os.makedirs(TEST_MATRICES_DIR, exist_ok=True)
os.makedirs(EDGE_MATRICES_DIR, exist_ok=True)
os.makedirs(EXPECTED_DIR, exist_ok=True)

# Constants
IMPLEMENTATIONS = [0]
MATRIX_SIZES = [2, 4, 8, 16]
DENSITY = 0.8  # 50% density of non-zero elements
EDGE_CASES = [
    ("empty_matrix", (0, 0)),  # Empty matrix
    ("single_element", (1, 1)),  # Single element matrix
    ("all_zeros", (4, 4, 0.0)),  # All elements are zero
    ("large_values", (4, 4, lambda: np.random.uniform(1e10, 1e12))),  # Very large values
    ("small_values", (4, 4, lambda: np.random.uniform(-1e12, -1e10))),  # Very small (negative) values
    ("rectangular", (4, 2)),  # Rectangular matrix
    ("sparse", (4, 4, 0.25))  # Sparse matrix with 25% density
]

TESTING = False

# Function to generate matrices
def generate_matrix(rows, cols, density=0.5):
    if callable(density):
        matrix = np.fromfunction(np.vectorize(lambda i, j: density()), (rows, cols))
    else:
        matrix = np.random.choice([0, 1], size=(rows, cols), p=[1-density, density]) * np.random.rand(rows, cols)
    return matrix

# Function to save matrix to file
def save_matrix_to_file(matrix, filename):
    rows, cols = matrix.shape
    try:
        max_nonzeros = int(np.count_nonzero(matrix, axis=1).max())
    except ValueError:  # empty matrix edge case
        max_nonzeros = 0

    with open(filename, 'w') as f:
        f.write(f"{rows},{cols},{max_nonzeros}\n")
        row_lines = []
        for row in matrix:
            row_values = [str(v) if v != 0 else "*" for v in row]
            row_lines.append(",".join(row_values))
        f.write(",".join(row_lines) + "\n")
    
        index_lines = []
        for row in matrix:
            row_indices = [str(idx) if row[idx] != 0 else "*" for idx in range(cols)]
            index_lines.append(",".join(row_indices))
        f.write(",".join(index_lines) + "\n")

# Function to compare matrices
def compare_matrices(file1, matrix2):
    try:
        matrix1 = load_and_clean_matrix(file1)
        if(TESTING):
            print(f"Matrix of own Implementation: \n{matrix1}")
            print(f"Matrix of comparison: \n{matrix2}")
        return np.allclose(matrix1, matrix2, atol=1e-6)
    except Exception as e:
        print(f"Comparison error: {e}")
        return False

# Function to compile the implementations
def compile_implementations():
    try:
        subprocess.run(["make"], check=True, cwd=BASE_DIR).stdout
    except subprocess.CalledProcessError as e:
        print(e.output)

# Deleting test files for regenerating them
def delete_files_in_directory(directory_path):
    try:
        files = os.listdir(directory_path)
        for file in files:
            file_path = os.path.join(directory_path, file)
            if os.path.isfile(file_path):
                os.remove(file_path)
        print("All files deleted successfully.")
    except OSError:
        print("Error occurred while deleting files.")

# Function to parse execution time from the C program output
def parse_execution_time(output):
    for line in output.splitlines():
        if "Execution time:" in line:
            return float(line.split(":")[1].strip().split()[0])
    return None

# Function to generate test matrices
def generate_test_matrices():
    delete_files_in_directory(TEST_MATRICES_DIR)
    for size in MATRIX_SIZES:
        matrix_a_filename = os.path.join(TEST_MATRICES_DIR, f"matrixA_{size}x{size}.txt")
        matrix_b_filename = os.path.join(TEST_MATRICES_DIR, f"matrixB_{size}x{size}.txt")
        if not (os.path.exists(matrix_a_filename) and os.path.exists(matrix_b_filename)):
            matrix_a = generate_matrix(size, size, DENSITY)
            matrix_b = generate_matrix(size, size, DENSITY)
            save_matrix_to_file(matrix_a, matrix_a_filename)
            save_matrix_to_file(matrix_b, matrix_b_filename)
            # print(f"Generated: {matrix_a_filename} and {matrix_b_filename}")

def generate_edge_case_matrices():
    delete_files_in_directory(EDGE_MATRICES_DIR)
    for case_name, case_params in EDGE_CASES:
        matrix_a_filename = os.path.join(EDGE_MATRICES_DIR, f"edge_case_{case_name}_A.txt")
        matrix_b_filename = os.path.join(EDGE_MATRICES_DIR, f"edge_case_{case_name}_B.txt")
        if not (os.path.exists(matrix_a_filename) and os.path.exists(matrix_b_filename)):
            if len(case_params) == 2:
                rows, cols = case_params
                matrix_a = generate_matrix(rows, cols)
                matrix_b = generate_matrix(rows, cols)
            else:
                rows, cols, density = case_params
                matrix_a = generate_matrix(rows, cols, density)
                matrix_b = generate_matrix(rows, cols, density)
            save_matrix_to_file(matrix_a, matrix_a_filename)
            save_matrix_to_file(matrix_b, matrix_b_filename)
            # print(f"Generated: {matrix_a_filename} and {matrix_b_filename}")

def load_and_clean_matrix(filename):
    # Read the entire file content as a single string
    with open(filename, 'r') as f:
        content = f.read()
    
    # Replace '*' with '0'
    content = content.replace('*', '0') 
    
    #Get dimensions
    lines = content.splitlines()
    dimensions = lines[0].split(',')
    rows, cols = int(dimensions[0]), int(dimensions[1])

    # Load the cleaned content into a NumPy array
    matrix = np.loadtxt(lines, delimiter=',', skiprows=1, max_rows=1)

    # Reshape the matrix according to the dimensions
    matrix = matrix.reshape((rows, cols))    
    return matrix

# Function to run the tests in isolation
def run_isolated_test(command):
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=BASE_DIR)
    stdout, stderr = process.communicate()
    return process.returncode, stdout, stderr

# Function to run the tests
def run_tests(num_runs):
    performance_results = {impl: [] for impl in IMPLEMENTATIONS}
    for size in MATRIX_SIZES:
        matrix_a_filename = os.path.join(TEST_MATRICES_DIR, f"matrixA_{size}x{size}.txt")
        matrix_b_filename = os.path.join(TEST_MATRICES_DIR, f"matrixB_{size}x{size}.txt")
        
        for impl in IMPLEMENTATIONS:
            print(f"\nTesting V{impl} with matrix size {size}x{size}")
            execution_times = []
            
            for i in range(num_runs):  # Run each test three times
                try:
                    command = [os.path.join(BASE_DIR, "main"), f"-V {impl}", "-B", f"-a{matrix_a_filename}", f"-b{matrix_b_filename}", f"-o{os.path.join(RESULTS_DIR, f'result_V{impl}_{size}x{size}.txt')}"]
                    returncode, stdout, stderr = run_isolated_test(command)
                    if returncode == 0:
                        execution_time = parse_execution_time(stdout.decode())
                        if execution_time is not None:
                            execution_times.append(execution_time)
                        else:
                            print("Failed to parse execution time from the output.")
                    else:
                        print(f"Run {i+1} Error: {stderr.decode().strip()}")
                except Exception as e:
                    print(f"Run {i+1} Execution error: {e}")

            if execution_times:
                avg_time = sum(execution_times) / len(execution_times)
                print(f"Average Execution Time for V{impl} with matrix size {size}x{size}: {avg_time:.6f} seconds")
                performance_results[impl].append(avg_time)
                
                # Check correctness with matrixmultiplication module of numpy
                mathmul = np.matmul(load_and_clean_matrix(matrix_a_filename), load_and_clean_matrix(matrix_b_filename))                
                if returncode == 0 and compare_matrices(os.path.join(RESULTS_DIR, f"result_V{impl}_{size}x{size}.txt"), mathmul):
                    print(f"Output correctness: PASSED")
                else:
                    print(f"Output correctness: FAILED")

    # Plot performance results
    plot_performance_results(performance_results)

# Function to run edge case tests
def run_edge_case_tests():
    for case_name, case_params in EDGE_CASES:
        matrix_a_filename = os.path.join(EDGE_MATRICES_DIR, f"edge_case_{case_name}_A.txt")
        matrix_b_filename = os.path.join(EDGE_MATRICES_DIR, f"edge_case_{case_name}_B.txt")

        for impl in IMPLEMENTATIONS:
            print(f"\nTesting V{impl} with edge case '{case_name}'")
            try:
                command = [os.path.join(BASE_DIR, "main"), f"-V {impl}", "-B", f"-a{matrix_a_filename}", f"-b{matrix_b_filename}", f"-o{os.path.join(RESULTS_DIR, f'result_edge_case_{case_name}_V{impl}.txt')}"]
                returncode, stdout, stderr = run_isolated_test(command)
                if returncode == 0:
                    print(f"Edge case '{case_name}' Execution: PASSED")
                else:
                    print(f"Edge case '{case_name}' Execution Error: {stderr.decode().strip()}")
            except Exception as e:
                print(f"Edge case '{case_name}' Execution error: {e}")

# Function to plot performance results
def plot_performance_results(performance_results):
    plt.figure(figsize=(12, 8))
    for impl in IMPLEMENTATIONS:
        # Filter out None values for plotting
        sizes = []
        times = []
        for size, time in zip(MATRIX_SIZES, performance_results[impl]):
            if time is not None:
                sizes.append(size)
                times.append(time)
        if sizes and times:  # Ensure there's data to plot
            plt.plot(sizes, times, marker='o', label=impl)
    
    plt.xlabel('Matrix Size (NxM)')
    plt.ylabel('Average Execution Time (seconds)')
    plt.title('Performance Comparison of Matrix Multiplication Implementations')
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(SCRIPT_DIR, 'performance_comparison.png'))

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Matrix Multiplication Performance Testing')
    parser.add_argument('-v','--versions', type=int, default=[0], help='Versions to test')
    parser.add_argument('-d','--density', type=float, default=0.2, help='Density of the matrices')
    parser.add_argument('-ms','--matrix_sizes', type=int, nargs='+', default=[2, 4, 8, 16], help='List of matrix sizes')
    parser.add_argument('-n','--num_runs', type=int, default=3, help='Number of runs for each test')

    parser.add_argument('-c', '--compile', action='store_false', help='Does NOT Compile the implementations')
    parser.add_argument('-g', '--generate', action='store_true', help='Generate test matrices')
    parser.add_argument('-e', '--edge', action='store_true', help='Test edge case matrices')
    
    parser.add_argument('-p', '--plot', action='store_false', help='Does NOT Plot performance results')
    parser.add_argument('-t', '--testing', action='store_true', help='Print Testing output')

    args = parser.parse_args()
    
    MATRIX_SIZES = args.matrix_sizes
    DENSITY = args.density
    IMPLEMENTATIONS = args.versions
    TESTING = args.testing


    # Compile the implementations
    if args.compile:
        compile_implementations()

    # Generate test matrices if needed
    if args.generate:
        generate_test_matrices()
        if(args.edge):
            generate_edge_case_matrices()

    # Run matrix tests
    run_tests(args.num_runs)
    
    # Run edge case tests
    if(args.edge):
        run_edge_case_tests()
