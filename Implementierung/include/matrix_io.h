#ifndef MATRIX_IO_H
#define MATRIX_IO_H

#include "ellpack.h"

int read_matrix(const char *filename, ELLPACKMatrix *matrix);
int write_matrix_V1(const char *filename, const ELLPACKMatrix *matrix, uint64_t new_noNonZero);
int write_matrix_V2(const char *filename, const ELLPACKMatrix *matrix);
int compute_noNonZero(const ELLPACKMatrix *matrix);
int control_indices(const char *filename, const ELLPACKMatrix *matrix);
int count_numbers_in_line(const char *line);

#endif // MATRIX_IO_H