#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parseFirstLine (char *line, int *result_numbers) {
    char *token = strtok(line, ",");
    int index = 0;

    while (token != NULL) {
        if (strcmp(token, "*") != 0) {
            result_numbers[index++] = atoi(token);
        }
        token = strtok(NULL, ",");
    }
}

void parseSecondLine (char *line, float *values) {
    char *token = strtok(line, ",");
    int index = 0;

    while (token != NULL) {
        if (strcmp(token, "*") == 0) {
            values[index++] = 0.0f;
        } else {
            values[index++] = atof(token);
        }
        token = strtok(NULL, ",");
    }

}

void parseThirdLine (char *line, int *indices) {
    char *token = strtok(line, ",");
    int index = 0;

    while (token != NULL) {
        if (strcmp(token, "*") == 0) {
            indices[index++] = -1;
        } else {
            indices[index++] = atoi(token);
        }
        token = strtok(NULL, ",");
    }

}

/*
void parseNumbersOfMatrix (float *result, char *values_a, char *values_b, char *indices_a, char* indices_b, uint64_t *index1, uint64_t *index2, int *result_numbers_a, int *result_numbers_b) {

        for (int i = 0; i < result_numbers_a[2]; i++) {

            for (int k = 0; k < result_numbers_b[2]; k++) {

                 index_a = indices_a[index1*result_numbers[2]+i];
                uint64_t index_b = indices_b[index2*result_numbers[2]+k];

                if index_a

                if (index_a == index_b) {
                    result += values_a[result_numbers_a[2]*index1+index_a] * values_b[result_numbers_b[2]*index2+index_b]
                }
            }
        }

}
*/

int main() {

    // Implement all File Paths + open files in readmode
    const char *filePath_input_a = "input_a.txt";
    const char *filePath_input_b = "input_b.txt";
    const char *filePath_output = "output.txt";

    FILE *input_a = fopen(filePath_input_a, "r");
    FILE *input_b = fopen(filePath_input_b, "r");
    FILE *output = fopen(filePath_output, "w");


    if (input_a == NULL) {
        perror("Fehler beim Öffnen des Input A");
        return EXIT_FAILURE;
    }

    if (input_b == NULL) {
        perror("Fehler beim Öffnen des Input B");
        return EXIT_FAILURE;
    }


    //save first lines form the input files
    char line1_a[256];
    char line1_b[256];

    fgets(line1_a, 256, input_a);
    printf("line 1a: %s", line1_a);
    fgets(line1_b, 256, input_b);
    printf("line 1b: %s", line1_b);


    // parse first lines of inputs and safe the results in numbers_line1_a/b
    int numbers_line1_a[3];
    int numbers_line1_b[3];
    parseFirstLine(line1_a, numbers_line1_a);
    parseFirstLine(line1_b, numbers_line1_b);

    for(int i = 0; i < 3; i++) {
        printf("A: %d\n", numbers_line1_a[i]);
        printf("B: %d\n", numbers_line1_b[i]);
    }


    //Überprüfung ob man die beiden Matrizen miteinander multiplizieren kann
    if (numbers_line1_a[1] != numbers_line1_b[0]) {
        perror("Die beiden Matrizen können nicht miteinander multipliziert werden ! Falsches Format");
        return EXIT_FAILURE;
    }


    // Einlesen der Werte und Indices

    char line2_a[256];
    char line3_a[256];

    char line2_b[256];
    char line3_b[256];

    fgets(line2_a, 256, input_a);
    printf("line 2a: %s", line2_a);

    fgets(line3_a, 256, input_a);
    printf("line 3a: %s\n", line3_a);

    fgets(line2_b, 256, input_b);
    printf("line 2b: %s", line2_b);

    fgets(line3_b, 256, input_b);
    printf("line 3b: %s\n", line3_b);

    float values_line2_a[numbers_line1_a[0]*numbers_line1_a[2]];
    float values_line2_b[numbers_line1_b[0]*numbers_line1_b[2]];

    uint64_t indices_line3_a[numbers_line1_a[0]*numbers_line1_a[2]];
    uint64_t indices_line3_b[numbers_line1_b[0]*numbers_line1_b[2]];

    parseSecondLine(line2_a, values_line2_a);
    parseSecondLine(line2_b, values_line2_b);

    for (int i = 0; i < 8; i++) {
        printf("Float: %f\n", values_line2_a[i]);
    }


    //Lösungsmatrix erstellen + Berechnung
    float result[numbers_line1_a[0]][numbers_line1_b[1]];

    char line1_out[256];
    char line2_out[256];
    char line3_out[256];

 /*   for (int i = 0; i < numbers_line1_a[0], i++); {
        for (int k = 0; k < numbers_line1_b[1], i++); {

        }

    }

*/

    fclose(input_a);
    fclose(input_b);

}