#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>



void matr_mult_ellpack_V4(const ELLPACKMatrix *restrict a, const ELLPACKMatrix *restrict b, ELLPACKMatrix *restrict result)
{
    if (a->noCols != b->noRows)
    {
        fprintf(stderr, "Matrix dimensions do not match for multiplication\n");
        exit(EXIT_FAILURE);
    }

    result->noRows = a->noRows;
    result->noCols = b->noCols;
    result->noNonZero = b->noCols;

    result->values = (float *)calloc(result->noRows * result->noNonZero, sizeof(float));
    result->indices = (uint64_t *)calloc(result->noRows * result->noNonZero, sizeof(uint64_t));

    if (!result->values || !result->indices)
    {
        free(result->values);
        free(result->indices);
        fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V4 (V4))\n");
        exit(EXIT_FAILURE);
    }

    if (a->noNonZero == 0 || b->noNonZero == 0)
    {
        result->noNonZero = 0;
        return;
    }

    // temporary result arrays for current row
    float *temp_values = (float *)calloc(result->noCols, sizeof(float));
    uint64_t *temp_indices = (uint64_t *)calloc(result->noCols, sizeof(uint64_t));


    for (uint64_t curr_a_row = 0; curr_a_row < a->noRows; ++curr_a_row)
    {

        for (uint64_t curr_a_nonZero = 0; curr_a_nonZero < a->noNonZero; ++curr_a_nonZero)
        {
            uint64_t a_index = curr_a_row * a->noNonZero + curr_a_nonZero;
            float a_value = a->values[a_index];
            if (a_value == 0.0f)
            {
                continue;
            }
            uint64_t a_col = a->indices[a_index];

            __builtin_prefetch(&b->values[a_col * b->noNonZero], 0, 1);
            __builtin_prefetch(&b->indices[a_col * b->noNonZero], 0, 1);

            for (uint64_t curr_b_nonZero = 0; curr_b_nonZero < b->noNonZero; ++curr_b_nonZero)
            {
                uint64_t b_index = a_col * b->noNonZero + curr_b_nonZero;
                float b_value = b->values[b_index];
                if (b_value == 0.0f)
                {
                    continue;
                }
                uint64_t b_col = b->indices[b_index];

                temp_values[b_col] += a_value * b_value;
                temp_indices[b_col] = b_col;
            }
            
            if (curr_a_nonZero + 1 < a->noNonZero)
            {
                uint64_t next_a_index = curr_a_row * a->noNonZero + curr_a_nonZero + 1;
                __builtin_prefetch(&a->values[next_a_index], 0, 1);
                __builtin_prefetch(&a->indices[next_a_index], 0, 1);
            }

        }

        uint64_t res_index_base = curr_a_row * result->noNonZero;

        // copy temp arrays into final array
        for (uint64_t i = 0; i < result->noCols; ++i)
        {
            if (temp_values[i] != 0.0f)
            {

                __builtin_prefetch(&result->values[res_index_base + i], 1, 1);
                __builtin_prefetch(&result->indices[res_index_base + i], 1, 1);

                result->values[res_index_base + i] = temp_values[i];
                result->indices[res_index_base + i] = temp_indices[i];
                temp_values[i] = 0.0f;
                temp_indices[i] = 0;
            }
        }

    }

    free(temp_values);
    free(temp_indices);
}