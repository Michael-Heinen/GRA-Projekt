#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

typedef struct {
    int64_t noRows;
    int64_t noCols;
    int64_t noNonZero;
    float *values;
    int64_t *indices;
} ELLPACKMatrix;

void read_matrix(const char *filename, ELLPACKMatrix *matrix) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fscanf(file, "%" SCNd64 ",%" SCNd64 ",%" SCNd64, &matrix->noRows, &matrix->noCols, &matrix->noNonZero);

    matrix->values = (float *)malloc(matrix->noRows * matrix->noNonZero * sizeof(float));
    matrix->indices = (int64_t *)malloc(matrix->noRows * matrix->noNonZero * sizeof(int64_t));

    char ch;
    for (int64_t i = 0; i < matrix->noRows * matrix->noNonZero; ++i) {
        if (fscanf(file, " %c", &ch) == 1 && ch == '*') {
            matrix->values[i] = 0.0f;
            // Skip the comma
            fscanf(file, "%*c");
        } else {
            ungetc(ch, file); // Put back the character if it's not '*'
            fscanf(file, "%f,", &matrix->values[i]);
        }
    }
    for (int64_t i = 0; i < matrix->noRows * matrix->noNonZero; ++i) {
        if (fscanf(file, " %c", &ch) == 1 && ch == '*') {
            matrix->indices[i] = -1;
            // Skip the comma
            fscanf(file, "%*c");
        } else {
            ungetc(ch, file); // Put back the character if it's not '*'
            fscanf(file, "%" SCNu64 ",", &matrix->indices[i]);
        }
    }

    fclose(file);
}

void write_matrix(const char *filename, const ELLPACKMatrix *matrix, int64_t new_noNonZero) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error opening file %s\n", filename);
        exit(EXIT_FAILURE);
    }


    fprintf(file, "%" PRId64 ",%" PRId64 ",%" PRId64 "\n", matrix->noRows, matrix->noCols, new_noNonZero);

    for (int64_t i = 0; i < matrix->noRows; ++i) {
        for (int64_t j = 0; j < new_noNonZero; j++) {
            if (matrix->values[i * matrix->noNonZero + j] == 0.0f) {
                fprintf(file, "%c", '*');
            } else {
                fprintf(file, "%.1f", matrix->values[i * matrix->noNonZero + j]);
            }
            if (i * matrix->noNonZero + j < matrix->noRows * matrix->noNonZero - 1) {
                fprintf(file, ",");
            }
        }
    }
    fprintf(file, "\n");

    for (int64_t i = 0; i < matrix->noRows; ++i) {
        for (int64_t j = 0; j < new_noNonZero; j++) {
            if (matrix->indices[i * matrix->noNonZero + j] < 0) {
                fprintf(file, "%c", '*');
            } else {
                fprintf(file, "%" PRId64, matrix->indices[i * matrix->noNonZero + j]);
            }
            if (i * matrix->noNonZero + j < matrix->noRows * matrix->noNonZero - 1) {
                fprintf(file, ",");
            }
        }    
    }
    //fprintf(file, "\n");

    fclose(file);
}

void matr_mult_ellpack(const ELLPACKMatrix *a, const ELLPACKMatrix *b, ELLPACKMatrix *result) {
    if (a->noCols != b->noRows) {
        fprintf(stderr, "Matrix dimensions do not match for multiplication\n");
        exit(EXIT_FAILURE);
    }

    result->noRows = a->noRows;
    result->noCols = b->noCols;
    result->noNonZero = b->noCols;

    result->values = (float *)calloc(result->noRows * result->noNonZero, sizeof(float));
    result->indices = (int64_t *)calloc(result->noRows * result->noNonZero, sizeof(int64_t));


    int64_t arr_len = result->noRows * result->noNonZero;
    for (int64_t i = 0; i < arr_len; i++) {
        result->indices[i] = -1;
    }


    for (int64_t i = 0; i < a->noRows; ++i) {
        for (int64_t k = 0; k < a->noNonZero; ++k) {
            int64_t a_index = i * a->noNonZero + k;
            float a_value = a->values[a_index];
            if (a_value == 0.0) {
                continue;
            }
            int64_t a_col = a->indices[a_index];

            for (int64_t l = 0; l < b->noNonZero; ++l) {
                int64_t b_index = a_col * b->noNonZero + l;
                float b_value = b->values[b_index];
                int64_t b_col = b->indices[b_index];
                if (b_col <= -1) {
                    continue;
                }

                for (int64_t m = 0; i < result->noNonZero; m++) {
                    if (result->indices[i * result->noNonZero + m] < 0 || result->indices[i * result->noNonZero + m] == b_col) {
                        result->values[i * result->noNonZero + m] += a_value * b_value;
                        result->indices[i * result->noNonZero + m] = b_col;
                        break;
                    }                    
                }
            }
        }
    }

}


//compute noNonZero in result matrix
int compute_noNonZero(ELLPACKMatrix *matrix) {
    int64_t maxNoNonZero = 0;
    for (int64_t i = 0; i < matrix->noCols; i++) {
        int64_t tmpNoNonZero = 0;
        for (int64_t j = 0; j < matrix->noNonZero; j++) {
            if (matrix->indices[i * matrix->noNonZero + j] < 0) {
                break;
            }
            tmpNoNonZero++;
        }
        if (tmpNoNonZero > maxNoNonZero) {
            maxNoNonZero = tmpNoNonZero;
        }
    }
    return maxNoNonZero;
}



int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <matrix_a_file> <matrix_b_file> <result_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    ELLPACKMatrix a, b, result;
    read_matrix(argv[1], &a);
    read_matrix(argv[2], &b);

    matr_mult_ellpack(&a, &b, &result);

    int64_t new_noNonZero = compute_noNonZero(&result);

    write_matrix(argv[3], &result, new_noNonZero);

    free(a.values);
    free(a.indices);
    free(b.values);
    free(b.indices);
    free(result.values);
    free(result.indices);

    return EXIT_SUCCESS;
}
