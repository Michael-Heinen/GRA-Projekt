import os
import subprocess
import numpy as np
import cupy as cp
import argparse
import time
import random

from matrix_mult.config import *
from matrix_mult.plotter import plot_performance_results

# Function to save matrix to file
def save_matrix_row_by_row(rows, cols, density, filename):
    max_nonzeros = 0
    row_values_list = []
    row_indices_list = []

    with open(filename, 'w') as f:
        # First pass: Generate the matrix and determine max_nonzeros
        for row in range(rows):
            row_values = []
            row_indices = []

            for col in range(cols):
                if random.random() < density:
                    value = random.random()
                    row_values.append(value)
                    row_indices.append(col)
            
            max_nonzeros = max(max_nonzeros, len(row_values))
            row_values_list.append(row_values)
            row_indices_list.append(row_indices)

        # Write metadata
        f.write(f"{rows},{cols},{max_nonzeros}\n")

        # Edge Case: if 0 Matrix
        if(max_nonzeros == 0):
            f.write("\n\n")
            return

        row_lines = []
        for row_values in row_values_list:
            filtered_values = [str(v) for v in row_values] + ["*"] * (max_nonzeros - len(row_values))
            row_lines.append(",".join(filtered_values))
        f.write(",".join(row_lines) + "\n")

        index_lines = []
        for row_indices in row_indices_list:
            filtered_indices = [str(i) for i in row_indices] + ["*"] * (max_nonzeros - len(row_indices))
            index_lines.append(",".join(filtered_indices))
        f.write(",".join(index_lines))
    
# Function to compare matrices
def compare_matrices(matrix1, matrix2):
    try:
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
        print(f"All files in {directory_path} deleted successfully.")
        print(f"All files in {directory_path} deleted successfully.")
    except OSError:
        print("Error occurred while deleting files.")

# Function to parse execution time from the C program output
def parse_execution_time(output):
    for line in output.splitlines():
        if "Execution time:" in line:
            return float(line.split(":")[1].strip().split()[0])
    return None

# Function to generate test matrices
def generate_test_matrices(density):
    for size in MATRIX_SIZES:
        matrix_a_filename = os.path.join(TEST_MATRICES_DIR, f"matrixA_{size}x{size}_D{density}.txt")
        matrix_b_filename = os.path.join(TEST_MATRICES_DIR, f"matrixB_{size}x{size}_D{density}.txt")
        if not (os.path.exists(matrix_a_filename) and os.path.exists(matrix_b_filename)):
            save_matrix_row_by_row(size, size, density, matrix_a_filename)
            save_matrix_row_by_row(size, size, density, matrix_b_filename)

# Function to generate edge case matrices
def generate_edge_case_matrices():
    for case_name, case_params in EDGE_CASES:
        matrix_a_filename = os.path.join(EDGE_MATRICES_DIR, f"edge_case_{case_name}_A.txt")
        matrix_b_filename = os.path.join(EDGE_MATRICES_DIR, f"edge_case_{case_name}_B.txt")
        if (os.path.exists(matrix_a_filename) and os.path.exists(matrix_b_filename)):
            if len(case_params) == 2:
                rows, cols = case_params
                density = 0.25
                matrix_a = np.random.choice([0, 1], size=(rows, cols), p=[1-density, density]) * np.random.rand(rows, cols)
                matrix_b = np.random.choice([0, 1], size=(rows, cols), p=[1-density, density]) * np.random.rand(rows, cols)
            else:
                rows, cols, density = case_params
                matrix_a = np.random.choice([0, 1], size=(rows, cols), p=[1-density, density]) * np.random.rand(rows, cols)
                matrix_b = np.random.choice([0, 1], size=(rows, cols), p=[1-density, density]) * np.random.rand(rows, cols)
        # save_matrix_row_by_row(size, size, density, matrix_a_filename)
        # save_matrix_row_by_row(size, size, density, matrix_b_filename)

# load and clean matrixes for comparison
def load_and_clean_matrix(filename):
    with open(filename, 'r') as f:
        rows, cols, max_nonzeros = map(int, f.readline().strip().split(','))
        
        value_lines = [f.readline().strip().split(',')]
        index_lines = [f.readline().strip().split(',')]

        values = np.zeros((rows, cols))
        
        value_row = value_lines[0]
        index_row = index_lines[0]
        count = 0
        row_counter = 0
        try:
            for val, idx in zip(value_row, index_row):
                if val != "*" and idx != "*":
                    col_index = int(idx)
                    values[row_counter, col_index] = float(val)
                count += 1
                if count >= max_nonzeros:
                    count = 0
                    row_counter += 1
        except ValueError as e:
            return values
        try:
            for val, idx in zip(value_row, index_row):
                if val != "*" and idx != "*":
                    col_index = int(idx)
                    values[row_counter, col_index] = float(val)
                count += 1
                if count >= max_nonzeros:
                    count = 0
                    row_counter += 1
        except ValueError as e:
            return values

    return values

# Function to run the tests in isolation
def run_isolated_test(command, timeout):
    try:
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=BASE_DIR)
        stdout, stderr = process.communicate(timeout=timeout)
        return process.returncode, stdout, stderr, False
    except subprocess.TimeoutExpired:
        process.kill()
        stdout, stderr = process.communicate()
        return -1, stdout, stderr, True

