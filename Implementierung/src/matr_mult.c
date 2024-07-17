#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>

void matr_mult_ellpack(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result)
{
    if (a->noCols != b->noRows)
    {
        fprintf(stderr, "Matrix dimensions do not match for multiplication (matr_mult_ellpack (V0))\n");
        exit(EXIT_FAILURE);
    }

    result->noRows = a->noRows;
    result->noCols = b->noCols;
    result->noNonZero = b->noCols;

    result->values = (float *)calloc(result->noRows * result->noNonZero, sizeof(float));
    result->indices = (uint64_t *)calloc(result->noRows * result->noNonZero, sizeof(uint64_t));

    if (!result->values || !result->indices)
    {
        fprintf(stderr, "Memory allocation failed (matr_mult (V0))\n");
        exit(EXIT_FAILURE);
    }

    for (uint64_t i = 0; i < a->noRows; ++i)
    {
        for (uint64_t k = 0; k < a->noNonZero; ++k)
        {
            uint64_t a_index = i * a->noNonZero + k;
            float a_value = a->values[a_index];
            if (a_value == 0.0)
            {
                continue;
            }
            uint64_t a_col = a->indices[a_index];

            for (uint64_t l = 0; l < b->noNonZero; ++l)
            {
                uint64_t b_index = a_col * b->noNonZero + l;
                float b_value = b->values[b_index];
                if (b_value == 0.0f)
                {
                    continue;
                }
                uint64_t b_col = b->indices[b_index];

                for (uint64_t m = 0; m < result->noNonZero; m++)
                {
                    if (result->values[i * result->noNonZero + m] == 0.0f || result->indices[i * result->noNonZero + m] == b_col)
                    {
                        result->values[i * result->noNonZero + m] += a_value * b_value;
                        result->indices[i * result->noNonZero + m] = b_col;
                        break;
                    }
                }
            }
        }
    }
}