#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>

void matr_mult_ellpack_V3(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result)
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

    if (a->noNonZero == 0 || b->noNonZero == 0)
    {
        result->noNonZero = 0;
        return;
    }

    // temporary result arrays for current row
    float *temp_values = (float *)calloc(result->noCols, sizeof(float));
    uint64_t *temp_indices = (uint64_t *)calloc(result->noCols, sizeof(uint64_t));
    // uint64_t temp_nonZero = 0;

    for (uint64_t curr_a_row = 0; curr_a_row < a->noRows; ++curr_a_row)
    {

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

                temp_values[b_col] += a_value * b_value;
                temp_indices[b_col] = b_col;
                // temp_nonZero++;
            }
        }

        // copy temp arrays into final array
        uint64_t cnt_zero = 0;
        for (uint64_t i = 0; i < result->noCols; ++i)
        {
            if (temp_values[i] != 0.0f)
            {
                uint64_t res_index = curr_a_row * result->noNonZero + i - cnt_zero;
                result->values[res_index] = temp_values[i];
                result->indices[res_index] = temp_indices[i];
                continue;
            }
            cnt_zero++;
        }

        // reset temp arrays
        for (uint64_t i = 0; i < result->noCols; ++i)
        {
            temp_values[i] = 0.0f;
            temp_indices[i] = 0;
        }
        // temp_nonZero = 0;
    }

    free(temp_values);
    free(temp_indices);
}
