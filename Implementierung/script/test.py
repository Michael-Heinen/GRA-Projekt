import os
import subprocess
import numpy as np
import argparse
import random
import signal
import json
from datetime import datetime

from matrix_mult.config import *
from matrix_mult.plotter import plot_performance_results

# Function to compile the implementations
def compile_implementations():
    try:
        subprocess.run(["make"], check=True, cwd=BASE_DIR).stdout
    except subprocess.CalledProcessError as e:
        print(e.output)

# Function to parse execution time from the main program output
def parse_execution_time(output):
    for line in output.splitlines():
        if "Average execution time:" in line:
            return float(line.split(":")[1].strip().split()[0])
    return None

# Function to save matrix to file
def save_matrix_row_by_row(rows, cols, density, filename):
    max_nonzeros = 0
    row_values_list = []
    row_indices_list = []

    with open(filename, 'w') as f:
        # First pass: Generate the matrix and determine max_nonzeros
        for _ in range(rows):
            row_values = []
            row_indices = []

            # Generating random float values
            for col in range(cols):
                if random.random() < density:
                    value = random.uniform(-1,1)  # Generates a float in the range [-1.0, 1.0]
                    row_values.append(value)
                    row_indices.append(col)
            
            max_nonzeros = max(max_nonzeros, len(row_values))
            row_values_list.append(row_values)
            row_indices_list.append(row_indices)

        # write ELLPACK Metadata into first line
        f.write(f"{rows},{cols},{max_nonzeros}\n")

        # edge case: if null matrix
        if(max_nonzeros == 0):
            f.write("\n\n")
            return

        # write values in second line
        row_lines = []
        for row_values in row_values_list:
            filtered_values = [str(v) for v in row_values] + ["*"] * (max_nonzeros - len(row_values))
            row_lines.append(",".join(filtered_values))
        f.write(",".join(row_lines) + "\n")

        # write indices in third line
        index_lines = []
        for row_indices in row_indices_list:
            filtered_indices = [str(i) for i in row_indices] + ["*"] * (max_nonzeros - len(row_indices))
            index_lines.append(",".join(filtered_indices))
        f.write(",".join(index_lines))
    

# Deleting files for a specific directory
def delete_files_in_directory(directory_path):
    try:
        files = os.listdir(directory_path)
        for file in files:
            file_path = os.path.join(directory_path, file)
            if os.path.isfile(file_path):
                os.remove(file_path)
        print(f"All files in {directory_path} deleted successfully.")
    except OSError:
        print("Error occurred while deleting files.")

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
        if not((os.path.exists(matrix_a_filename) and os.path.exists(matrix_b_filename))):
            rows, cols = case_params[0], case_params[1]
            density = 0.8
            if(len(case_params) == 3):
                density = float(case_params[2])
            save_matrix_row_by_row(rows, cols, density, matrix_a_filename)
            save_matrix_row_by_row(rows, cols, density, matrix_b_filename)

# load and clean matrixes for comparison
def load_matrix_values_from_file(filename):
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
                    values[row_counter, col_index] = np.float32(val)
                count += 1
                if count >= max_nonzeros:
                    count = 0
                    row_counter += 1
        except ValueError as e:
            return values
        
    return values

# Function to compare matrices
def compare_matrices(matrix1, matrix2):
    try:
        if(TESTING):
            print(f"Matrix of own Implementation: \n{matrix1}")
            print(f"Matrix of comparison: \n{matrix2}")
        return np.allclose(matrix1, matrix2,  rtol=1e-04, atol=1e-05)
    except Exception as e:
        print(f"Comparison error: {e}")
        return False
    
# Function to handle interrupts and save results
def save_results_on_interrupt(signal, frame, results, filename):
    print("\nInterrupt received, saving results...")
    save_results(results, filename)
    print("Results saved. Exiting.")
    exit(0)

# Function to save results to a JSON file
def save_results(results, filename):
    with open(filename, 'w') as f:
        json.dump(results, f, indent=4)

