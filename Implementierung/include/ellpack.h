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
int write_matrix_V1(const char *filename, const ELLPACKMatrix *matrix, uint64_t new_noNonZero);
int write_matrix_V2(const char *filename, const ELLPACKMatrix *matrix);
void matr_mult_ellpack(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result);
void matr_mult_ellpack_V1(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result);
void matr_mult_ellpack_V2(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result);
void matr_mult_ellpack_V3(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result);
void matr_mult_ellpack_V4(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result);
int compute_noNonZero(const ELLPACKMatrix *matrix);
void handle_error(const char *message, ELLPACKMatrix *matrix_a, ELLPACKMatrix *matrix_b, ELLPACKMatrix *result, int version);
void free_matrix(const ELLPACKMatrix *matrix, int version);

#endif // ELLPACK_H