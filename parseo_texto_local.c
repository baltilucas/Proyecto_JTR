#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PASSWORD_LENGTH 256
#define MAX_LINE_LENGTH 512
#define MAX_LINES 1000

char** read_dictionary(const char *filename, int *count){
    char lines[MAX_LINES][MAX_LINE_LENGTH];
    int line_count = 0;
    FILE* fp;

    fp = fopen(filename, "r");

    if (fp == NULL) {
        perror("Error opening file");
        return NULL;
    }
    while (fgets(lines[line_count], MAX_LINE_LENGTH, fp) != NULL) {
        lines[line_count][strcspn(lines[line_count], "\n")] = '\0';
        line_count++;

        if (line_count >= MAX_LINES) {
            printf("Warning: Maximum lines reached, stopping read.\n");
            break;
        }
    }
    *count = line_count;
    
    fclose(fp);

    char **dictionary = malloc(line_count * sizeof(char*));
    for (int i = 0; i < line_count; i++) {

        dictionary[i] = malloc(MAX_PASSWORD_LENGTH * sizeof(char)); 
        strcpy(dictionary[i], lines[i]); 
        printf("la palabra: %s, La copiada: %s\n", lines[i], dictionary[i]);
    }

    return dictionary;
}

void free_dictionary(char **dictionary, int count) {
    for (int i = 0; i < count; i++) {
        dictionary[i] = NULL;
        free(dictionary[i]);  
    }
    dictionary = NULL;
    free(dictionary);
}



int main(int argc, char *argv[]) {

    const char *dict_file = argv[1];
        // Leer diccionario
    int dict_count;
    char **dictionary = read_dictionary(dict_file, &dict_count);
        if (dict_count == 0) {
        fprintf(stderr, "[ERROR] El diccionario está vacío\n");
        free_dictionary(dictionary, dict_count);
        return 1;
    }
    
    printf("[✓] Diccionario cargado: %d passwords\n", dict_count);
    return 0;
}