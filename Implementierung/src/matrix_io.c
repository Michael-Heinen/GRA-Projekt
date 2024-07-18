#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
// #include <stdint.h>

int read_matrix(const char *filename, ELLPACKMatrix *matrix)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error opening file %s\n", filename);
        return -1;
    }

    //char line[(INT64_MAX-1)*(INT64_MAX-1)];
    char line[65000];
    long file_pos_gets;
    long file_pos_scan;

    //line 1
    if (!fgets(line, sizeof(line), file)) {
        fprintf(stderr, "Error reading matrix dimensions\n");
        fclose(file);
        return -1;
    }

    if (count_numbers_in_line(&line) != 3)
    {
        fprintf(stderr, "Wrong Number in line 1.\n");
        return -1;
    }

    file_pos_gets = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fscanf(file, "%" SCNu64 ",%" SCNu64 ",%" SCNu64, &matrix->noRows, &matrix->noCols, &matrix->noNonZero) != 3)
    {
        fprintf(stderr, "Error reading matrix dimensions. Filename: %s\n", filename);
        fclose(file);
        return -1;
    }

    if (matrix->noRows == 0 || matrix->noCols == 0)
    {
        fprintf(stderr, "Error: Rows or Cols equals 0. Filename: %s\n", filename);
        fclose(file);
        return -1;
    }

    if (matrix->noRows > INT64_MAX || matrix->noCols > INT64_MAX || matrix->noNonZero > INT64_MAX)
    {
        fprintf(stderr, "Error: Matrix dimensions exceed maximum allowed value. Filename: %s\n", filename);
        fclose(file);
        return -1;
    }

    if (matrix->noRows < matrix->noNonZero)
    {
        fprintf(stderr, "Error: noNonZero larger then noRows. Rows: %ld, noNonZero: %ld. Filename: %s\n", matrix->noRows, matrix->noNonZero, filename);
        fclose(file);
        return -1;
    }
    file_pos_scan = ftell(file);
    fseek(file, file_pos_gets, SEEK_SET);

    //line 2
    if (!fgets(line, sizeof(line), file)) {
        fprintf(stderr, "Error reading matrix dimensions\n");
        fclose(file);
        return -1;
    }

    if (count_numbers_in_line(line) != (int)(matrix->noRows * matrix->noNonZero))
    {
        fprintf(stderr, "Wrong Number in line 2.\n");
        return -1;
    }

    //line 3
    if (!fgets(line, sizeof(line), file)) {
        fprintf(stderr, "Error reading matrix dimensions\n");
        fclose(file);
        return -1;
    }

    if (count_numbers_in_line(line) != (int)(matrix->noRows * matrix->noNonZero))
    {
        fprintf(stderr, "Wrong Number in line 3.\n");
        return -1;
    }

    fseek(file, file_pos_scan, SEEK_SET);

    matrix->values = (float *)malloc(matrix->noRows * matrix->noNonZero * sizeof(float));
    matrix->indices = (uint64_t *)malloc(matrix->noRows * matrix->noNonZero * sizeof(uint64_t));

    if (!matrix->values || !matrix->indices)
    {
        fprintf(stderr, "Memory allocation failed. Filename: %s\n", filename);
        fclose(file);
        return -1;
    }

    for (uint64_t i = 0; i < matrix->noRows * matrix->noNonZero; ++i)
    {
        char ch;
        if (fscanf(file, " %c", &ch) != 1)
        {
            fprintf(stderr, "Error reading values from file %s\n", filename);
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

int count_numbers_in_line(const char *line) {

    int count = 0;
    char *token = strtok(line, ",");
    while (token != NULL) {
        count++;
        token = strtok(NULL, ",");
    }

    return count;
}

int control_indices(const char *filename, const ELLPACKMatrix *matrix)
{
    for (uint64_t i = 0; i < matrix->noCols; i++)
    {
        bool* temp_array = (bool *)calloc(matrix->noRows, sizeof(bool));
        bool secondZero = false;
            for(uint64_t j = 0; j < matrix->noNonZero; j++)
            {
                if (temp_array[matrix->indices[i * matrix->noNonZero + j]] && matrix->indices[i * matrix->noNonZero + j] != 0 || secondZero && matrix->indices[i * matrix->noNonZero + j] != 0)
                {
                    fprintf(stderr,"Error: Double indices in Rows.\n", filename);
                    return -1;
                }
                else if (matrix->indices[i * matrix->noNonZero + j] >= matrix->noRows)
                {
                    fprintf(stderr,"Error: Index larger then Rows (Index out of Bound). Filename: %s\n", filename);
                    return -1;
                }
                else
                {
                    if(matrix->indices[i* matrix->noNonZero + j] == 0 && temp_array[matrix->indices[i*matrix->noNonZero + j]])
                    {
                        secondZero = true;
                    }
                    else
                    {
                        temp_array[matrix->indices[i * matrix->noNonZero + j]] = true;
                    }
                }
            }
        free(temp_array);
    }

    return 0;
}