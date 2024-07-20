#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

// Function to multiply two matrices in ELLPACK format
void matr_mult_ellpack_V5(const ELLPACKMatrix *matrix_a, const ELLPACKMatrix *matrix_b, ELLPACKMatrix *matrix_result)
{
    // Check if the number of columns in matrix_a is equal to the number of rows in matrix_b
    if (matrix_a->noCols != matrix_b->noRows)
    {
        fprintf(stderr, "Matrix dimensions do not match for multiplication\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the result matrix dimensions and allocate space for values and indices
    matrix_result->noRows = matrix_a->noRows;
    matrix_result->noCols = matrix_b->noCols;
    matrix_result->noNonZero = matrix_b->noCols;

    matrix_result->values = (float *)calloc(matrix_result->noRows * matrix_result->noNonZero, sizeof(float));
    matrix_result->indices = (uint64_t *)calloc(matrix_result->noRows * matrix_result->noNonZero, sizeof(uint64_t));

    if (!matrix_result->values || !matrix_result->indices)
    {
        free(matrix_result->values);
        free(matrix_result->indices);
        fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V2)\n");
        exit(EXIT_FAILURE);
    }

    // Allocate temporary arrays to store intermediate results for the current row
    float *tempValues = (float *)calloc(matrix_result->noCols, sizeof(float));
    uint64_t *tempIndices = (uint64_t *)calloc(matrix_result->noCols, sizeof(uint64_t));
    float *tempBRowValues = (float *)calloc(matrix_b->noCols, sizeof(float));

    if (!tempValues || !tempIndices || !tempBRowValues)
    {
        free(tempValues);
        free(tempIndices);
        free(tempBRowValues);
        free(matrix_result->values);
        free(matrix_result->indices);
        fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V2 temp arrays)\n");
        exit(EXIT_FAILURE);
    }

    // Loop through each row of matrix_a
    for (uint64_t rowA = 0; rowA < matrix_a->noRows; ++rowA)
    {
        // Reset the temporary arrays for the new row of matrix_a
        memset(tempValues, 0, matrix_result->noCols * sizeof(float));
        memset(tempIndices, 0, matrix_result->noCols * sizeof(uint64_t));

        // Loop through each non-zero element in the current row of matrix_a
        for (uint64_t nzIndexA = 0; nzIndexA < matrix_a->noNonZero; ++nzIndexA)
        {
            uint64_t indexA = rowA * matrix_a->noNonZero + nzIndexA;
            __m256 simdValueA = _mm256_set1_ps(matrix_a->values[indexA]);
            uint64_t colA = matrix_a->indices[indexA];
            uint64_t baseIndexB = colA * matrix_b->noNonZero;

            // Loop through each non-zero element in the current row of matrix_b
            for (uint64_t nzIndexB = 0; nzIndexB < matrix_b->noNonZero; ++nzIndexB)
            {
                uint64_t indexB = baseIndexB + nzIndexB;
                uint64_t colB = matrix_b->indices[indexB];
                tempBRowValues[colB] += matrix_b->values[indexB];
                tempIndices[colB] = colB;
            }

            // Perform SIMD multiplication and accumulation
            for (uint64_t colB = 0; colB < (matrix_b->noCols - (matrix_b->noCols % 8)); colB += 8)
            {
                __m256 simdValuesB = _mm256_loadu_ps(&tempBRowValues[colB]);
                __m256 simdTempValues = _mm256_loadu_ps(&tempValues[colB]);
                simdTempValues = _mm256_add_ps(simdTempValues, _mm256_mul_ps(simdValueA, simdValuesB));
                _mm256_storeu_ps(&tempValues[colB], simdTempValues);
            }

            // Process remaining elements
            float a_value = matrix_a->values[indexA];
            for (uint64_t remainB_Index = matrix_b->noCols - (matrix_b->noCols % 8); remainB_Index < matrix_b->noCols; ++remainB_Index)
            {
                tempValues[remainB_Index] += tempBRowValues[remainB_Index] * a_value;
            }

            // Reset the temporary B row array for the next iteration
            memset(tempBRowValues, 0, matrix_b->noCols * sizeof(float));
        }

        // Transfer the non-zero values from the temporary values array to the result matrix
        uint64_t nzCountResult = 0;
        for (uint64_t colResult = 0; colResult < matrix_result->noCols; ++colResult)
        {
            if (tempValues[colResult] != 0.0f)
            {
                uint64_t resultIndex = rowA * matrix_result->noNonZero + nzCountResult;
                matrix_result->values[resultIndex] = tempValues[colResult];
                matrix_result->indices[resultIndex] = tempIndices[colResult];
                nzCountResult++;
            }
        }
    }

    // Free the allocated memory for the temporary arrays
    free(tempValues);
    free(tempIndices);
    free(tempBRowValues);
}