def run_isolated_test(command, timeout):
    try:
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=BASE_DIR)
        stdout, stderr = process.communicate(timeout=timeout)
        return process.returncode, stdout, stderr, False
    except subprocess.TimeoutExpired:
        process.kill()
        stdout, stderr = process.communicate()
        return -1, stdout, stderr, True


# Function to run the tests
def run_tests(num_runs, timeout=60):
    performance_results = {impl: [] for impl in IMPLEMENTATIONS}
    timed_out_versions = set()

    timed_out_versions = set()

    for size in MATRIX_SIZES:
        matrix_a_filename = os.path.join(TEST_MATRICES_DIR, f"matrixA_{size}x{size}_D{density}.txt")
        matrix_b_filename = os.path.join(TEST_MATRICES_DIR, f"matrixB_{size}x{size}_D{density}.txt")
        
        for impl in IMPLEMENTATIONS:
            if impl in timed_out_versions:
                continue
            
            print(f"\nTesting V{impl} with matrix size {size}x{size}_D{density}")
            execution_times = []
            
            for i in range(num_runs):  # Run each test three times
                try:
                    command = [os.path.join(BASE_DIR, "main"), f"-V {impl}", "-B", f"-a{matrix_a_filename}", f"-b{matrix_b_filename}", f"-o{os.path.join(RESULTS_DIR, f'result_V{impl}_{size}x{size}.txt')}"]
                    returncode, stdout, stderr, timed_out = run_isolated_test(command, timeout)
                    if timed_out:
                        print(f"Run {i+1} for V{impl} timed out.")
                        timed_out_versions.add(impl)
                        break  # Skip further runs for this implementation
                    returncode, stdout, stderr, timed_out = run_isolated_test(command, timeout)
                    if timed_out:
                        print(f"Run {i+1} for V{impl} timed out.")
                        timed_out_versions.add(impl)
                        break  # Skip further runs for this implementation
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
                performance_results[impl].append(avg_time)
                print(f"Average Execution Time for V{impl} with matrix size {size}x{size}_D{density}: {avg_time:.6f} seconds")
                
                if COMPARE:
                    # Check correctness with matrixmultiplication module of numpy
                    if GPU:
                        mathmul = cp.matmul(load_and_clean_matrix(matrix_a_filename), load_and_clean_matrix(matrix_b_filename))                
                    else:
                        mathmul = np.matmul(load_and_clean_matrix(matrix_a_filename), load_and_clean_matrix(matrix_b_filename))                               

                    result_path = os.path.join(RESULTS_DIR, f"result_V{impl}_{size}x{size}.txt")
                    if returncode == 0 and compare_matrices(load_and_clean_matrix(result_path), mathmul):
                        print(f"Output correctness: PASSED")
                    else:
                        print(f"Output correctness: FAILED")

    return performance_results       
    return performance_results       

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

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Matrix Multiplication Performance Testing')
    parser.add_argument('-V','--versions', type=int, nargs='+', default=[0, 2, 4], help='Versions to test')
    parser.add_argument('-d','--density', type=float, nargs='+', default=[0.2, 0.5, 0.8], help='Density of the matrices')
    parser.add_argument('-ms','--matrix_sizes', type=int, nargs='+', default=[8, 16, 32, 64, 128, 256, 512,750, 1024,1535 ,2048, 3064, 4096], help='List of matrix sizes')#6045, 8054, 10564, 12354]
    parser.add_argument('-n','--num_runs', type=int, default=1, help='Number of runs for each test')
    parser.add_argument('-tmo','--timeout', type=int, default=60, help='Timeout for each test in seconds')
    parser.add_argument('-tmo','--timeout', type=int, default=60, help='Timeout for each test in seconds')

    parser.add_argument('-c', '--compile', action='store_false', help='Does NOT Compile the implementations')
    parser.add_argument('-g', '--generate', action='store_true', help='Generate new test matrices for all specified indices')
    parser.add_argument('-e', '--edge', action='store_true', help='Test edge case matrices')
    
    parser.add_argument('-p', '--plot', action='store_false', help='Does NOT Plot performance results')
    parser.add_argument('-o', '--output', action='store_false', help='Outputs more specific')
    parser.add_argument('-t', '--testing', action='store_true', help='Print Comparison Matrizes output to see differences')
    parser.add_argument('-gpu', '--gpu', action='store_true', help='Using GPU for Comparing Matrix Mul')
    parser.add_argument('-cmp', '--compare', action='store_false', help='Comparing with numpy matrix multiplication')
    args = parser.parse_args()
    
    MATRIX_SIZES = args.matrix_sizes
    IMPLEMENTATIONS = args.versions
    TESTING = args.testing
    GPU = args.gpu
    COMPARE = args.compare
    GPU = args.gpu
    COMPARE = args.compare

    delete_files_in_directory(RESULTS_DIR)

    # Compile the implementations
    if args.compile:
        compile_implementations()
    
    # delete test_matrices files if they should be newly generated 
    if args.generate:
        delete_files_in_directory(TEST_MATRICES_DIR)

    # Testing the script by each density provided
    performances = []
    for density in args.density:
        # Generate test matrices if needed
        generate_test_matrices(density)
        
        # Run matrix tests
        perf = run_tests(args.num_runs, args.timeout)
        performances.append(perf)
    
    # Plot performance results
    if args.plot:
        plot_performance_results(performances, args.density, args.matrix_sizes, args.versions)
    
    # Run edge case tests
    if(args.edge):
        generate_edge_case_matrices()
        generate_edge_case_matrices()
        run_edge_case_tests()
