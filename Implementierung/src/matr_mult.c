#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>

void matr_mult_ellpack(const ELLPACKMatrix *matrix_a, const ELLPACKMatrix *matrix_b, ELLPACKMatrix *matrix_result)
{
    if (matrix_a->noCols != matrix_b->noRows)
    {
        fprintf(stderr, "Matrix dimensions do not match for multiplication (matr_mult_ellpack (V0))\n");
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
        fprintf(stderr, "Memory allocation failed (matr_mult_ellpack (V0))\n");
        exit(EXIT_FAILURE);
    }

    for (uint64_t i = 0; i < matrix_a->noRows; ++i)
    {
        for (uint64_t k = 0; k < matrix_a->noNonZero; ++k)
        {
            uint64_t a_index = i * matrix_a->noNonZero + k;
            float a_value = matrix_a->values[a_index];
            if (a_value == 0.0)
            {
                continue;
            }
            uint64_t a_col = matrix_a->indices[a_index];

            for (uint64_t l = 0; l < matrix_b->noNonZero; ++l)
            {
                uint64_t b_index = a_col * matrix_b->noNonZero + l;
                float b_value = matrix_b->values[b_index];
                if (b_value == 0.0f)
                {
                    continue;
                }
                uint64_t b_col = matrix_b->indices[b_index];

                for (uint64_t m = 0; m < matrix_result->noNonZero; m++)
                {
                    if (matrix_result->values[i * matrix_result->noNonZero + m] == 0.0f || matrix_result->indices[i * matrix_result->noNonZero + m] == b_col)
                    {
                        matrix_result->values[i * matrix_result->noNonZero + m] += a_value * b_value;
                        matrix_result->indices[i * matrix_result->noNonZero + m] = b_col;
                        break;
                    }
                }
            }
        }
    }
}