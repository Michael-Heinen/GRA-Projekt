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
    result->noNonZero = a->noNonZero * b->noNonZero;

    result->values = (float *)calloc(result->noRows * result->noNonZero, sizeof(float));
    result->indices = (uint64_t *)calloc(result->noRows * result->noNonZero, sizeof(uint64_t));

    for (uint64_t i = 0; i < a->noRows; ++i)
    {
        for (uint64_t k = 0; k < a->noNonZero; k += 4)
        {
            uint64_t a_index = i * a->noNonZero + k;
            __m128 a_values = _mm_loadu_ps(&a->values[a_index]);

            // Check if all elements in a_values are zero, if so, skip to the next iteration
            if (_mm_testz_ps(a_values, a_values))
            {
                continue;
            }

            __m128i a_cols = _mm_loadu_si128((__m128i *)&a->indices[a_index]);

            for (uint64_t l = 0; l < b->noNonZero; l += 4)
            {
                uint64_t b_index = _mm_extract_epi32(a_cols, 0) * b->noNonZero + l;
                __m128 b_values = _mm_loadu_ps(&b->values[b_index]);
                __m128i b_cols = _mm_loadu_si128((__m128i *)&b->indices[b_index]);

                __m128 result_values = _mm_mul_ps(a_values, b_values);

                for (int m = 0; m < 4; ++m)
                {
                    int col_idx = _mm_extract_epi32(b_cols, m);
                    if (col_idx < 0 || col_idx >= b->noCols)
                        continue;
                    uint64_t result_idx = i * result->noNonZero + col_idx;

                    result->values[result_idx] += _mm_cvtss_f32(_mm_shuffle_ps(result_values, result_values, _MM_SHUFFLE(m, m, m, m)));
                    result->indices[result_idx] = col_idx;
                }
            }
        }
    }
}

int compute_noNonZero_V2(ELLPACKMatrix *matrix)
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
