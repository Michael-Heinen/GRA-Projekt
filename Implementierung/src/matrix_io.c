#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
// #include <stdint.h>

void read_matrix(const char *filename, ELLPACKMatrix *matrix)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error opening file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fscanf(file, "%" SCNd64 ",%" SCNd64 ",%" SCNd64, &matrix->noRows, &matrix->noCols, &matrix->noNonZero);

    matrix->values = (float *)malloc(matrix->noRows * matrix->noNonZero * sizeof(float));
    matrix->indices = (int64_t *)malloc(matrix->noRows * matrix->noNonZero * sizeof(int64_t));

    char ch;
    for (int64_t i = 0; i < matrix->noRows * matrix->noNonZero; ++i)
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
    for (int64_t i = 0; i < matrix->noRows * matrix->noNonZero; ++i)
    {
        if (fscanf(file, " %c", &ch) == 1 && ch == '*')
        {
            matrix->indices[i] = -1;
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

void write_matrix(const char *filename, const ELLPACKMatrix *matrix, int64_t new_noNonZero)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        fprintf(stderr, "Error opening file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%" PRId64 ",%" PRId64 ",%" PRId64 "\n", matrix->noRows, matrix->noCols, new_noNonZero);

    for (int64_t i = 0; i < matrix->noRows; ++i)
    {
        for (int64_t j = 0; j < new_noNonZero; j++)
        {
            if (matrix->values[i * matrix->noNonZero + j] == 0.0f)
            {
                fprintf(file, "%c", '*');
            }
            else
            {
                fprintf(file, "%.1f", matrix->values[i * matrix->noNonZero + j]);
            }
            if (i * matrix->noNonZero + j < matrix->noRows * matrix->noNonZero - 1)
            {
                fprintf(file, ",");
            }
        }
    }
    fprintf(file, "\n");

    for (int64_t i = 0; i < matrix->noRows; ++i)
    {
        for (int64_t j = 0; j < new_noNonZero; j++)
        {
            if (matrix->indices[i * matrix->noNonZero + j] < 0)
            {
                fprintf(file, "%c", '*');
            }
            else
            {
                fprintf(file, "%" PRId64, matrix->indices[i * matrix->noNonZero + j]);
            }
            if (i * matrix->noNonZero + j < matrix->noRows * matrix->noNonZero - 1)
            {
                fprintf(file, ",");
            }
        }
    }
    // fprintf(file, "\n");

    fclose(file);
}