#ifndef MATRIX_IO_H
#define MATRIX_IO_H

#include "ellpack.h"

int read_matrix(const char *filename, ELLPACKMatrix *matrix);
int write_matrix_V1(const char *filename, const ELLPACKMatrix *matrix, uint64_t num_non_zero);
int write_matrix_V2(const char *filename, const ELLPACKMatrix *matrix);
int compute_num_non_zero(ELLPACKMatrix *matrix);

#endif // MATRIX_IO_H