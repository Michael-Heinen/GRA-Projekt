#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <linux/time.h>


#include "ellpack.h"
#include "matrix_io.h"

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

void print_usage(const char *progname)
{
    fprintf(stderr, usage_msg, progname, progname, progname);
}

void print_help(const char *progname)
{
    print_usage(progname);
    fprintf(stderr, "\n%s", help_msg);
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
            exit(EXIT_FAILURE);
        }
    }

    if (!input_file_a || !input_file_b || !output_file)
    {
        fprintf(stderr, "Wrong input/output formatting: Input and output files must be specified.\n");
        exit(EXIT_FAILURE);
    }

    ELLPACKMatrix matrix_a, matrix_b, result;
    read_matrix(input_file_a, &matrix_a);
    read_matrix(input_file_b, &matrix_b);

    //start clock
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

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
        fprintf(stderr, "Unknown version specified.\n");
        exit(EXIT_FAILURE);
    }

    //stop clock
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);


    double time = end.tv_sec - start.tv_sec + 1e-9 * (end.tv_nsec - start.tv_nsec);

    uint64_t new_noNonZero = compute_noNonZero(&result);

    write_matrix(output_file, &result, new_noNonZero);

    free(matrix_a.values);
    free(matrix_a.indices);
    free(matrix_b.values);
    free(matrix_b.indices);
    free(result.values);
    free(result.indices);

    return EXIT_SUCCESS;
}
