#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

// Function to multiply two matrices in ELLPACK format
void matr_mult_ellpack_V2(const ELLPACKMatrix *restrict matrix_a, const ELLPACKMatrix *restrict matrix_b, ELLPACKMatrix *restrict matrix_result)
{
    // Check if the number of columns in matrix_a is equal to the number of rows in matrix_b
    if (matrix_a->num_cols != matrix_b->num_rows)
    {
        fprintf(stderr, "Matrix dimensions do not match for multiplication\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the matrix_result matrix dimensions and allocate space for values and indices
    matrix_result->num_rows = matrix_a->num_rows;
    matrix_result->num_cols = matrix_b->num_cols;

    matrix_result->result_values = (float **)calloc(matrix_result->num_rows, sizeof(float *));
    matrix_result->result_indices = (uint64_t **)calloc(matrix_result->num_rows, sizeof(uint64_t *));

    if (!matrix_result->result_values || !matrix_result->result_indices)
    {
        free(matrix_result->result_values);
        free(matrix_result->result_indices);
        fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V2 (V2))\n");
        exit(EXIT_FAILURE);
    }

    // Allocate temporary arrays to store intermediate results for the current row

    float *temp_values_row_b = (float *)calloc(matrix_b->num_cols, sizeof(float));
    
    if (!temp_values_row_b)
        {
            free(temp_values_row_b);
            free(matrix_result->result_values);
            free(matrix_result->result_indices);
            fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V2 (V2))\n");
            exit(EXIT_FAILURE);
        }

    uint64_t max_non_zero = 0;

    // Loop through each row of matrix_a
    for (uint64_t curr_row_a = 0; curr_row_a < matrix_a->num_rows; ++curr_row_a)
    {
        matrix_result->result_values[curr_row_a] = (float *)calloc(matrix_result->num_cols, sizeof(float));
        matrix_result->result_indices[curr_row_a] = (uint64_t *)calloc(matrix_result->num_cols, sizeof(uint64_t));

        if (!matrix_result->result_values[curr_row_a] || !matrix_result->result_indices[curr_row_a])
        {
            for (uint64_t i = 0; i <= curr_row_a; ++i)
            {
                free(matrix_result->result_values[i]);
                free(matrix_result->result_indices[i]);
            }
            
            free(temp_values_row_b);
            free(matrix_result->result_values);
            free(matrix_result->result_indices);
            fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V3)\n");
            exit(EXIT_FAILURE);
        }

        // Loop through each non-zero element in the current row of matrix_a
        for (uint64_t curr_non_zero_a = 0; curr_non_zero_a < matrix_a->num_non_zero; ++curr_non_zero_a)

        {
            // Calculate the index of the current non-zero element in matrix_a
            uint64_t index_a = curr_row_a * matrix_a->num_non_zero + curr_non_zero_a;
            // Load the current value from matrix_a into a SIMD register and broadcast it
            __m128 simd_value_a = _mm_set1_ps(matrix_a->values[index_a]);
            // Get the column index from the current non-zero element in matrix_a
            uint64_t col_a = matrix_a->indices[index_a];

            // Calculate the base index of the row in matrix_b corresponding to the column index in matrix_a
            uint64_t base_index_b = col_a * matrix_b->num_non_zero;

            // Loop through each non-zero element in the current row of matrix_b
            for (uint64_t num_non_zero_b = 0; num_non_zero_b < matrix_b->num_non_zero; ++num_non_zero_b)
            {
                // Calculate the index of the current non-zero element in matrix_b
                uint64_t index_b = base_index_b + num_non_zero_b;
                // Get the column index and value from the current non-zero element in matrix_b
                uint64_t col_b = matrix_b->indices[index_b];
                temp_values_row_b[col_b] += matrix_b->values[index_b];
                // Store the column index in the temporary indices array
                matrix_result->result_indices[curr_row_a][col_b] = col_b;
            }

            // Loop through the temporary B row array in chunks of 4 elements (SIMD width)
            for (uint64_t col_b = 0; col_b < (matrix_b->num_cols - (matrix_b->num_cols % 4)); col_b += 4)
            {
                // Load 4 values from the temporary B row array into a SIMD register
                __m128 simd_values_b = _mm_load_ps(&temp_values_row_b[col_b]);
                // Load 4 values from the temporary values array into a SIMD register
                __m128 sind_temp_values = _mm_load_ps(&matrix_result->result_values[curr_row_a][col_b]);
                // Multiply and accumulate the values
                sind_temp_values = _mm_add_ps(sind_temp_values, _mm_mul_ps(simd_value_a, simd_values_b));
                // Store the matrix_result back into the temporary values array
                _mm_store_ps(&matrix_result->result_values[curr_row_a][col_b], sind_temp_values);
            }

            float value_a = matrix_a->values[index_a];
            base_index_b = (matrix_b->num_cols - (matrix_b->num_cols % 4));
            for (uint64_t index_remain_b = 0; index_remain_b < matrix_b->num_cols % 4; index_remain_b++)
            {
                float calc_temp = temp_values_row_b[base_index_b + index_remain_b] * value_a;
                matrix_result->result_values[curr_row_a][base_index_b + index_remain_b] += calc_temp;
            }

            // Reset the temporary B row array for the next iteration
            for (uint64_t col_b = 0; col_b < matrix_b->num_cols; ++col_b)
            {
                temp_values_row_b[col_b] = 0.0f;
            }
        }

        // Transfer the non-zero values from the temporary values array to the matrix_result matrix
        uint64_t cnt_non_zero = 0;
        for (uint64_t index_col_result = 0; index_col_result < matrix_result->num_cols; ++index_col_result)
        {
            if (matrix_result->result_values[curr_row_a][index_col_result] != 0.0f)
            {
                // Store the value and index in the9 matrix_result matrix
                matrix_result->result_values[curr_row_a][cnt_non_zero] = matrix_result->result_values[curr_row_a][index_col_result];
                matrix_result->result_indices[curr_row_a][cnt_non_zero] = matrix_result->result_indices[curr_row_a][index_col_result];
                cnt_non_zero++;
            }
        }

        if (cnt_non_zero > max_non_zero)
        {
            max_non_zero = cnt_non_zero;
        }
    }

    matrix_result->num_non_zero = max_non_zero;

    // Free the allocated memory for the temporary arrays
    free(temp_values_row_b);
}
