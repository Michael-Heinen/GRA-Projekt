#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <emmintrin.h> // SSE2
#include <smmintrin.h> // SSE4.2

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

    // Temporary result arrays for current row
    float *temp_values = (float *)calloc(result->noCols, sizeof(float));
    uint64_t *temp_indices = (uint64_t *)calloc(result->noCols, sizeof(uint64_t));

    for (uint64_t curr_a_row = 0; curr_a_row < a->noRows; ++curr_a_row)
    {
        for (uint64_t curr_a_nonZero = 0; curr_a_nonZero < a->noNonZero; curr_a_nonZero += 4)
        {
            uint64_t a_index = curr_a_row * a->noNonZero + curr_a_nonZero;

            __m128 a_values = _mm_loadu_ps(&a->values[a_index]);
            if (_mm_testz_ps(a_values, a_values))
            {
                continue; // Skip if all values in the SIMD register are zero
            }

            __m128i a_cols = _mm_loadu_si128((__m128i *)&a->indices[a_index]);

            for (uint64_t curr_b_col = 0; curr_b_col < b->noCols; ++curr_b_col)
            {
                for (uint64_t curr_b_row = 0; curr_b_row < b->noRows; curr_b_row += 4)
                {
                    /* code */
                }

                uint64_t b_base_index = _mm_extract_epi32(a_cols, 0) * b->noNonZero + curr_b_col;

                __m128 b_values = _mm_loadu_ps(&b->values[b_base_index]);
                if (_mm_testz_ps(b_values, b_values))
                {
                    continue; // Skip if all values in the SIMD register are zero
                }

                __m128i b_cols = _mm_loadu_si128((__m128i *)&b->indices[b_base_index]);
                __m128 result_values = _mm_mul_ps(a_values, b_values);

                for (int i = 0; i < 4; ++i)
                {
                    int b_col = _mm_extract_epi32(b_cols, i);
                    if (b_col >= 0 && b_col < b->noCols)
                    {
                        temp_values[b_col] += _mm_cvtss_f32(_mm_shuffle_ps(result_values, result_values, _MM_SHUFFLE(i, i, i, i)));
                        temp_indices[b_col] = b_col;
                    }
                }
            }
        }

        // Copy temp arrays into the final array
        for (uint64_t i = 0; i < result->noCols; ++i)
        {
            if (temp_values[i] != 0.0f)
            {
                uint64_t res_index = curr_a_row * result->noNonZero + i;
                result->values[res_index] = temp_values[i];
                result->indices[res_index] = temp_indices[i];
            }
        }

        // Reset temp arrays
        for (uint64_t i = 0; i < result->noCols; ++i)
        {
            temp_values[i] = 0.0f;
            temp_indices[i] = 0;
        }
    }

    free(temp_values);
    free(temp_indices);
}