# Function to run one isolated instance of our C ELLPACK Matrix multiplication
def run_isolated_test(command, timeout):
    try:
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=BASE_DIR)
        stdout, stderr = process.communicate(timeout)
        return process.returncode, stdout, stderr, False
    except subprocess.TimeoutExpired:
        process.kill()
        stdout, stderr = process.communicate()
        return -1, stdout, stderr, True

# Function to run the all tests based on specified arguments or default arguments
def run_tests(num_runs, timeout, densities, results_filename):
    performance_results = {density: {impl: [] for impl in IMPLEMENTATIONS} for density in densities}
    timed_out_versions = set()

    # Register interrupt by Keyboard Interruption ->  save results
    signal.signal(signal.SIGINT, lambda sig, frame: save_results_on_interrupt(sig, frame, performance_results, results_filename))
    
    for size in MATRIX_SIZES:
        for density in densities:
            matrix_a_filename = os.path.join(TEST_MATRICES_DIR, f"matrixA_{size}x{size}_D{density}.txt")
            matrix_b_filename = os.path.join(TEST_MATRICES_DIR, f"matrixB_{size}x{size}_D{density}.txt")

            for impl in IMPLEMENTATIONS:
                if impl in timed_out_versions:
                    continue

                print(f"\nTesting V{impl} with matrix size {size}x{size} and density {density}")
                execution_times = []

                try:
                    command = [os.path.join(BASE_DIR, "main"), f"-V {impl}", f"-B{num_runs}", f"-a{matrix_a_filename}", f"-b{matrix_b_filename}", f"-o{os.path.join(RESULTS_DIR, f'result_V{impl}_{size}x{size}.txt')}"]
                    returncode, stdout, stderr, timed_out = run_isolated_test(command, timeout)
                    if timed_out:
                        print(f"Run V{impl} timed out.")
                        timed_out_versions.add(impl)

                        break  # Skip further runs for this implementation
                    if returncode == 0:
                        execution_time = parse_execution_time(stdout.decode())
                        if execution_time is not None:
                            execution_times.append(execution_time)
                        else:
                            print("Failed to parse execution time from the output.")
                    else:
                        print(f"Error: {stderr.decode().strip()}")
                except Exception as e:
                    print(f"Execution error: {e}")

                if execution_times:
                    avg_time = sum(execution_times) / len(execution_times)
                    performance_results[density][impl].append(avg_time)
                    print(f"Average Execution Time for V{impl} with matrix size {size}x{size} and density {density}: {avg_time:.6f} seconds")

                    if COMPARE:
                        # Check correctness with matrixmultiplication module of numpy
                        np_matrix_mul = np.matmul(load_matrix_values_from_file(matrix_a_filename), load_matrix_values_from_file(matrix_b_filename), dtype=np.float32)
                        result_path = os.path.join(RESULTS_DIR, f"result_V{impl}_{size}x{size}.txt")
                        if returncode == 0 and compare_matrices(load_matrix_values_from_file(result_path), np_matrix_mul):
                            print(f"Output correctness: PASSED")
                        else:
                            print(f"Output correctness: FAILED")

                # Save results after each implementation
                save_results(performance_results, results_filename)

    return performance_results

# Function to run edge case tests
def run_edge_case_tests(timeout):
    for case_name, case_params in EDGE_CASES:
        matrix_a_filename = os.path.join(EDGE_MATRICES_DIR, f"edge_case_{case_name}_A.txt")
        matrix_b_filename = os.path.join(EDGE_MATRICES_DIR, f"edge_case_{case_name}_B.txt")

        for impl in IMPLEMENTATIONS:
            print(f"\nTesting V{impl} with edge case '{case_name}'")
            try:
                command = [os.path.join(BASE_DIR, "main"), f"-V {impl}", "-B", f"-a{matrix_a_filename}", f"-b{matrix_b_filename}", f"-o{os.path.join(RESULTS_DIR, f'result_edge_case_{case_name}_V{impl}.txt')}"]
                returncode, stdout, stderr = run_isolated_test(command, timeout)
                if returncode == 0:
                    print(f"Edge case '{case_name}' Execution: PASSED")
                else:
                    print(f"Edge case '{case_name}' Execution Error: {stderr.decode().strip()}")
            except Exception as e:
                print(f"Edge case '{case_name}' Execution error: {e}")

