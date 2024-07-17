#ifndef MATRIX_IO_H
#define MATRIX_IO_H

#include "ellpack.h"

int read_matrix(const char *filename, ELLPACKMatrix *matrix);
int write_matrix(const char *filename, const ELLPACKMatrix *matrix, uint64_t new_noNonZero);

#endif // MATRIX_IO_H