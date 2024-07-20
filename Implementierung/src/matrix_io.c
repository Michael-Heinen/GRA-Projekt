#include "ellpack.h"
#include "matrix_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/types.h>
#include <string.h>

int read_matrix(const char *restrict filename, ELLPACKMatrix *restrict matrix)
{
    // open input file
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error opening file %s\n", filename);
        return -1;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    long pos_getline_file;
    long pos_scan_file;

    // check first line of the file (count_number_in_line counts the numbers of values)
    if ((read = getline(&line, &len, file)) == -1)
    {
        fprintf(stderr, "Error reading line 1 (dimension).\n");
        free(line);
        fclose(file);
        return -1;
    }

    if (count_numbers_in_line(line) != 3)
    {
        fprintf(stderr, "Wrong number of characters in line 1 (dimension).\n");
        free(line);
        fclose(file);
        return -1;
    }

    // save "file getline position" to check the next lines and set file position to zero to read in the dimension (line 1)
    pos_getline_file = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fscanf(file, "%" SCNu64 ",%" SCNu64 ",%" SCNu64, &matrix->num_rows, &matrix->num_cols, &matrix->num_non_zero) != 3)
    {
        fprintf(stderr, "Error reading matrix dimensions. Filename: %s\n", filename);
        free(line);
        fclose(file);
        return -1;
    }

    // check all values of the dimension (rows and columns must not be 0 / rows,columns and num_non_zero must not be negative / num_non_zero must not be larger than the rows)
    if (matrix->num_rows == 0 || matrix->num_cols == 0)
    {
        fprintf(stderr, "Error: Rows or Cols equals 0. Filename: %s\n", filename);
        free(line);
        fclose(file);
        return -1;
    }

    if (matrix->num_rows > INT64_MAX || matrix->num_cols > INT64_MAX || matrix->num_non_zero > INT64_MAX)
    {
        fprintf(stderr, "Error: Matrix dimensions exceed maximum allowed value. Filename: %s\n", filename);
        free(line);
        fclose(file);
        return -1;
    }

    if (matrix->num_rows < matrix->num_non_zero)
    {
        fprintf(stderr, "Error: num_non_zero larger then num_rows. Rows: %ld, num_non_zero: %ld. Filename: %s\n", matrix->num_rows, matrix->num_non_zero, filename);
        free(line);
        fclose(file);
        return -1;
    }

    // save "file scan position" (reading position (fscanf)) and set file position to the old "file getline position"
    pos_scan_file = ftell(file);
    fseek(file, pos_getline_file, SEEK_SET);

    if (matrix->num_non_zero != 0)
    {
        // line 2
        if ((read = getline(&line, &len, file)) == -1)
        {
            fprintf(stderr, "Error reading line 2 (values).\n");
            free(line);
            fclose(file);
            return -1;
        }

        if (count_numbers_in_line(line) != (int)(matrix->num_rows * matrix->num_non_zero))
        {
            fprintf(stderr, "Wrong Number in line 2 (values).\n");
            free(line);
            fclose(file);
            return -1;
        }

        // line 3
        if ((read = getline(&line, &len, file)) == -1)
        {
            fprintf(stderr, "Error reading line 3 (indices).\n");
            free(line);
            fclose(file);
            return -1;
        }

        if (count_numbers_in_line(line) != (int)(matrix->num_rows * matrix->num_non_zero))
        {
            fprintf(stderr, "Wrong Number in line 3 (indices).\n");
            free(line);
            fclose(file);
            return -1;
        }

        if ((read = getline(&line, &len, file)) != -1)
        {
            fprintf(stderr, "Error: There are more lines as 3.\n");
            free(line);
            fclose(file);
            return -1;
        }
    }
    else if ((read = getline(&line, &len, file)) != -1)
    {
        if (*line != '\n')
        {
            fprintf(stderr, "Error: In the values line (line 2) are characters, but num_non_zero == 0.\n");
            free(line);
            fclose(file);
            return -1;
        }

        if ((read = getline(&line, &len, file)) != -1)
        {
            if (*line != '\n')
            {
                fprintf(stderr, "Error: In the indices line (line 3) are characters, but num_non_zero == 0.\n");
                free(line);
                fclose(file);
                return -1;
            }
        }

        if ((read = getline(&line, &len, file)) != -1)
        {
            fprintf(stderr, "Error: There are more lines as 3.\n");
            free(line);
            fclose(file);
            return -1;
        }
    }
    else
    {
        fprintf(stderr, "Error: There are less then 3 lines.\n");
        free(line);
        fclose(file);
        return -1;
    }

    // set file position to the old "file scan position" to scan the values and indices lines (line 2 and 3)
    fseek(file, pos_scan_file, SEEK_SET);

    matrix->values = (float *)malloc(matrix->num_rows * matrix->num_non_zero * sizeof(float));
    matrix->indices = (uint64_t *)malloc(matrix->num_rows * matrix->num_non_zero * sizeof(uint64_t));

    if (!matrix->values || !matrix->indices)
    {
        fprintf(stderr, "Memory allocation failed. Filename: %s\n", filename);
        free(line);
        fclose(file);
        return -1;
    }

    for (uint64_t i = 0; i < matrix->num_rows * matrix->num_non_zero; ++i)
    {
        char ch;
        if (fscanf(file, " %c", &ch) != 1)
        {
            fprintf(stderr, "Error reading values from file %s\n", filename);
            free(line);
            fclose(file);
            return -1;
        }

        if (ch == '*')
        {
            matrix->values[i] = 0.0f;
            if (fscanf(file, "%*c") != 0)
            {
                fprintf(stderr, "Error skipping the comma (value) %s\n", filename);
                free(line);
                fclose(file);
            }
        }
        else
        {
            ungetc(ch, file); // Put back the character if it's not '*'
            if (fscanf(file, "%f,", &matrix->values[i]) != 1)
            {
                fprintf(stderr, "Error reading value from file %s\n", filename);
                free(line);
                fclose(file);
                return -1;
            }
        }
    }
    for (uint64_t i = 0; i < matrix->num_rows * matrix->num_non_zero; ++i)
    {
        char ch;
        if (fscanf(file, " %c", &ch) != 1)
        {
            fprintf(stderr, "Error reading indices from file %s\n", filename);
            free(line);
            fclose(file);
            return -1;
        }

        if (ch == '*')
        {
            matrix->indices[i] = 0;
            if (fscanf(file, "%*c") != 0)
            {
                if (i != (matrix->num_rows * matrix->num_non_zero - 1))
                {
                    fprintf(stderr, "Error skipping the comma (index) %s\n", filename);
                    free(line);
                    fclose(file);
                    return -1;
                }
            }
        }
        else
        {
            ungetc(ch, file);
            if (fscanf(file, "%" SCNu64 ",", &matrix->indices[i]) != 1)
            {
                fprintf(stderr, "Error reading index from file %s\n", filename);
                free(line);
                fclose(file);
                return -1;
            }
        }
    }
    free(line);
    fclose(file);
    return 0;
}