def print_parameters(args):
    print("Execution Parameters:")
    print(f"Versions: {args.versions}")
    print(f"Densities: {args.density}")
    print(f"Matrix Sizes: {args.matrix_sizes}")
    print(f"Number of Runs: {args.num_runs}")
    print(f"Timeout: {args.timeout}")
    print(f"Compile: {args.compile}")
    print(f"Generate New Matrices: {args.generate}")
    print(f"Test Edge Cases: {args.edge}")
    print(f"Plot Results: {args.plot}")
    print(f"JSON Filename: {args.json}")
    print(f"Compare with Numpy: {args.compare}")
    print(f"Testing Mode: {args.testing}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Matrix Multiplication Performance Testing')
    parser.add_argument('-V','--versions', type=int, nargs='+', default=[0, 1, 2], help='List of Versions to test (0-2)')
    parser.add_argument('-d','--density', type=float, nargs='+', default=[0.2, 0.5, 0.8], help='List of density for generated matrices (0.0-1.0)')
    parser.add_argument('-ms','--matrix_sizes', type=int, nargs='+', default=[8, 16, 32, 64, 128, 256, 512,750, 1024, 1265, 1535, 1794 ,2048, 2564, 3064, 3465, 4096, 6045, 8054, 10564, 12354], help='List of matrix sizes (int)')
    parser.add_argument('-n','--num_runs', type=int, default=3, help='Number of runs for each test (int)')
    parser.add_argument('-tmo','--timeout', type=int, default=1800, help='Specify Timeout in seconds, to prevent executing long execution times of a algorithm')

    parser.add_argument('-c', '--compile', action='store_false', help='Does NOT compile the implementations')
    parser.add_argument('-g', '--generate', action='store_true', help='Deletes old test matrices, and generates new ones')
    parser.add_argument('-e', '--edge', action='store_true', help='Test edge case matrices')
    
    parser.add_argument('-p', '--plot', action='store_false', help='Does NOT Plot performance results')
    parser.add_argument('-j','--json', type=str, default=datetime.now().strftime("%Y-%m-%d_%H:%M:%S"), help='Specify output filename of json file')
    parser.add_argument('-cmp', '--compare', action='store_false', help='Do NOT verify correctnes with numpy matrix multiplication')
    parser.add_argument('-t', '--testing', action='store_true', help='Print Comparison Matrizes to see differences')
    args = parser.parse_args()
    print_parameters(args)

    MATRIX_SIZES = args.matrix_sizes
    IMPLEMENTATIONS = args.versions
    TESTING = args.testing
    COMPARE = args.compare

    # delete_files_in_directory(RESULTS_DIR)

    # Compile the implementations
    if args.compile:
        compile_implementations()
    
    # delete test_matrices files if they should be newly generated 
    if args.generate:
        delete_files_in_directory(TEST_MATRICES_DIR)

    # Testing the script by each density provided
    for density in args.density:
        # Generate test matrices if needed
        generate_test_matrices(density)

    results_filename = os.path.join(BASE_DIR, f'performance_results_{args.json}.json')
    # Run matrix tests
    performances = run_tests(args.num_runs, args.timeout, args.density, results_filename)
    # Plot performance results
    if args.plot:
        plot_performance_results(results_filename, args.density, args.matrix_sizes, args.versions, args.timeout, args.num_runs)
    
    # Run edge case tests
    if(args.edge):
        generate_edge_case_matrices()
        run_edge_case_tests(args.timeout)