#ifndef ELLPACK_H
#define ELLPACK_H

#include <stdint.h>

typedef struct
{
    uint64_t noRows;
    uint64_t noCols;
    uint64_t noNonZero;
    float *values;
    uint64_t *indices;
    float **result_values;
    uint64_t **result_indices;

} ELLPACKMatrix;

void matr_mult_ellpack(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result);
void matr_mult_ellpack_V1(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result);
void matr_mult_ellpack_V2(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result);
void matr_mult_ellpack_V3(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result);
void matr_mult_ellpack_V4(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result);
void free_matrix(const ELLPACKMatrix *matrix, int version);
void handle_error(const char *message, ELLPACKMatrix *matrix_a, ELLPACKMatrix *matrix_b, ELLPACKMatrix *result, int version);

#endif // ELLPACK_H