#ifndef ELLPACK_H
#define ELLPACK_H

#include <stdint.h>

typedef struct
{
    uint64_t num_rows;
    uint64_t num_cols;
    uint64_t num_non_zero;
    float *values;
    uint64_t *indices;
    float **result_values;
    uint64_t **result_indices;

} ELLPACKMatrix;

void matr_mult_ellpack(const ELLPACKMatrix *matrix_a, const ELLPACKMatrix *matrix_b, ELLPACKMatrix *matrix_result);
void matr_mult_ellpack_V1(const ELLPACKMatrix *matrix_a, const ELLPACKMatrix *matrix_b, ELLPACKMatrix *matrix_result);
void matr_mult_ellpack_V2(const ELLPACKMatrix *matrix_a, const ELLPACKMatrix *matrix_b, ELLPACKMatrix *matrix_result);
void matr_mult_ellpack_V3(const ELLPACKMatrix *matrix_a, const ELLPACKMatrix *matrix_b, ELLPACKMatrix *matrix_result);
void matr_mult_ellpack_V4(const ELLPACKMatrix *matrix_a, const ELLPACKMatrix *matrix_b, ELLPACKMatrix *matrix_result);
void matr_mult_ellpack_V5(const ELLPACKMatrix *matrix_a, const ELLPACKMatrix *matrix_b, ELLPACKMatrix *matrix_result);

#endif // ELLPACK_H