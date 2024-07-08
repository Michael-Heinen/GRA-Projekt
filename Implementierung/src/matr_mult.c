#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>

void matr_mult_ellpack(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result)
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
    result->indices = (int64_t *)calloc(result->noRows * result->noNonZero, sizeof(int64_t));

    int64_t arr_len = result->noRows * result->noNonZero;
    for (int64_t i = 0; i < arr_len; i++)
    {
        result->indices[i] = -1;
    }

    for (int64_t i = 0; i < a->noRows; ++i)
    {
        for (int64_t k = 0; k < a->noNonZero; ++k)
        {
            int64_t a_index = i * a->noNonZero + k;
            float a_value = a->values[a_index];
            if (a_value == 0.0)
            {
                continue;
            }
            int64_t a_col = a->indices[a_index];

            for (int64_t l = 0; l < b->noNonZero; ++l)
            {
                int64_t b_index = a_col * b->noNonZero + l;
                float b_value = b->values[b_index];
                int64_t b_col = b->indices[b_index];
                if (b_col <= -1)
                {
                    continue;
                }

                for (int64_t m = 0; i < result->noNonZero; m++)
                {
                    if (result->indices[i * result->noNonZero + m] < 0 || result->indices[i * result->noNonZero + m] == b_col)
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

// compute noNonZero in result matrix
int compute_noNonZero(ELLPACKMatrix *matrix)
{
    int64_t maxNoNonZero = 0;
    for (int64_t i = 0; i < matrix->noCols; i++)
    {
        int64_t tmpNoNonZero = 0;
        for (int64_t j = 0; j < matrix->noNonZero; j++)
        {
            if (matrix->indices[i * matrix->noNonZero + j] < 0)
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