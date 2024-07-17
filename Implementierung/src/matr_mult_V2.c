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

    for (uint64_t i = 0; i < a->noRows; ++i)
    {
        for (uint64_t k = 0; k < a->noNonZero; k += 4)
        {
            uint64_t a_index = i * a->noNonZero + k;
            __m128 a_values = _mm_loadu_ps(&a->values[a_index]);

            if (_mm_testz_ps(a_values, a_values))
            {
                continue;
            }

            __m128 a_cols = _mm_loadu_si128((__m128i *)&a->indices[a_index]);

            for (uint64_t l = 0; l < b->noNonZero; l += 4)
            {
                uint64_t b_index = (uint64_t)_mm_extract_epi32(a_cols, l) * b->noNonZero + l;
                __m128 b_values = _mm_loadu_ps(&b->values[b_index]);
                __m128 b_cols = _mm_loadu_si128((__m128i *)&b->indices[b_index]);

                __m128 result_values = _mm_mul_ps(a_values, b_values);

                for (int m = 0; m < 4; ++m)
                {
                    int idx = _mm_extract_epi32(b_cols, m);
                    if (result->indices[i * result->noNonZero + m] == 0 && result->values[i * result->noNonZero + m] == 0.0f)
                        uint64_t b_index = a_col * b->noNonZero + l;
                    float b_value = b->values[b_index];
                    if (b_value == 0.0f)
                    {
                        continue;
                    }
                    uint64_t b_col = b->indices[b_index];

                    for (uint64_t m = 0; i < result->noNonZero; m++)
                    {
                        if (result->indices[i * result->noNonZero + m] == 0 && result->values[i * result->noNonZero + m] == 0.0f)
                        {
                            result->values[i * result->noNonZero + m] = _mm_cvtss_f32(result_values);
                            result->indices[i * result->noNonZero + m] = idx;
                        }
                    }
                }
            }
        }
    }

    // compute noNonZero in result matrix
    int compute_noNonZero_V2(ELLPACKMatrix * matrix)
    {
        uint64_t maxNoNonZero = 0;
        for (uint64_t a_row = 0; a_row < matrix->noCols; a_row++)
        {
            uint64_t tmpNoNonZero = 0;
            for (uint64_t j = 0; j < matrix->noNonZero; j++)
            {
                if (matrix->values[a_row * matrix->noNonZero + j] == 0.0f)
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