int write_matrix_V1(const char *restrict filename, const ELLPACKMatrix *restrict matrix, uint64_t num_non_zero)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        fprintf(stderr, "Error opening file %s\n", filename);
        return -1;
    }

    fprintf(file, "%" PRId64 ",%" PRId64 ",%" PRId64 "\n", matrix->num_rows, matrix->num_cols, num_non_zero);

    for (uint64_t i = 0; i < matrix->num_rows; ++i)
    {
        for (uint64_t j = 0; j < num_non_zero; j++)
        {
            if (matrix->values[i * matrix->num_non_zero + j] == 0.0f)
            {
                fprintf(file, "%c", '*');
            }
            else
            {
                fprintf(file, "%f", matrix->values[i * matrix->num_non_zero + j]);
            }
            if (i * num_non_zero + j < matrix->num_rows * num_non_zero - 1)
            {
                fprintf(file, ",");
            }
        }
    }
    fprintf(file, "\n");

    for (uint64_t i = 0; i < matrix->num_rows; ++i)
    {
        for (uint64_t j = 0; j < num_non_zero; j++)
        {
            if (matrix->values[i * matrix->num_non_zero + j] == 0.0f)
            {
                fprintf(file, "%c", '*');
            }
            else
            {
                fprintf(file, "%" PRId64, matrix->indices[i * matrix->num_non_zero + j]);
            }
            if (i * num_non_zero + j < matrix->num_rows * num_non_zero - 1)
            {
                fprintf(file, ",");
            }
        }
    }

    fclose(file);
    return 0;
}

