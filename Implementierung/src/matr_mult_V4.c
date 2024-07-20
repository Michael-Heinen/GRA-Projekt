#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>

void matr_mult_ellpack_V4(const ELLPACKMatrix *restrict matrix_a, const ELLPACKMatrix *restrict matrix_b, ELLPACKMatrix *restrict matrix_result)
{
    if (matrix_a->noCols != matrix_b->noRows)
    {
        fprintf(stderr, "Matrix dimensions do not match for multiplication\n");
        exit(EXIT_FAILURE);
    }

    matrix_result->noRows = matrix_a->noRows;
    matrix_result->noCols = matrix_b->noCols;
    matrix_result->noNonZero = matrix_b->noCols;

    matrix_result->values = (float *)calloc(matrix_result->noRows * matrix_result->noNonZero, sizeof(float));
    matrix_result->indices = (uint64_t *)calloc(matrix_result->noRows * matrix_result->noNonZero, sizeof(uint64_t));

    if (!matrix_result->values || !matrix_result->indices)
    {
        free(matrix_result->values);
        free(matrix_result->indices);
        fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V4 (V4))\n");
        exit(EXIT_FAILURE);
    }

    if (matrix_a->noNonZero == 0 || matrix_b->noNonZero == 0)
    {
        matrix_result->noNonZero = 0;
        return;
    }

    // temporary matrix_result arrays for current row
    float *temp_values = (float *)calloc(matrix_result->noCols, sizeof(float));
    uint64_t *temp_indices = (uint64_t *)calloc(matrix_result->noCols, sizeof(uint64_t));

    for (uint64_t curr_a_row = 0; curr_a_row < matrix_a->noRows; ++curr_a_row)
    {

        for (uint64_t curr_a_nonZero = 0; curr_a_nonZero < matrix_a->noNonZero; ++curr_a_nonZero)
        {
            uint64_t a_index = curr_a_row * matrix_a->noNonZero + curr_a_nonZero;
            float a_value = matrix_a->values[a_index];
            if (a_value == 0.0f)
            {
                continue;
            }
            uint64_t a_col = matrix_a->indices[a_index];

            __builtin_prefetch(&matrix_b->values[a_col * matrix_b->noNonZero], 0, 1);
            __builtin_prefetch(&matrix_b->indices[a_col * matrix_b->noNonZero], 0, 1);

            for (uint64_t curr_b_nonZero = 0; curr_b_nonZero < matrix_b->noNonZero; ++curr_b_nonZero)
            {
                uint64_t b_index = a_col * matrix_b->noNonZero + curr_b_nonZero;
                float b_value = matrix_b->values[b_index];
                if (b_value == 0.0f)
                {
                    continue;
                }
                uint64_t b_col = matrix_b->indices[b_index];

                temp_values[b_col] += a_value * b_value;
                temp_indices[b_col] = b_col;
            }

            if (curr_a_nonZero + 1 < matrix_a->noNonZero)
            {
                uint64_t next_a_index = curr_a_row * matrix_a->noNonZero + curr_a_nonZero + 1;
                __builtin_prefetch(&matrix_a->values[next_a_index], 0, 1);
                __builtin_prefetch(&matrix_a->indices[next_a_index], 0, 1);
            }
        }

        // copy temp arrays into final array
        uint64_t cnt_zero = 0;
        uint64_t res_index_base = curr_a_row * matrix_result->noNonZero;
        for (uint64_t i = 0; i < matrix_result->noCols; ++i)
        {
            if (temp_values[i] != 0.0f)
            {

                __builtin_prefetch(&matrix_result->values[res_index_base + i], 1, 1);
                __builtin_prefetch(&matrix_result->indices[res_index_base + i], 1, 1);

                matrix_result->values[res_index_base + i - cnt_zero] = temp_values[i];
                matrix_result->indices[res_index_base + i - cnt_zero] = temp_indices[i];
                temp_values[i] = 0.0f;
                temp_indices[i] = 0;
                continue;
            }
            cnt_zero++;
        }
    }

    free(temp_values);
    free(temp_indices);
}
