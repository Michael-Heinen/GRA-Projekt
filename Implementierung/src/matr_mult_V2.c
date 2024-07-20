#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

// Function to multiply two matrices in ELLPACK format
void matr_mult_ellpack_V2(const ELLPACKMatrix *restrict matrix_a, const ELLPACKMatrix *restrict matrix_b, ELLPACKMatrix *restrict matrix_result)
{
    // Check if the number of columns in matrix_a is equal to the number of rows in matrix_b
    if (matrix_a->noCols != matrix_b->noRows)
    {
        fprintf(stderr, "Matrix dimensions do not match for multiplication\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the matrix_result matrix dimensions and allocate space for values and indices
    matrix_result->noRows = matrix_a->noRows;
    matrix_result->noCols = matrix_b->noCols;
    matrix_result->noNonZero = matrix_b->noCols;

    matrix_result->result_values = (float **)calloc(matrix_result->noRows, sizeof(float*));
    matrix_result->result_indices = (uint64_t **)calloc(matrix_result->noRows, sizeof(uint64_t*));

    if (!matrix_result->result_values || !matrix_result->result_indices)
    {
        free(matrix_result->result_values);
        free(matrix_result->result_indices);
        fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V2 (V2))\n");
        exit(EXIT_FAILURE);
    }

    // Allocate temporary arrays to store intermediate results for the current row

    float *tempBRowValues = (float *)calloc(matrix_b->noCols, sizeof(float));

    uint64_t max_non_zero = 0;

    // Loop through each row of matrix_a
    for (uint64_t rowA = 0; rowA < matrix_a->noRows; ++rowA)
    {
        matrix_result->result_values[rowA] = (float *)calloc(matrix_result->noCols, sizeof(float));
        matrix_result->result_indices[rowA] = (uint64_t *)calloc(matrix_result->noCols, sizeof(uint64_t));

        if (!matrix_result->result_values[rowA] || !matrix_result->result_indices[rowA])
        {
            for (uint64_t i = 0; i <= rowA; ++i)
            {
                free(matrix_result->result_values[i]);
                free(matrix_result->result_indices[i]);
            }
            free(matrix_result->result_values);
            free(matrix_result->result_indices);
            fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V3)\n");
            exit(EXIT_FAILURE);
        }



        // Loop through each non-zero element in the current row of matrix_a
        for (uint64_t nzIndexA = 0; nzIndexA < matrix_a->noNonZero; ++nzIndexA)

        {
            // Calculate the index of the current non-zero element in matrix_a
            uint64_t indexA = rowA * matrix_a->noNonZero + nzIndexA;
            // Load the current value from matrix_a into a SIMD register and broadcast it
            __m128 simdValueA = _mm_set1_ps(matrix_a->values[indexA]);
            // Get the column index from the current non-zero element in matrix_a
            uint64_t colA = matrix_a->indices[indexA];

            // Calculate the base index of the row in matrix_b corresponding to the column index in matrix_a
            uint64_t baseIndexB = colA * matrix_b->noNonZero;

            // Loop through each non-zero element in the current row of matrix_b
            for (uint64_t nzIndexB = 0; nzIndexB < matrix_b->noNonZero; ++nzIndexB)
            {
                // Calculate the index of the current non-zero element in matrix_b
                uint64_t indexB = baseIndexB + nzIndexB;
                // Get the column index and value from the current non-zero element in matrix_b
                uint64_t colB = matrix_b->indices[indexB];
                tempBRowValues[colB] += matrix_b->values[indexB];
                // Store the column index in the temporary indices array
                matrix_result->result_indices[rowA][colB] = colB;
            }

            // Loop through the temporary B row array in chunks of 4 elements (SIMD width)
            for (uint64_t colB = 0; colB < (matrix_b->noCols - (matrix_b->noCols % 4)); colB += 4)
            {
                // Load 4 values from the temporary B row array into a SIMD register
                __m128 simdValuesB = _mm_load_ps(&tempBRowValues[colB]);
                // Load 4 values from the temporary values array into a SIMD register
                __m128 simdTempValues = _mm_load_ps(&matrix_result->result_values[rowA][colB]);
                // Multiply and accumulate the values
                simdTempValues = _mm_add_ps(simdTempValues, _mm_mul_ps(simdValueA, simdValuesB));
                // Store the matrix_result back into the temporary values array
                _mm_store_ps(&matrix_result->result_values[rowA][colB], simdTempValues);
            }

            float a_value = matrix_a->values[indexA];
            baseIndexB = (matrix_b->noCols - (matrix_b->noCols % 4));
            for (uint64_t remainB_Index = 0; remainB_Index < matrix_b->noCols % 4; remainB_Index++)
            {
                float calc = tempBRowValues[baseIndexB + remainB_Index] * a_value;
                matrix_result->result_values[rowA][baseIndexB + remainB_Index] += calc;
            }

            // Reset the temporary B row array for the next iteration
            for (uint64_t colB = 0; colB < matrix_b->noCols; ++colB)
            {
                tempBRowValues[colB] = 0.0f;
            }
        }

        // Transfer the non-zero values from the temporary values array to the matrix_result matrix
        uint64_t cnt_non_zero = 0;
        for (uint64_t colResult = 0; colResult < matrix_result->noCols; ++colResult)
        {
            if (matrix_result->result_values[rowA][colResult] != 0.0f)
            {
                // Store the value and index in the9 matrix_result matrix
                matrix_result->result_values[rowA][cnt_non_zero] = matrix_result->result_values[rowA][colResult];
                matrix_result->result_indices[rowA][cnt_non_zero] = matrix_result->result_indices[rowA][colResult];
                cnt_non_zero++;
            }
        }

        if (cnt_non_zero > max_non_zero)
        {
            max_non_zero = cnt_non_zero;
        }
    }

    matrix_result->noNonZero = max_non_zero;

    // Free the allocated memory for the temporary arrays
    free(tempBRowValues);
}
