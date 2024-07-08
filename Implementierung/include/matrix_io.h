#ifndef MATRIX_IO_H
#define MATRIX_IO_H

#include "ellpack.h"

void read_matrix(const char *filename, ELLPACKMatrix *matrix);
void write_matrix(const char *filename, const ELLPACKMatrix *matrix, int64_t new_noNonZero);

#endif // MATRIX_IO_H