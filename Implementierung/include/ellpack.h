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

int read_matrix(const char *filename, ELLPACKMatrix *matrix);
int write_matrix(const char *filename, const ELLPACKMatrix *matrix, uint64_t new_noNonZero);
void matr_mult_ellpack(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result);
void matr_mult_ellpack_V1(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result);
void matr_mult_ellpack_V2(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result);
void matr_mult_ellpack_V3(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result);
void matr_mult_ellpack_V4(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result);
int compute_noNonZero(ELLPACKMatrix *matrix);
void free_matrix(ELLPACKMatrix *matrix);

#endif // ELLPACK_H