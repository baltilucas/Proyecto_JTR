#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "md5.h"

#define MAX_PASSWORD_LENGTH 256
#define MAX_LINE_LENGTH 512
#define MAX_LINES 1000

// Estructura para estadísticas
typedef struct {
    long total_attempts;
    double elapsed_time;
    long passwords_per_second;
} Stats;

/**
 * Lee un diccionario de passwords desde un archivo
 * @param filename Nombre del archivo
 * @param count Puntero para almacenar el número de passwords
 * @return Array de strings con los passwords, o NULL si hay error
 */

char** read_dictionary(const char *filename, int *count){
    char lines[MAX_LINES][MAX_LINE_LENGTH];
    int line_count = 0;
    FILE* fp;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
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
    
    free(fp);
    char **dictionary = malloc(line_count * sizeof(char*));
    for (int i = 0; i < line_count; i++) {
        dictionary[i] = malloc(MAX_PASSWORD_LENGTH * sizeof(char*));
        dictionary[i] = &lines[i];
        printf("la palabra: %s, La copiada: %s\n", lines[i], dictionary[i]);
    }

    return dictionary;
}

/**
 * Libera la memoria del diccionario
 */
void free_dictionary(char **dictionary, int count) {
    for (int i = 0; i < count; i++) {
        dictionary[i] = NULL;
        free(dictionary[i]);  
    }
    dictionary = NULL;
    free(dictionary);
}

/**
 * Realiza un ataque de diccionario contra un hash
 * @param target_hash Hash objetivo a crackear
 * @param dictionary Array de passwords candidatos
 * @param count Número de passwords en el diccionario
 * @param stats Estructura para almacenar estadísticas
 * @return 1 si encuentra el password, 0 si no
 */
int dictionary_attack(const char *target_hash, char **dictionary, int count, Stats *stats) {
    char hash[33];
    clock_t start = clock();
    
    printf("[+] Iniciando ataque de diccionario...\n");
    printf("[+] Passwords a probar: %d\n\n", count);

    for (int i = 0; i < count; i++) {
        md5_hash(dictionary[i], hash);
        printf("Palabra: %s ==> Hash: %s == Hash Objetivo: %s \n", dictionary[i], hash, target_hash);
        stats->total_attempts++;
        if (strcmp(hash, target_hash) == 0) {
            clock_t end = clock();
            stats->elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;
            
            if (stats->elapsed_time > 0) {
                stats->passwords_per_second = (long)(stats->total_attempts / stats->elapsed_time);
            } else {
                stats->passwords_per_second = stats->total_attempts;
            }
            
            printf("\n");
            printf("=================================================\n");
            printf("   PASSWORD ENCONTRADO!\n");
            printf("=================================================\n");
            printf("Password: %s\n", dictionary[i]);
            printf("Hash:     %s\n", hash);
            printf("Intentos: %ld\n", stats->total_attempts);
            printf("Tiempo:   %.3f segundos\n", stats->elapsed_time);
            printf("Velocidad: %ld passwords/seg\n", stats->passwords_per_second);
            printf("=================================================\n");
            return 1;
        }
        
        // Mostrar progreso cada 1000 intentos
        if (i % 1000 == 0 && i > 0) {
            printf("\r[.] Progreso: %d/%d passwords (%.1f%%)    ", 
                   i, count, (100.0 * i) / count);
            fflush(stdout);
        }
    }
    
    clock_t end = clock();
    stats->elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    if (stats->elapsed_time > 0) {
        stats->passwords_per_second = (long)(stats->total_attempts / stats->elapsed_time);
    } else {
        stats->passwords_per_second = stats->total_attempts;
    }
    
    printf("\n\n");
    printf("=================================================\n");
    printf("    Password NO encontrado\n");
    printf("=================================================\n");
    printf("Total intentos: %ld\n", stats->total_attempts);
    printf("Tiempo total:   %.3f segundos\n", stats->elapsed_time);
    printf("Velocidad:      %ld passwords/seg\n", stats->passwords_per_second);
    printf("=================================================\n");
    
    return 0;
}

void print_banner() {
    printf("\n");
    printf("╔═══════════════════════════════════════════════╗\n");
    printf("║     JOHN THE RIPPER - VERSIÓN SECUENCIAL     ║\n");
    printf("║   Programación Paralela y Distribuida        ║\n");
    printf("║   Unidad I: Fundamentos                      ║\n");
    printf("╚═══════════════════════════════════════════════╝\n");
    printf("\n");
}

void print_usage(const char *program_name) {
    printf("Uso: %s <hash_objetivo> <archivo_diccionario>\n\n", program_name);
    printf("Ejemplos:\n");
    printf("  %s 8b1a9953c4611296a827abf8c47804d7 diccionario.txt\n", program_name);
    printf("  %s 5f4dcc3b5aa765d61d8327deb882cf99 wordlist.txt\n\n", program_name);
    printf("Hashes de prueba:\n");
    printf("  cthulhu:  8b1a9953c4611296a827abf8c47804d7\n");
    printf("  password: 5f4dcc3b5aa765d61d8327deb882cf99\n");
    printf("  lovecraft: 18126e7bd3f84b3f3e4df094def5b7de\n");
}

int main(int argc, char *argv[]) {
    print_banner();
    
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char *target_hash = argv[1];
    const char *dict_file = argv[2];
    
    // Validar formato del hash
    if (strlen(target_hash) != 32) {
        fprintf(stderr, "[ERROR] Hash MD5 inválido (debe tener 32 caracteres hexadecimales)\n");
        return 1;
    }
    
    printf("[i] Hash objetivo: %s\n", target_hash);
    printf("[i] Diccionario:   %s\n\n", dict_file);
    
    // Leer diccionario
    int dict_count;
    char **dictionary = read_dictionary(dict_file, &dict_count);

    if (!dictionary) {
        fprintf(stderr, "[ERROR] No se pudo leer el diccionario\n");
        return 1;
    }
    
    if (dict_count == 0) {
        fprintf(stderr, "[ERROR] El diccionario está vacío\n");
        free_dictionary(dictionary, dict_count);
        return 1;
    }
    
    printf("[✓] Diccionario cargado: %d passwords\n", dict_count);
    
    // Ejecutar ataque
    Stats stats = {0, 0.0, 0};
    int found = dictionary_attack(target_hash, dictionary, dict_count, &stats);
    
    // Liberar memoria
    free_dictionary(dictionary, dict_count);
    
    return found ? 0 : 1;
}
