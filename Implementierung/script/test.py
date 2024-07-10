import os
import subprocess
import time
import numpy as np
import matplotlib.pyplot as plt

# Constants
IMPLEMENTATIONS = [0, 1, 2]
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
    max_nonzeros = int(np.count_nonzero(matrix, axis=1).max())
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
def compare_matrices(file1, file2):
    try:
        matrix1 = np.loadtxt(file1, delimiter=',', skiprows=1)
        matrix2 = np.loadtxt(file2, delimiter=',', skiprows=1)
        return np.allclose(matrix1, matrix2, atol=1e-6)
    except Exception as e:
        print(f"Comparison error: {e}")
        return False

# Function to compile the implementations
def compile_implementations():
    try:
        subprocess.run(["make"], check=True, cwd=".").stdout
    except subprocess.CalledProcessError as e:
        print(e.output)

# Deleting test files for regenerating them
def delete_files_in_directory():
    directory_path = "files/"
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
    delete_files_in_directory()
    for size in MATRIX_SIZES:
        matrix_a_filename = f"files/matrixA_{size}.txt"
        matrix_b_filename = f"files/matrixB_{size}.txt"
        if not (os.path.exists(matrix_a_filename) and os.path.exists(matrix_b_filename)):
            matrix_a = generate_matrix(size, size, DENSITY)
            matrix_b = generate_matrix(size, size, DENSITY)
            save_matrix_to_file(matrix_a, matrix_a_filename)
            save_matrix_to_file(matrix_b, matrix_b_filename)
            print(f"Generated: {matrix_a_filename} and {matrix_b_filename}")

def generate_edge_case_matrices():
    for case_name, case_params in EDGE_CASES:
        matrix_a_filename = f"files/edge_case_{case_name}_A.txt"
        matrix_b_filename = f"files/edge_case_{case_name}_B.txt"
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
            print(f"Generated: {matrix_a_filename} and {matrix_b_filename}")

# Function to run the tests in isolation
def run_isolated_test(command):
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()
    return process.returncode, stdout, stderr

# Function to run the tests
def run_tests():
    performance_results = {impl: [] for impl in IMPLEMENTATIONS}
    for size in MATRIX_SIZES:
        matrix_a_filename = f"files/matrixA_{size}.txt"
        matrix_b_filename = f"files/matrixB_{size}.txt"
        
        for impl in IMPLEMENTATIONS:
            print(f"\nTesting {impl} with matrix size {size}x{size}")
            execution_times = []
            
            for i in range(3):  # Run each test three times
                # start_time = time.time()
                try:
                    command = [f"./main", f"-V {impl}", "-B", f"-a{matrix_a_filename}", f"-b{matrix_b_filename}", f"-ofiles/result_V{impl}_{size}.txt"]
                    returncode, stdout, stderr = run_isolated_test(command)
                    # elapsed_time = time.time() - start_time
                    if returncode == 0:
                        execution_time = parse_execution_time(stdout.decode())
                        if execution_time is not None:
                            execution_times.append(execution_time)
                            print(f"Run {i+1} Execution Time: {execution_time} seconds")
                            # print(f"Execution time: {execution_time} seconds")
                        else:
                            print("Failed to parse execution time from the output.")
                    else:
                        print(f"Run {i+1} Error: {stderr.decode().strip()}")
                except Exception as e:
                    print(f"Run {i+1} Execution error: {e}")

            if execution_times:
                avg_time = sum(execution_times) / len(execution_times)
                print(f"Average Execution Time for {impl} with matrix size {size}x{size}: {avg_time:.6f} seconds")
                performance_results[impl].append(avg_time)
                
                # Check correctness
                expected_file = f"files/expected_result_{size}.txt"
                command = ["./main", "-V 0", matrix_a_filename, matrix_b_filename, expected_file]
                returncode, stdout, stderr = run_isolated_test(command)
                if returncode == 0 and compare_matrices(f"files/result_{impl}_{size}.txt", expected_file):
                    print(f"Output correctness: PASSED")
                else:
                    print(f"Output correctness: FAILED")

    # Plot performance results
    plot_performance_results(performance_results)

# Function to run edge case tests
def run_edge_case_tests():
    for case_name, case_params in EDGE_CASES:
        print(f"\nTesting edge case: {case_name}")
        matrix_a_filename = f"files/edge_case_{case_name}_A.txt"
        matrix_b_filename = f"files/edge_case_{case_name}_B.txt"
        
        for impl in IMPLEMENTATIONS:
            print(f"\nTesting {impl} with edge case: {case_name}")
            
            # Measure execution time
            start_time = time.time()
            try:
                command = [f"./main", f"-V{impl}", "-B", f"-a{matrix_a_filename}", f"-b{matrix_b_filename}", f"-ofiles/result_{impl}_{case_name}.txt"]
                returncode, stdout, stderr = run_isolated_test(command)
                elapsed_time = time.time() - start_time
                if returncode == 0:
                    print(f"Execution Time: {elapsed_time:.6f} seconds")
                else:
                    print(f"Error: {stderr.decode().strip()}")
                
                # Check correctness
                expected_file = f"files/expected_result_{case_name}.txt"
                command = ["./main", "-V 0", matrix_a_filename, matrix_b_filename, expected_file]
                returncode, stdout, stderr = run_isolated_test(command)
                if returncode == 0 and compare_matrices(f"files/result_{impl}_{case_name}.txt", expected_file):
                    print(f"Output correctness: PASSED")
                else:
                    print(f"Output correctness: FAILED")
            except Exception as e:
                print(f"Execution error: {e}")

# Function to plot performance results
def plot_performance_results(performance_results):
    plt.figure(figsize=(12, 8))
    for impl in IMPLEMENTATIONS:
        plt.plot(MATRIX_SIZES, performance_results[impl], marker='o', label=impl)
    
    plt.xlabel('Matrix Size (NxN)')
    plt.ylabel('Average Execution Time (seconds)')
    plt.title('Performance Comparison of Matrix Multiplication Implementations')
    plt.legend()
    plt.grid(True)
    plt.savefig('performance_comparison.png')
    # plt.show()

if __name__ == "__main__":
    # Compile the implementations
    compile_implementations()

    # Generate test matrices if needed
    generate_test_matrices()
    # generate_edge_case_matrices()
    
    # Run matrix tests
    run_tests()
    
    # Run edge case tests
    # run_edge_case_tests()
