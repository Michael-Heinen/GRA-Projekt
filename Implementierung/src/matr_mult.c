#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>

void matr_mult_ellpack(const ELLPACKMatrix *restrict matrix_a, const ELLPACKMatrix *restrict matrix_b, ELLPACKMatrix *restrict matrix_result)
{
    if (matrix_a->num_cols != matrix_b->num_rows)
    {
        fprintf(stderr, "Matrix dimensions do not match for multiplication\n");
        exit(EXIT_FAILURE);
    }

    matrix_result->num_rows = matrix_a->num_rows;
    matrix_result->num_cols = matrix_b->num_cols;

    matrix_result->result_values = (float **)calloc(matrix_result->num_rows, sizeof(float *));
    matrix_result->result_indices = (uint64_t **)calloc(matrix_result->num_rows, sizeof(uint64_t *));

    if (!matrix_result->result_values || !matrix_result->result_indices)
    {
        free(matrix_result->result_values);
        free(matrix_result->result_indices);
        fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V3 (V3))\n");
        exit(EXIT_FAILURE);
    }

    if (matrix_a->num_non_zero == 0 || matrix_b->num_non_zero == 0)
    {
        matrix_result->num_non_zero = 0;
        return;
    }

    uint64_t max_non_zero = 0;

    for (uint64_t curr_row_a = 0; curr_row_a < matrix_a->num_rows; ++curr_row_a)
    {

        matrix_result->result_values[curr_row_a] = (float *)calloc(matrix_result->num_cols, sizeof(float));
        matrix_result->result_indices[curr_row_a] = (uint64_t *)calloc(matrix_result->num_cols, sizeof(uint64_t));

        if (!matrix_result->result_values[curr_row_a] || !matrix_result->result_indices[curr_row_a])
        {
            for (uint64_t i = 0; i <= curr_row_a; ++i)
            {
                free(matrix_result->result_values[i]);
                free(matrix_result->result_indices[i]);
            }
            free(matrix_result->result_values);
            free(matrix_result->result_indices);
            fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V3)\n");
            exit(EXIT_FAILURE);
        }

        for (uint64_t curr_non_zero_a = 0; curr_non_zero_a < matrix_a->num_non_zero; ++curr_non_zero_a)
        {
            uint64_t index_a = curr_row_a * matrix_a->num_non_zero + curr_non_zero_a;
            float value_a = matrix_a->values[index_a];
            if (value_a == 0.0)
            {
                continue;
            }
            uint64_t col_a = matrix_a->indices[index_a];

            for (uint64_t curr_b_nonZero = 0; curr_b_nonZero < matrix_b->num_non_zero; ++curr_b_nonZero)
            {
                uint64_t index_b = col_a * matrix_b->num_non_zero + curr_b_nonZero;
                float value_b = matrix_b->values[index_b];
                if (value_b == 0.0f)
                {
                    continue;
                }
                uint64_t col_b = matrix_b->indices[index_b];

                matrix_result->result_values[curr_row_a][col_b] += value_a * value_b;
                matrix_result->result_indices[curr_row_a][col_b] = col_b;
            }
        }

        uint64_t cnt_non_zero = 0;
        for (uint64_t i = 0; i < matrix_result->num_cols; ++i)
        {
            if (matrix_result->result_values[curr_row_a][i] != 0.0f)
            {
                matrix_result->result_values[curr_row_a][cnt_non_zero] = matrix_result->result_values[curr_row_a][i];
                matrix_result->result_indices[curr_row_a][cnt_non_zero] = matrix_result->result_indices[curr_row_a][i];
                cnt_non_zero++;
            }
        }

        if (cnt_non_zero > max_non_zero)
        {
            max_non_zero = cnt_non_zero;
        }
    }

    matrix_result->num_non_zero = max_non_zero;
}
