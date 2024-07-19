#ifndef MATRIX_IO_H
#define MATRIX_IO_H

#include "ellpack.h"

int read_matrix(const char *filename, ELLPACKMatrix *matrix);
int write_matrix(const char *filename, const ELLPACKMatrix *matrix, uint64_t new_noNonZero);
int compute_noNonZero(ELLPACKMatrix *matrix);
int control_indices(const char *filename, const ELLPACKMatrix *matrix);
int count_numbers_in_line(const char *line);

#endif // MATRIX_IO_H