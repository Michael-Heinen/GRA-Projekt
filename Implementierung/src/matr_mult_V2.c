#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

void matr_mult_ellpack_V2(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result)
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
    float *b_temp_values = (float *)calloc(b->noCols, sizeof(float));
    // uint64_t temp_nonZero = 0;

    for (uint64_t curr_a_row = 0; curr_a_row < a->noRows; ++curr_a_row)
    {

        for (uint64_t curr_a_nonZero = 0; curr_a_nonZero < a->noNonZero; ++curr_a_nonZero)
        {
            uint64_t a_index = curr_a_row * a->noNonZero + curr_a_nonZero;
            __m128 a_values = _mm_set1_ps(a->values[a_index]);
            // if (_mm_testz_ps(a_values, a_values))
            // {
            //     continue; // Skip if all values in the SIMD register are zero
            // }
            uint64_t a_col = a->indices[a_index];

            // load b row into temp b array
            uint64_t b_row_base_index = a_col * b->noNonZero;

            // loop over row of b corresponding to the a_col
            for (uint64_t b_in = 0; b_in < b->noNonZero; b_in++)
            {
                uint64_t cur_b_index = b_row_base_index + b_in;
                uint64_t b_col = b->indices[cur_b_index];
                b_temp_values[b_col] = b->values[cur_b_index];
                temp_indices[b_col] = b_col;
            }

            // loop over b_temp_array and calculate into temp_array
            for (uint64_t curr_b_nonZero = 0; curr_b_nonZero < b->noNonZero; curr_b_nonZero += 4)
            {
                uint64_t b_index = b_row_base_index + curr_b_nonZero;
                __m128 b_values = _mm_loadu_ps(&b_temp_values[curr_b_nonZero]);
                // if (_mm_testz_ps(b_values, b_values))
                // {
                //     continue; // Skip if all values in the SIMD register are zero
                // }
                __m128 result_simd = _mm_mul_ps(a_values, b_values);
                __m128 temp_simd = _mm_load_ps(&temp_values[curr_b_nonZero]);
                temp_simd = _mm_add_ps(result_simd, temp_simd);

                // load back into temp array
                _mm_store_ps(&b_temp_values[curr_b_nonZero], temp_simd);
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
}
