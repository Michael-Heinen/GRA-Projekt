#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>

void matr_mult_ellpack_V6(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result)
{
    if (a->noCols != b->noRows)
    {
        fprintf(stderr, "Matrix dimensions do not match for multiplication\n");
        exit(EXIT_FAILURE);
    }

    result->noRows = a->noRows;
    result->noCols = b->noCols;
    result->noNonZero = b->noCols;

    result->result_values = (float **)calloc(result->noRows, sizeof(float*));
    result->result_indices = (uint64_t **)calloc(result->noRows, sizeof(uint64_t*));

    if (!result->result_values || !result->result_indices)
    {
        free(result->result_values);
        free(result->result_indices);
        fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V3 (V3))\n");
        exit(EXIT_FAILURE);
    }

    if (a->noNonZero == 0 || b->noNonZero == 0)
    {
        result->noNonZero = 0;
        return;
    }

    uint64_t max_non_zero = 0;

    #pragma omp parallel for reduction(max:max_non_zero)
    for (uint64_t curr_a_row = 0; curr_a_row < a->noRows; ++curr_a_row)
    {
        result->result_values[curr_a_row] = (float *)calloc(result->noCols, sizeof(float));
        result->result_indices[curr_a_row] = (uint64_t *)calloc(result->noCols, sizeof(uint64_t));

        for (uint64_t curr_a_nonZero = 0; curr_a_nonZero < a->noNonZero; ++curr_a_nonZero)
        {
            uint64_t a_index = curr_a_row * a->noNonZero + curr_a_nonZero;
            float a_value = a->values[a_index];
            if (a_value == 0.0)
            {
                continue;
            }
            uint64_t a_col = a->indices[a_index];

            for (uint64_t curr_b_nonZero = 0; curr_b_nonZero < b->noNonZero; ++curr_b_nonZero)
            {
                uint64_t b_index = a_col * b->noNonZero + curr_b_nonZero;
                float b_value = b->values[b_index];
                if (b_value == 0.0f)
                {
                    continue;
                }
                uint64_t b_col = b->indices[b_index];

                
                result->result_values[curr_a_row][b_col] += a_value * b_value;
                result->result_indices[curr_a_row][b_col] = b_col;
            }
        }

        uint64_t cnt_non_zero = 0;
        for (uint64_t i = 0; i < result->noCols; ++i)
        {
            if (result->result_values[curr_a_row][i] != 0.0f)
            {
                result->result_values[curr_a_row][cnt_non_zero] = result->result_values[curr_a_row][i];
                result->result_indices[curr_a_row][cnt_non_zero] = result->result_indices[curr_a_row][i];
                cnt_non_zero++;
            }
        }

        if (cnt_non_zero > max_non_zero)
        {
            max_non_zero = cnt_non_zero;
        }
    }

    result->noNonZero = max_non_zero;
}
