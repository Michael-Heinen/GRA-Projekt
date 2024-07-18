#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
// #include <stdint.h>

int read_matrix(const char *filename, ELLPACKMatrix *matrix)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error opening file %s\n", filename);
        return -1;
    }

    if (fscanf(file, "%" SCNu64 ",%" SCNu64 ",%" SCNu64, &matrix->noRows, &matrix->noCols, &matrix->noNonZero) != 3)
    {
        fprintf(stderr, "Error reading matrix dimensions from file %s\n", filename);
        fclose(file);
        return -1;
    }

    matrix->values = (float *)malloc(matrix->noRows * matrix->noNonZero * sizeof(float));
    matrix->indices = (uint64_t *)malloc(matrix->noRows * matrix->noNonZero * sizeof(uint64_t));

    if (!matrix->values || !matrix->indices)
    {
        fprintf(stderr, "Memory allocation failed\n");
        free(matrix->values);
        free(matrix->indices);
        fclose(file);
        return -1;
    }

    for (uint64_t i = 0; i < matrix->noRows * matrix->noNonZero; ++i)
    {
        char ch;
        if (fscanf(file, " %c", &ch) != 1)
        {
            fprintf(stderr, "Error reading values from file %s\n", filename);
            free(matrix->values);
            free(matrix->indices);
            fclose(file);
            return -1;
        }

        if (ch == '*')
        {
            matrix->values[i] = 0.0f;
            if (fscanf(file, "%*c") != 0)
            {
                fprintf(stderr, "Error skipping the comma (value) %s\n", filename);
            }
        } else {
            ungetc(ch, file); // Put back the character if it's not '*'
            if (fscanf(file, "%f,", &matrix->values[i]) != 1) {
                fprintf(stderr, "Error reading value from file %s\n", filename);
                free(matrix->values);
                free(matrix->indices);
                fclose(file);
                return -1;
            }
        }
    }
    for (uint64_t i = 0; i < matrix->noRows * matrix->noNonZero; ++i)
    {
        char ch;
        if (fscanf(file, " %c", &ch) != 1)
        {
            fprintf(stderr, "Error reading indices from file %s\n", filename);
            free(matrix->values);
            free(matrix->indices);
            fclose(file);
            return -1;
        }

        if (ch == '*')
        {
            matrix->indices[i] = 0;
            if (fscanf(file, "%*c") != 0)
            {
                fprintf(stderr, "Error skipping the comma (index) %s\n", filename);
            }
        } else {
            ungetc(ch, file);
            if (fscanf(file, "%" SCNu64 ",", &matrix->indices[i]) != 1)
            {
                fprintf(stderr, "Error reading index from file %s\n", filename);
                free(matrix->values);
                free(matrix->indices);
                fclose(file);
                return -1;
            }
        }
    }
    fclose(file);
    return 0;
}

int write_matrix(const char *filename, const ELLPACKMatrix *matrix, uint64_t new_noNonZero)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        fprintf(stderr, "Error opening file %s\n", filename);
        return -1;
    }

    fprintf(file, "%" PRId64 ",%" PRId64 ",%" PRId64 "\n", matrix->noRows, matrix->noCols, new_noNonZero);

    for (uint64_t i = 0; i < matrix->noRows; ++i)
    {
        for (uint64_t j = 0; j < new_noNonZero; j++)
        {
            if (matrix->values[i * matrix->noNonZero + j] == 0.0f)
            {
                fprintf(file, "%c", '*');
            }
            else
            {
                fprintf(file, "%f", matrix->values[i * matrix->noNonZero + j]);
            }
            if (i * new_noNonZero + j < matrix->noRows * new_noNonZero - 1)
            {
                fprintf(file, ",");
            }
        }
    }
    fprintf(file, "\n");

    for (uint64_t i = 0; i < matrix->noRows; ++i)
    {
        for (uint64_t j = 0; j < new_noNonZero; j++)
        {
            if (matrix->values[i * matrix->noNonZero + j] == 0.0f)
            {
                fprintf(file, "%c", '*');
            }
            else
            {
                fprintf(file, "%" PRId64, matrix->indices[i * matrix->noNonZero + j]);
            }
            if (i * new_noNonZero + j < matrix->noRows * new_noNonZero - 1)
            {
                fprintf(file, ",");
            }
        }
    }
    // fprintf(file, "\n");

    fclose(file);
    return 0;
}


// compute noNonZero in result matrix
int compute_noNonZero(ELLPACKMatrix *matrix)
{
    uint64_t maxNoNonZero = 0;
    for (uint64_t i = 0; i < matrix->noCols; i++)
    {
        uint64_t tmpNoNonZero = 0;
        for (uint64_t j = 0; j < matrix->noNonZero; j++)
        {
            if (matrix->values[i * matrix->noNonZero + j] == 0.0f)
            {
                break;
            }
            tmpNoNonZero++;
        }
        if (tmpNoNonZero > maxNoNonZero)
        {
            maxNoNonZero = tmpNoNonZero;
        }
    }
    return maxNoNonZero;
}