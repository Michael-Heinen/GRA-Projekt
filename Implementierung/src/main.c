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

void free_matrix(ELLPACKMatrix *matrix, int version)
{
    if (matrix)
    {
        if (version == 1)
        {
            free(matrix->values);
            free(matrix->indices);

        }
        else
        {
            if (matrix->result_values)
                {
                    for (uint64_t i = 0; i < matrix->num_cols; i++)
                    {
                        free(matrix->result_values[i]);
                    }

                    free(matrix->result_values);
                }

                if (matrix->result_indices)
                {
                    for (uint64_t i = 0; i < matrix->num_cols; i++)
                    {
                        free(matrix->result_indices[i]);
                    }

                    free(matrix->result_indices);
                }

                free(matrix->values);
                free(matrix->indices);
        }
    }
}

void handle_error(const char *message, ELLPACKMatrix *matrix_a, ELLPACKMatrix *matrix_b, ELLPACKMatrix *result, int version)
{
    free_matrix(matrix_a, version);
    free_matrix(matrix_b, version);
    free_matrix(result, version);

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
            handle_error("Wrong format: look at Help Message (Usage)", NULL, NULL, NULL, version);
        }
    }

    if (!input_file_a || !input_file_b || !output_file)
    {
        handle_error("Error: Wrong input/output formatting: Input and output files must be specified", NULL, NULL, NULL, version);
        print_usage(progname);
    }

    ELLPACKMatrix matrix_a = {0}, matrix_b = {0}, result = {0};

    if (read_matrix(input_file_a, &matrix_a) != 0)
    {
        handle_error("Error reading input matrix A", &matrix_a, NULL, NULL, version);
    }

    if (read_matrix(input_file_b, &matrix_b) != 0)
    {
        handle_error("Error reading input matrix B", &matrix_a, &matrix_b, NULL, version);
    }

    if (control_indices(input_file_a, &matrix_a) != 0)
    {
        handle_error("in control_indices_inputs (A)", &matrix_a, &matrix_b, NULL, version);
    }

    if (control_indices(input_file_b, &matrix_b) != 0)
    {
        handle_error("in control_indices (B)", &matrix_a, &matrix_b, NULL, version);
    }

    // clock_start_time clock
    struct timespec clock_start_time;
    if (clock_gettime(CLOCK_MONOTONIC, &clock_start_time) != 0)
    {
        handle_error("Error getting clock time", &matrix_a, &matrix_b, NULL, version);
    }

    // ensure one second between time measurement
    sleep(1);

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
        handle_error("Unknown version specified", &matrix_a, &matrix_b, NULL, version);
    }

    // stop clock
    struct timespec clock_end_time;
    if (clock_gettime(CLOCK_MONOTONIC, &clock_end_time) != 0)
    {
        handle_error("Error getting end time", &matrix_a, &matrix_b, &result, version);
    }

    double time = clock_end_time.tv_sec - clock_start_time.tv_sec + 1e-9 * (clock_end_time.tv_nsec - clock_start_time.tv_nsec);

    if (benchmark)
    {
        fprintf(stdout, "Execution time: %f seconds\n", time);
    }

    if (version == 1)
    {
        uint64_t num_non_zero = compute_num_non_zero(&result);
        if (write_matrix_V1(output_file, &result, num_non_zero) != 0)
        {
            handle_error("Error writing output matrix", &matrix_a, &matrix_b, &result, version);
        }
    }
    else
    {
        if (write_matrix_V2(output_file, &result) != 0)
        {
            handle_error("Error writing output matrix", &matrix_a, &matrix_b, &result, version);
        }
    }

    free_matrix(&matrix_a, version);
    free_matrix(&matrix_b, version);
    free_matrix(&result, version);

    return EXIT_SUCCESS;
}
