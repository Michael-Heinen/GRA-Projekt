#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

// Function to multiply two matrices in ELLPACK format
void matr_mult_ellpack_V2(const ELLPACKMatrix *restrict matrixA, const ELLPACKMatrix *restrict matrixB, ELLPACKMatrix *restrict result)
{
    // Check if the number of columns in matrixA is equal to the number of rows in matrixB
    if (matrixA->noCols != matrixB->noRows)
    {
        fprintf(stderr, "Matrix dimensions do not match for multiplication\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the result matrix dimensions and allocate space for values and indices
    result->noRows = matrixA->noRows;
    result->noCols = matrixB->noCols;
    result->noNonZero = matrixB->noCols;

    result->result_values = (float **)calloc(result->noRows, sizeof(float*));
    result->result_indices = (uint64_t **)calloc(result->noRows, sizeof(uint64_t*));

    if (!result->result_values || !result->result_indices)
    {
        free(result->result_values);
        free(result->result_indices);
        fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V2 (V2))\n");
        exit(EXIT_FAILURE);
    }

    // Allocate temporary arrays to store intermediate results for the current row

    float *tempBRowValues = (float *)calloc(matrixB->noCols, sizeof(float));

    uint64_t max_non_zero = 0;

    // Loop through each row of matrixA
    for (uint64_t rowA = 0; rowA < matrixA->noRows; ++rowA)
    {
        result->result_values[rowA] = (float *)calloc(result->noCols, sizeof(float));
        result->result_indices[rowA] = (uint64_t *)calloc(result->noCols, sizeof(uint64_t));

        if (!result->result_values[rowA] || !result->result_indices[rowA])
        {
            for (uint64_t i = 0; i <= rowA; ++i)
            {
                free(result->result_values[i]);
                free(result->result_indices[i]);
            }
            free(result->result_values);
            free(result->result_indices);
            fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V3)\n");
            exit(EXIT_FAILURE);
        }



        // Loop through each non-zero element in the current row of matrixA
        for (uint64_t nzIndexA = 0; nzIndexA < matrixA->noNonZero; ++nzIndexA)

        {
            // Calculate the index of the current non-zero element in matrixA
            uint64_t indexA = rowA * matrixA->noNonZero + nzIndexA;
            // Load the current value from matrixA into a SIMD register and broadcast it
            __m128 simdValueA = _mm_set1_ps(matrixA->values[indexA]);
            // Get the column index from the current non-zero element in matrixA
            uint64_t colA = matrixA->indices[indexA];

            // Calculate the base index of the row in matrixB corresponding to the column index in matrixA
            uint64_t baseIndexB = colA * matrixB->noNonZero;

            // Loop through each non-zero element in the current row of matrixB
            for (uint64_t nzIndexB = 0; nzIndexB < matrixB->noNonZero; ++nzIndexB)
            {
                // Calculate the index of the current non-zero element in matrixB
                uint64_t indexB = baseIndexB + nzIndexB;
                // Get the column index and value from the current non-zero element in matrixB
                uint64_t colB = matrixB->indices[indexB];
                tempBRowValues[colB] += matrixB->values[indexB];
                // Store the column index in the temporary indices array
                result->result_indices[rowA][colB] = colB;
            }

            // Loop through the temporary B row array in chunks of 4 elements (SIMD width)
            for (uint64_t colB = 0; colB < (matrixB->noCols - (matrixB->noCols % 4)); colB += 4)
            {
                // Load 4 values from the temporary B row array into a SIMD register
                __m128 simdValuesB = _mm_load_ps(&tempBRowValues[colB]);
                // Load 4 values from the temporary values array into a SIMD register
                __m128 simdTempValues = _mm_load_ps(&result->result_values[rowA][colB]);
                // Multiply and accumulate the values
                simdTempValues = _mm_add_ps(simdTempValues, _mm_mul_ps(simdValueA, simdValuesB));
                // Store the result back into the temporary values array
                _mm_store_ps(&result->result_values[rowA][colB], simdTempValues);
            }

            float a_value = matrixA->values[indexA];
            baseIndexB = (matrixB->noCols - (matrixB->noCols % 4));
            for (uint64_t remainB_Index = 0; remainB_Index < matrixB->noCols % 4; remainB_Index++)
            {
                float calc = tempBRowValues[baseIndexB + remainB_Index] * a_value;
                result->result_values[rowA][baseIndexB + remainB_Index] += calc;
            }

            // Reset the temporary B row array for the next iteration
            for (uint64_t colB = 0; colB < matrixB->noCols; ++colB)
            {
                tempBRowValues[colB] = 0.0f;
            }
        }

        // Transfer the non-zero values from the temporary values array to the result matrix
        uint64_t cnt_non_zero = 0;
        for (uint64_t colResult = 0; colResult < result->noCols; ++colResult)
        {
            if (result->result_values[rowA][colResult] != 0.0f)
            {
                // Store the value and index in the9 result matrix
                result->result_values[rowA][cnt_non_zero] = result->result_values[rowA][colResult];
                result->result_indices[rowA][cnt_non_zero] = result->result_indices[rowA][colResult];
                cnt_non_zero++;
            }
        }

        if (cnt_non_zero > max_non_zero)
        {
            max_non_zero = cnt_non_zero;
        }
    }

    result->noNonZero = max_non_zero;

    // Free the allocated memory for the temporary arrays
    free(tempBRowValues);
}
