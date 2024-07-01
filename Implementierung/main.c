#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include <getopt.h>
#include <stdbool.h>

typedef struct
{
    uint64_t noRows;
    uint64_t noCols;
    uint64_t noNonZero;
    float *values;
    uint64_t *indices;
} ELLPACKMatrix;

void read_matrix(const char *filename, ELLPACKMatrix *matrix)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error opening file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fscanf(file, "%" SCNu64 ",%" SCNu64 ",%" SCNu64, &matrix->noRows, &matrix->noCols, &matrix->noNonZero);

    matrix->values = (float *)malloc(matrix->noRows * matrix->noNonZero * sizeof(float));
    matrix->indices = (uint64_t *)malloc(matrix->noRows * matrix->noNonZero * sizeof(uint64_t));

    char ch;
    for (uint64_t i = 0; i < matrix->noRows * matrix->noNonZero; ++i)
    {
        if (fscanf(file, " %c", &ch) == 1 && ch == '*')
        {
            matrix->values[i] = 0.0f;
            // Skip the comma
            fscanf(file, "%*c");
        }
        else
        {
            ungetc(ch, file); // Put back the character if it's not '*'
            fscanf(file, "%f,", &matrix->values[i]);
        }
    }
    for (uint64_t i = 0; i < matrix->noRows * matrix->noNonZero; ++i)
    {
        if (fscanf(file, " %c", &ch) == 1 && ch == '*')
        {
            matrix->indices[i] = 0;
            // Skip the comma
            fscanf(file, "%*c");
        }
        else
        {
            ungetc(ch, file); // Put back the character if it's not '*'
            fscanf(file, "%" SCNu64 ",", &matrix->indices[i]);
        }
    }

    fclose(file);
}

void write_matrix(const char *filename, const ELLPACKMatrix *matrix)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        fprintf(stderr, "Error opening file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%" PRIu64 ",%" PRIu64 ",%" PRIu64 "\n", matrix->noRows, matrix->noCols, matrix->noNonZero);

    for (uint64_t i = 0; i < matrix->noRows * matrix->noNonZero; ++i)
    {
        if (matrix->values[i] == 0.0f)
        {
            fprintf(file, "%c", '*');
        }
        else
        {
            fprintf(file, "%.1f", matrix->values[i]);
        }
        if (i < matrix->noRows * matrix->noNonZero - 1)
        {
            fprintf(file, ",");
        }
    }
    fprintf(file, "\n");

    for (uint64_t i = 0; i < matrix->noRows * matrix->noNonZero; ++i)
    {
        fprintf(file, "%" PRIu64, matrix->indices[i]);
        if (i < matrix->noRows * matrix->noNonZero - 1)
        {
            fprintf(file, ",");
        }
    }
    // fprintf(file, "\n");

    fclose(file);
}

void matr_mult_ellpack(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result)
{
    if (a->noCols != b->noRows)
    {
        fprintf(stderr, "Matrix dimensions do not match for multiplication\n");
        exit(EXIT_FAILURE);
    }

    result->noRows = a->noRows;
    result->noCols = b->noCols;
    result->noNonZero = b->noNonZero;

    result->values = (float *)calloc(result->noRows * result->noNonZero, sizeof(float));
    result->indices = (uint64_t *)calloc(result->noRows * result->noNonZero, sizeof(uint64_t));

    for (uint64_t i = 0; i < a->noRows; ++i)
    {
        for (uint64_t k = 0; k < a->noNonZero; ++k)
        {
            uint64_t a_index = i * a->noNonZero + k;
            float a_value = a->values[a_index];
            uint64_t a_col = a->indices[a_index];

            for (uint64_t l = 0; l < b->noNonZero; ++l)
            {
                uint64_t b_index = a_col * b->noNonZero + l;
                float b_value = b->values[b_index];
                uint64_t b_col = b->indices[b_index];

                result->values[i * result->noNonZero + l + k] += a_value * b_value;
                result->indices[i * result->noNonZero + l + k] = b_col;
            }
        }
    }
}

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

    matr_mult_ellpack(&matrix_a, &matrix_b, &result);

    write_matrix(output_file, &result);

    free(matrix_a.values);
    free(matrix_a.indices);
    free(matrix_b.values);
    free(matrix_b.indices);
    free(result.values);
    free(result.indices);

    return EXIT_SUCCESS;
}