int write_matrix_V2(const char *restrict filename, const ELLPACKMatrix *restrict matrix)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        fprintf(stderr, "Error opening file %s\n", filename);
        return -1;
    }

    fprintf(file, "%" PRId64 ",%" PRId64 ",%" PRId64 "\n", matrix->num_rows, matrix->num_cols, matrix->num_non_zero);

    for (uint64_t i = 0; i < matrix->num_rows; ++i)
    {
        for (uint64_t j = 0; j < matrix->num_non_zero; j++)
        {
            if (matrix->result_values[i][j] == 0.0f)
            {
                fprintf(file, "%c", '*');
            }
            else
            {
                fprintf(file, "%f", matrix->result_values[i][j]);
            }
            if (i * matrix->num_non_zero + j < matrix->num_rows * matrix->num_non_zero - 1)
            {
                fprintf(file, ",");
            }
        }
    }
    fprintf(file, "\n");

    for (uint64_t i = 0; i < matrix->num_rows; ++i)
    {
        for (uint64_t j = 0; j < matrix->num_non_zero; j++)
        {
            if (matrix->result_values[i][j] == 0.0f)
            {
                fprintf(file, "%c", '*');
            }
            else
            {
                fprintf(file, "%" PRId64, matrix->result_indices[i][j]);
            }
            if (i * matrix->num_non_zero + j < matrix->num_rows * matrix->num_non_zero - 1)
            {
                fprintf(file, ",");
            }
        }
    }
    fclose(file);
    return 0;
}

// compute num_non_zero in result matrix
int compute_num_non_zero(ELLPACKMatrix *restrict matrix)
{
    uint64_t max_num_non_zero = 0;
    for (uint64_t i = 0; i < matrix->num_cols; i++)
    {
        uint64_t tmp_num_non_zero = 0;
        for (uint64_t j = 0; j < matrix->num_non_zero; j++)
        {
            if (matrix->values[i * matrix->num_non_zero + j] == 0.0f)
            {
                break;
            }
            tmp_num_non_zero++;
        }
        if (tmp_num_non_zero > max_num_non_zero)
        {
            max_num_non_zero = tmp_num_non_zero;
        }
    }
    return max_num_non_zero;
}

int count_numbers_in_line(char *restrict line)
{
    int count = 0;
    char *token = strtok(line, ",");

    while (token != NULL)
    {
        count++;
        token = strtok(NULL, ",");
    }

    return count;
}

int control_indices(const char *filename, const ELLPACKMatrix *restrict matrix)
{
    for (uint64_t i = 0; i < matrix->num_cols; i++)
    {
        bool *temp_array = (bool *)calloc(matrix->num_rows, sizeof(bool));
        bool first_zero = false;
        bool second_zero = false;

        for (uint64_t j = 0; j < matrix->num_non_zero; j++)
        {
            if ((temp_array[matrix->indices[i * matrix->num_non_zero + j]] || second_zero) && matrix->indices[i * matrix->num_non_zero + j] != 0)
            {
                fprintf(stderr, "Error: Double indices in row or wrong order. Filename: %s\n", filename);
                return -1;
            }
            else if (matrix->indices[i * matrix->num_non_zero + j] >= matrix->num_rows)
            {
                fprintf(stderr, "Error: Index larger then rows (Index out of bound). Filename: %s\n", filename);
                return -1;
            }
            else if (matrix->indices[i * matrix->num_non_zero + j] != 0)
            {
                temp_array[matrix->indices[i * matrix->num_non_zero + j]] = true;
            }
            else
            {
                if (first_zero)
                {
                    if (matrix->values[i * matrix->num_non_zero + j] != 0)
                    {
                        fprintf(stderr, "Error: Double indices (zero) in row or wrong values. Filename: %s\n", filename);
                        return -1;
                    }
                    else
                    {
                        second_zero = true;
                    }
                }
                else
                {
                    temp_array[matrix->indices[i * matrix->num_non_zero + j]] = true;
                    first_zero = true;
                }
            }
        }
        free(temp_array);
    }

    return 0;
}

void handle_error_io(char *line, FILE *file)
{
    free(line);
    fclose(file);
    return -1;
}