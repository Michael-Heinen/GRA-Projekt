#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

// Function to multiply two matrices in ELLPACK format
void matr_mult_ellpack_V5(const ELLPACKMatrix *matrix_a, const ELLPACKMatrix *matrix_b, ELLPACKMatrix *matrix_result)
{
    // Check if the number of columns in matrix_a is equal to the number of rows in matrix_b
    if (matrix_a->num_cols != matrix_b->num_rows)
    {
        fprintf(stderr, "Matrix dimensions do not match for multiplication\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the result matrix dimensions and allocate space for values and indices
    matrix_result->num_rows = matrix_a->num_rows;
    matrix_result->num_cols = matrix_b->num_cols;
    matrix_result->num_non_zero = matrix_b->num_cols;

    matrix_result->values = (float *)calloc(matrix_result->num_rows * matrix_result->num_non_zero, sizeof(float));
    matrix_result->indices = (uint64_t *)calloc(matrix_result->num_rows * matrix_result->num_non_zero, sizeof(uint64_t));

    if (!matrix_result->values || !matrix_result->indices)
    {
        free(matrix_result->values);
        free(matrix_result->indices);
        fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V2)\n");
        exit(EXIT_FAILURE);
    }

    // Allocate temporary arrays to store intermediate results for the current row
    float *tempValues = (float *)calloc(matrix_result->num_cols, sizeof(float));
    uint64_t *tempIndices = (uint64_t *)calloc(matrix_result->num_cols, sizeof(uint64_t));
    float *tempBRowValues = (float *)calloc(matrix_b->num_cols, sizeof(float));

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
    for (uint64_t rowA = 0; rowA < matrix_a->num_rows; ++rowA)
    {
        // Reset the temporary arrays for the new row of matrix_a
        memset(tempValues, 0, matrix_result->num_cols * sizeof(float));
        memset(tempIndices, 0, matrix_result->num_cols * sizeof(uint64_t));

        // Loop through each non-zero element in the current row of matrix_a
        for (uint64_t nzIndexA = 0; nzIndexA < matrix_a->num_non_zero; ++nzIndexA)
        {
            uint64_t indexA = rowA * matrix_a->num_non_zero + nzIndexA;
            __m256 simdValueA = _mm256_set1_ps(matrix_a->values[indexA]);
            uint64_t colA = matrix_a->indices[indexA];
            uint64_t base_index_b = colA * matrix_b->num_non_zero;

            // Loop through each non-zero element in the current row of matrix_b
            for (uint64_t num_non_zero_b = 0; num_non_zero_b < matrix_b->num_non_zero; ++num_non_zero_b)
            {
                uint64_t index_b = base_index_b + num_non_zero_b;
                uint64_t col_b = matrix_b->indices[index_b];
                tempBRowValues[col_b] += matrix_b->values[index_b];
                tempIndices[col_b] = col_b;
            }

            // Perform SIMD multiplication and accumulation
            for (uint64_t col_b = 0; col_b < (matrix_b->num_cols - (matrix_b->num_cols % 8)); col_b += 8)
            {
                __m256 simd_values_b = _mm256_loadu_ps(&tempBRowValues[col_b]);
                __m256 sind_temp_values = _mm256_loadu_ps(&tempValues[col_b]);
                sind_temp_values = _mm256_add_ps(sind_temp_values, _mm256_mul_ps(simdValueA, simd_values_b));
                _mm256_storeu_ps(&tempValues[col_b], sind_temp_values);
            }

            // Process remaining elements
            float a_value = matrix_a->values[indexA];
            for (uint64_t index_remain_b = matrix_b->num_cols - (matrix_b->num_cols % 8); index_remain_b < matrix_b->num_cols; ++index_remain_b)
            {
                tempValues[index_remain_b] += tempBRowValues[index_remain_b] * a_value;
            }

            // Reset the temporary B row array for the next iteration
            memset(tempBRowValues, 0, matrix_b->num_cols * sizeof(float));
        }

        // Transfer the non-zero values from the temporary values array to the result matrix
        uint64_t nzCountResult = 0;
        for (uint64_t index_col_result = 0; index_col_result < matrix_result->num_cols; ++index_col_result)
        {
            if (tempValues[index_col_result] != 0.0f)
            {
                uint64_t resultIndex = rowA * matrix_result->num_non_zero + nzCountResult;
                matrix_result->values[resultIndex] = tempValues[index_col_result];
                matrix_result->indices[resultIndex] = tempIndices[index_col_result];
                nzCountResult++;
            }
        }
    }

    // Free the allocated memory for the temporary arrays
    free(tempValues);
    free(tempIndices);
    free(tempBRowValues);
}
