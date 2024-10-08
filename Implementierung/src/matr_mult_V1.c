#include "ellpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void matr_mult_ellpack_V1(const ELLPACKMatrix *restrict matrix_a, const ELLPACKMatrix *restrict matrix_b, ELLPACKMatrix *restrict matrix_result)
{
    bool free_input_matrix = false;

    // Check if dimensions match
    if (matrix_a->num_cols != matrix_b->num_rows)
    {
        fprintf(stderr, "Matrix dimensions do not match for multiplication (matr_mult_ellpack_V1 (V1))\n");
        free_input_matrix = true;
        goto free_input_matrix;
    }

    // Initialize dimensions and allocate memory for result_matrix
    matrix_result->num_rows = matrix_a->num_rows;
    matrix_result->num_cols = matrix_b->num_cols;
    matrix_result->num_non_zero = matrix_b->num_cols;

    matrix_result->values = (float *)calloc(matrix_result->num_rows * matrix_result->num_non_zero, sizeof(float));
    matrix_result->indices = (uint64_t *)calloc(matrix_result->num_rows * matrix_result->num_non_zero, sizeof(uint64_t));

    if (!matrix_result->values || !matrix_result->indices)
    {
        free(matrix_result->values);
        free(matrix_result->indices);
        fprintf(stderr, "Memory allocation failed (matr_mult_ellpack_V1 (V1))\n");
        free_input_matrix = true;
        goto free_input_matrix;
    }

    // Iterate over rows of matrix_a
    for (uint64_t i = 0; i < matrix_a->num_rows; ++i)
    {
        // Iterate over non_zero elements of current row of matrix_a
        for (uint64_t k = 0; k < matrix_a->num_non_zero; ++k)
        {   
            uint64_t a_index = i * matrix_a->num_non_zero + k;
            float a_value = matrix_a->values[a_index];

            if (a_value == 0.0)
            {
                continue;
            }

            uint64_t a_col = matrix_a->indices[a_index];
            
            // Iterate over non-zero elements of row in matrix_b
            for (uint64_t l = 0; l < matrix_b->num_non_zero; ++l)
            {
                uint64_t b_index = a_col * matrix_b->num_non_zero + l;
                float b_value = matrix_b->values[b_index];

                if (b_value == 0.0f)
                {
                    continue;
                }

                uint64_t b_col = matrix_b->indices[b_index];

                // Compute result
                for (uint64_t m = 0; m < matrix_result->num_non_zero; m++)
                {
                    if (matrix_result->values[i * matrix_result->num_non_zero + m] == 0.0f || matrix_result->indices[i * matrix_result->num_non_zero + m] == b_col)
                    {
                        matrix_result->values[i * matrix_result->num_non_zero + m] += a_value * b_value;
                        matrix_result->indices[i * matrix_result->num_non_zero + m] = b_col;
                        break;
                    }
                }
            }
        }
    }

free_input_matrix:
    if (free_input_matrix)
    {
        free(matrix_a->values);
        free(matrix_a->indices);
        free(matrix_b->values);
        free(matrix_b->indices);
        exit(EXIT_FAILURE);
    }
}
