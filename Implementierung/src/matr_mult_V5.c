#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

// Function to multiply two matrices in ELLPACK format
void matr_mult_ellpack_V5(const ELLPACKMatrix *matrixA, const ELLPACKMatrix *matrixB, ELLPACKMatrix *resultMatrix)
{
    // Check if the number of columns in matrixA is equal to the number of rows in matrixB
    if (matrixA->noCols != matrixB->noRows)
    {
        fprintf(stderr, "Matrix dimensions do not match for multiplication\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the result matrix dimensions and allocate space for values and indices
    resultMatrix->noRows = matrixA->noRows;
    resultMatrix->noCols = matrixB->noCols;
    resultMatrix->noNonZero = matrixB->noCols;

    resultMatrix->values = (float *)calloc(resultMatrix->noRows * resultMatrix->noNonZero, sizeof(float));
    resultMatrix->indices = (uint64_t *)calloc(resultMatrix->noRows * resultMatrix->noNonZero, sizeof(uint64_t));

    if (!resultMatrix->values || !resultMatrix->indices)
    {
        free(resultMatrix->values);
        free(resultMatrix->indices);
        fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V2)\n");
        exit(EXIT_FAILURE);
    }

    // Allocate temporary arrays to store intermediate results for the current row
    float *tempValues = (float *)calloc(resultMatrix->noCols, sizeof(float));
    uint64_t *tempIndices = (uint64_t *)calloc(resultMatrix->noCols, sizeof(uint64_t));
    float *tempBRowValues = (float *)calloc(matrixB->noCols, sizeof(float));

    if (!tempValues || !tempIndices || !tempBRowValues)
    {
        free(tempValues);
        free(tempIndices);
        free(tempBRowValues);
        free(resultMatrix->values);
        free(resultMatrix->indices);
        fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V2 temp arrays)\n");
        exit(EXIT_FAILURE);
    }

    // Loop through each row of matrixA
    for (uint64_t rowA = 0; rowA < matrixA->noRows; ++rowA)
    {
        // Reset the temporary arrays for the new row of matrixA
        memset(tempValues, 0, resultMatrix->noCols * sizeof(float));
        memset(tempIndices, 0, resultMatrix->noCols * sizeof(uint64_t));

        // Loop through each non-zero element in the current row of matrixA
        for (uint64_t nzIndexA = 0; nzIndexA < matrixA->noNonZero; ++nzIndexA)
        {
            uint64_t indexA = rowA * matrixA->noNonZero + nzIndexA;
            __m256 simdValueA = _mm256_set1_ps(matrixA->values[indexA]);
            uint64_t colA = matrixA->indices[indexA];
            uint64_t baseIndexB = colA * matrixB->noNonZero;

            // Loop through each non-zero element in the current row of matrixB
            for (uint64_t nzIndexB = 0; nzIndexB < matrixB->noNonZero; ++nzIndexB)
            {
                uint64_t indexB = baseIndexB + nzIndexB;
                uint64_t colB = matrixB->indices[indexB];
                tempBRowValues[colB] += matrixB->values[indexB];
                tempIndices[colB] = colB;
            }

            // Perform SIMD multiplication and accumulation
            for (uint64_t colB = 0; colB < (matrixB->noCols - (matrixB->noCols % 8)); colB += 8)
            {
                __m256 simdValuesB = _mm256_loadu_ps(&tempBRowValues[colB]);
                __m256 simdTempValues = _mm256_loadu_ps(&tempValues[colB]);
                simdTempValues = _mm256_add_ps(simdTempValues, _mm256_mul_ps(simdValueA, simdValuesB));
                _mm256_storeu_ps(&tempValues[colB], simdTempValues);
            }

            // Process remaining elements
            float a_value = matrixA->values[indexA];
            for (uint64_t remainB_Index = matrixB->noCols - (matrixB->noCols % 8); remainB_Index < matrixB->noCols; ++remainB_Index)
            {
                tempValues[remainB_Index] += tempBRowValues[remainB_Index] * a_value;
            }

            // Reset the temporary B row array for the next iteration
            memset(tempBRowValues, 0, matrixB->noCols * sizeof(float));
        }

        // Transfer the non-zero values from the temporary values array to the result matrix
        uint64_t nzCountResult = 0;
        for (uint64_t colResult = 0; colResult < resultMatrix->noCols; ++colResult)
        {
            if (tempValues[colResult] != 0.0f)
            {
                uint64_t resultIndex = rowA * resultMatrix->noNonZero + nzCountResult;
                resultMatrix->values[resultIndex] = tempValues[colResult];
                resultMatrix->indices[resultIndex] = tempIndices[colResult];
                nzCountResult++;
            }
        }
    }

    // Free the allocated memory for the temporary arrays
    free(tempValues);
    free(tempIndices);
    free(tempBRowValues);
}
