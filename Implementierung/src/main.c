#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "ellpack.h"
#include "matrix_io.h"
#include <unistd.h> // sleep

// help and info messages
const char *usage_msg =
    "Help Message (Usage): "
    "./matrix_mult [-h] [-V version] [-B [iterations]] -a inputA -b inputB -o output\n";

const char *help_msg =
    "Help Message:\n"
    "Positional arguments:\n"
    "  -a, --input_a FILE     Input file for matrix A\n"
    "  -b, --input_b FILE     Input file for matrix B\n"
    "  -o, --output FILE      Output file for the result matrix\n"
    "\n"
    "Optional arguments:\n"
    "  -h, --help             Display this help message and exit\n"
    "  -V, --version VERSION  Specify the version of the multiplication algorithm (default is 0)\n"
    "  -B, --benchmark [N]    Run benchmark with N iterations (default is 3)\n";

const char *help_input_files_format =
    "\n"
    "Inputs Format:\n"
    "LINE | CONTENT\n"
    "1 | <num_rows>,<num_cols>,<num_non_zero>\n"
    "2 | <values>\n"
    "3 | <indices>\n"
    "\n"
    "Example:"
    "4,4,2"
    "5,*,6,*,0.5,7,3,*"
    "0,*,1,*,0,1,3,*"
    "\n"
    "Lines 2 and 3 must contain the correct number of values\n";


// functions to print help and info messages
void print_input_files_format()
{
    fprintf(stdout, "%s", help_input_files_format);
}

void print_usage(const char *progname)
{
    fprintf(stdout, usage_msg, progname, progname, progname);
}

void print_help(const char *progname)
{
    print_usage(progname);
    fprintf(stdout, "\n%s", help_msg);
    print_input_files_format();
}


// function to free the allocated memory
void free_matrix(ELLPACKMatrix *matrix)
{
    if (matrix)
    {
        if (matrix->values)
        {
            free(matrix->values);
        }

        if (matrix->indices)
        {
            free(matrix->indices);
        }


        if (matrix->result_values)
        {
                for (uint64_t i = 0; i < matrix->num_rows; i++)
                {
                    if(matrix->result_values[i])
                    {
                        free(matrix->result_values[i]);
                    }
                }

                free(matrix->result_values);
        }

        if (matrix->result_indices)
        {
            for (uint64_t i = 0; i < matrix->num_rows; i++)
            {
                if(matrix->result_indices[i])
                {
                    free(matrix->result_indices[i]);
                }
            }

            free(matrix->result_indices);
        }
    }
}

// function to handle the errors central
void handle_error(const char *message, ELLPACKMatrix *matrix_a, ELLPACKMatrix *matrix_b, ELLPACKMatrix *result)
{
    free_matrix(matrix_a);
    free_matrix(matrix_b);
    free_matrix(result);

    if (errno == 0)
    {
        fprintf(stderr, "%s\n", message);
    }
    else
    {
        fprintf(stderr, "%s: Errno: %s\n", message, strerror(errno));
    }

    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    const char *progname = argv[0];

    int opt;
    char *input_file_a = NULL, *input_file_b = NULL, *output_file = NULL;
    int version = 0, benchmark = 0;

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"version", required_argument, 0, 'V'},
        {"benchmark", optional_argument, 0, 'B'},
        {"input_a", required_argument, 0, 'a'},
        {"input_b", required_argument, 0, 'b'},
        {"output", required_argument, 0, 'o'},
        {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, "hV:B::a:b:o:", long_options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'h':
            print_help(progname);
            exit(0);
        case 'V':
            version = atoi(optarg);
            break;
        case 'B':
            benchmark = optarg ? atoi(optarg) : 1;
            printf("Benchmark value: %d.\n", benchmark);
            break;
        case 'a':
            input_file_a = optarg;
            break;
        case 'b':
            input_file_b = optarg;
            break;
        case 'o':
            output_file = optarg;
            break;
        default:
            print_usage(progname);
            handle_error("Wrong format: look at Help Message (Usage)", NULL, NULL, NULL);
        }
    }

    if (!input_file_a || !input_file_b || !output_file)
    {
        handle_error("Error: Wrong input/output formatting: Input and output files must be specified", NULL, NULL, NULL);
        print_usage(progname);
    }

    // reading the ELLPACK input files into the ELLPACKMatrix struct and after that control_indices check the correctness of the input indices
    ELLPACKMatrix matrix_a = {0}, matrix_b = {0}, result = {0};

    if (read_matrix(input_file_a, &matrix_a) != 0)
    {
        handle_error("Error reading input matrix A", &matrix_a, NULL, NULL);
    }

    if (read_matrix(input_file_b, &matrix_b) != 0)
    {
        handle_error("Error reading input matrix B", &matrix_a, &matrix_b, NULL);
    }

    if (control_indices(input_file_a, &matrix_a) != 0)
    {
        handle_error("in control_indices_inputs (A)", &matrix_a, &matrix_b, NULL);
    }

    if (control_indices(input_file_b, &matrix_b) != 0)
    {
        handle_error("in control_indices (B)", &matrix_a, &matrix_b, NULL);
    }



    // takes the start time for the time measurement
    struct timespec clock_start_time;
    if (clock_gettime(CLOCK_MONOTONIC, &clock_start_time) != 0)
    {
        handle_error("Error getting clock time", &matrix_a, &matrix_b, NULL);
    }

    // sleep(1) makes sure there is one second between the timing
    sleep(1);

    // the switch-case block starts the entered version (getopt: -V). If nothing has been entered, version 0 is always executed
    switch (version)
    {
    case 0:
        matr_mult_ellpack(&matrix_a, &matrix_b, &result);
        break;
    case 1:
        matr_mult_ellpack_V1(&matrix_a, &matrix_b, &result);
        break;
    case 2:
        matr_mult_ellpack_V2(&matrix_a, &matrix_b, &result);
        break;
    default:
        handle_error("Unknown version specified", &matrix_a, &matrix_b, NULL);
    }

    // takes the end time for the time measurement
    struct timespec clock_end_time;
    if (clock_gettime(CLOCK_MONOTONIC, &clock_end_time) != 0)
    {
        handle_error("Error getting end time", &matrix_a, &matrix_b, &result);
    }

    // calculates the running time of the matrix multiplication and outputs (print) it
    double time = clock_end_time.tv_sec - clock_start_time.tv_sec + 1e-9 * (clock_end_time.tv_nsec - clock_start_time.tv_nsec);

    if (benchmark)
    {
        fprintf(stdout, "Execution time: %f seconds\n", time);
    }


    /*
    Calls the functions that create the output file.
    There are 2 Versions to create the output file. This is because the result arrays of the Versions are different.
    */
    if (version == 1)
    {
        uint64_t num_non_zero = compute_num_non_zero(&result);
        if (write_matrix_V1(output_file, &result, num_non_zero) != 0)
        {
            handle_error("Error writing output matrix", &matrix_a, &matrix_b, &result);
        }
    }
    else
    {
        if (write_matrix_V2(output_file, &result) != 0)
        {
            handle_error("Error writing output matrix", &matrix_a, &matrix_b, &result);
        }
    }

    // calls the function to free the allocated memory
    free_matrix(&matrix_a);
    free_matrix(&matrix_b);
    free_matrix(&result);

    return EXIT_SUCCESS;
}
