#ifndef ELLPACK_H
#define ELLPACK_H

#include <stdint.h>

typedef struct
{
    int64_t noRows;
    int64_t noCols;
    int64_t noNonZero;
    float *values;
    int64_t *indices;
} ELLPACKMatrix;

void read_matrix(const char *filename, ELLPACKMatrix *matrix);
void write_matrix(const char *filename, const ELLPACKMatrix *matrix, int64_t new_noNonZero);
void matr_mult_ellpack(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result);
void matr_mult_ellpack_V1(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result);
void matr_mult_ellpack_V2(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result);
int compute_noNonZero(ELLPACKMatrix *matrix);

#endif // ELLPACK_H