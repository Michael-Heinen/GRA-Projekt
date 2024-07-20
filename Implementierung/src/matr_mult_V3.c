#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>

void matr_mult_ellpack_V3(const ELLPACKMatrix *restrict matrix_a, const ELLPACKMatrix *restrict matrix_b, ELLPACKMatrix *restrict matrix_result)
{
    if (matrix_a->noCols != matrix_b->noRows)
    {
        fprintf(stderr, "Matrix dimensions do not match for multiplication\n");
        exit(EXIT_FAILURE);
    }

    matrix_result->noRows = matrix_a->noRows;
    matrix_result->noCols = matrix_b->noCols;

    matrix_result->result_values = (float **)calloc(matrix_result->noRows, sizeof(float*));
    matrix_result->result_indices = (uint64_t **)calloc(matrix_result->noRows, sizeof(uint64_t*));

    if (!matrix_result->result_values || !matrix_result->result_indices)
    {
        free(matrix_result->result_values);
        free(matrix_result->result_indices);
        fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V3 (V3))\n");
        exit(EXIT_FAILURE);
    }

    if (matrix_a->noNonZero == 0 || matrix_b->noNonZero == 0)
    {
        matrix_result->noNonZero = 0;
        return;
    }

    uint64_t max_non_zero = 0;

    for (uint64_t curr_a_row = 0; curr_a_row < matrix_a->noRows; ++curr_a_row)
    {

        matrix_result->result_values[curr_a_row] = (float *)calloc(matrix_result->noCols, sizeof(float));
        matrix_result->result_indices[curr_a_row] = (uint64_t *)calloc(matrix_result->noCols, sizeof(uint64_t));

        if (!matrix_result->result_values[curr_a_row] || !matrix_result->result_indices[curr_a_row])
        {
            for (uint64_t i = 0; i <= curr_a_row; ++i)
            {
                free(matrix_result->result_values[i]);
                free(matrix_result->result_indices[i]);
            }
            free(matrix_result->result_values);
            free(matrix_result->result_indices);
            fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V3)\n");
            exit(EXIT_FAILURE);
        }

        for (uint64_t curr_a_nonZero = 0; curr_a_nonZero < matrix_a->noNonZero; ++curr_a_nonZero)
        {
            uint64_t a_index = curr_a_row * matrix_a->noNonZero + curr_a_nonZero;
            float a_value = matrix_a->values[a_index];
            if (a_value == 0.0)
            {
                continue;
            }
            uint64_t a_col = matrix_a->indices[a_index];

            for (uint64_t curr_b_nonZero = 0; curr_b_nonZero < matrix_b->noNonZero; ++curr_b_nonZero)
            {
                uint64_t b_index = a_col * matrix_b->noNonZero + curr_b_nonZero;
                float b_value = matrix_b->values[b_index];
                if (b_value == 0.0f)
                {
                    continue;
                }
                uint64_t b_col = matrix_b->indices[b_index];

                matrix_result->result_values[curr_a_row][b_col] += a_value * b_value;
                matrix_result->result_indices[curr_a_row][b_col] = b_col;

            }
        }


        uint64_t cnt_non_zero = 0;
        for (uint64_t i = 0; i < matrix_result->noCols; ++i)
        {
            if (matrix_result->result_values[curr_a_row][i] != 0.0f)
            {
                matrix_result->result_values[curr_a_row][cnt_non_zero] = matrix_result->result_values[curr_a_row][i];
                matrix_result->result_indices[curr_a_row][cnt_non_zero] = matrix_result->result_indices[curr_a_row][i];
                cnt_non_zero++;
            }

        }

        if (cnt_non_zero > max_non_zero)
        {
            max_non_zero = cnt_non_zero;
        }


    }

    matrix_result->noNonZero = max_non_zero;
}
