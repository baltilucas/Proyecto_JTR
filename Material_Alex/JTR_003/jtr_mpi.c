/*
 * John the Ripper - Versión MPI (Memoria Distribuida)
 * 
 * Compilación:
 *   mpicc -o jtr_mpi jtr_mpi.c md5.c -O3 -lm
 * 
 * Ejecución:
 *   mpirun -np 4 ./jtr_mpi hashes.txt diccionario.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "md5.h"
#include <mpi.h>

#define MAX_LINE 256
#define MAX_HASH 33
#define MAX_PASSWORD 128
#define BUFFER_SIZE 100
#define MAX_HASHES 1000

// Tags para mensajes MPI
#define TAG_WORK_REQUEST  1
#define TAG_WORK_ASSIGN   2
#define TAG_WORK_RESULT   3
#define TAG_TERMINATE     4

// Estructura para resultado de cracking
typedef struct {
    char hash[MAX_HASH];
    char password[MAX_PASSWORD];
    int found;
    int worker_rank;
} CrackResult;

// Estructura para estadísticas
typedef struct {
    long passwords_tested;
    double time_spent;
    int hashes_cracked;
} WorkerStats;


/*
 * Calcula hash MD5 de una cadena
 */
void calculate_md5(const char *str, char *output) {
    unsigned char digest[16];
    MD5_CTX ctx;
    
    MD5Init(&ctx);
    MD5Update(&ctx, (unsigned char*)str, strlen(str));
    MD5Final(digest, &ctx);
    
    for (int i = 0; i < 16; i++) {
        sprintf(&output[i*2], "%02x", (unsigned int)digest[i]);
    }
    output[32] = '\0';
}


/*
 * Lee hashes del archivo
 */
int read_hashes(const char *filename, char hashes[][MAX_HASH], int max_hashes) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: no se puede abrir %s\n", filename);
        return -1;
    }
    
    int count = 0;
    char line[MAX_LINE];
    
    while (fgets(line, MAX_LINE, file) && count < max_hashes) {
        line[strcspn(line, "\n")] = 0;
        
        if (strlen(line) == 32) {
            strcpy(hashes[count], line);
            count++;
        }
    }
    
    fclose(file);
    return count;
}


/*
 * Cuenta palabras en el diccionario
 */
long count_dictionary_words(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;
    
    long count = 0;
    char line[MAX_LINE];
    
    while (fgets(line, MAX_LINE, file)) {
        count++;
    }
    
    fclose(file);
    return count;
}


/*
 * PROCESO MAESTRO (rank 0)
 */
void master_process(const char *hash_file, const char *dict_file, int num_workers) {
    printf("\n=== JOHN THE RIPPER - MPI VERSION ===\n");
    printf("Proceso Maestro iniciado\n");
    printf("Workers disponibles: %d\n\n", num_workers);
    
    // Leer hashes
    char hashes[MAX_HASHES][MAX_HASH];
    int num_hashes = read_hashes(hash_file, hashes, MAX_HASHES);
    
    if (num_hashes <= 0) {
        fprintf(stderr, "Error: no se pudieron leer hashes\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        return;
    }
    
    printf("Hashes cargados: %d\n", num_hashes);
    
    // Contar palabras
    long total_words = count_dictionary_words(dict_file);
    printf("Palabras en diccionario: %ld\n\n", total_words);
    
    // Abrir diccionario
    FILE *dict = fopen(dict_file, "r");
    if (!dict) {
        fprintf(stderr, "Error: no se puede abrir diccionario\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        return;
    }
    
    // Estadísticas
    int hashes_cracked = 0;
    int found_flags[MAX_HASHES];
    CrackResult results[MAX_HASHES];
    
    for (int i = 0; i < MAX_HASHES; i++) {
        found_flags[i] = 0;
    }
    
    // Buffer de palabras
    char word_buffer[BUFFER_SIZE][MAX_PASSWORD];
    int buffer_count = 0;
    
    int active_workers = num_workers;
    
    double start_time = MPI_Wtime();
    
    printf("Iniciando cracking...\n");
    printf("Distribuyendo trabajo a %d workers...\n\n", num_workers);
    
    // Loop principal
    while (active_workers > 0) {
        MPI_Status status;
        int request;
        
        // Esperar petición
        MPI_Recv(&request, 1, MPI_INT, MPI_ANY_SOURCE, TAG_WORK_REQUEST, 
                 MPI_COMM_WORLD, &status);
        
        int worker_rank = status.MPI_SOURCE;
        
        // Verificar si hay resultado
        if (request == -1) {
            CrackResult result;
            
            // Recibir longitud y hash
            int hash_len;
            MPI_Recv(&hash_len, 1, MPI_INT, worker_rank, 
                     TAG_WORK_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(result.hash, hash_len, MPI_CHAR, worker_rank, 
                     TAG_WORK_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // Recibir longitud y password
            int password_len;
            MPI_Recv(&password_len, 1, MPI_INT, worker_rank, 
                     TAG_WORK_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(result.password, password_len, MPI_CHAR, worker_rank, 
                     TAG_WORK_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // Recibir found y worker_rank
            MPI_Recv(&result.found, 1, MPI_INT, worker_rank, 
                     TAG_WORK_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&result.worker_rank, 1, MPI_INT, worker_rank, 
                     TAG_WORK_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            if (result.found) {
                printf("✓ ENCONTRADO por Worker %d: %s = %s\n", 
                       result.worker_rank, result.hash, result.password);
                
                for (int i = 0; i < num_hashes; i++) {
                    if (strcmp(hashes[i], result.hash) == 0 && !found_flags[i]) {
                        found_flags[i] = 1;
                        results[hashes_cracked] = result;
                        hashes_cracked++;
                        break;
                    }
                }
            }
        }
        
        // Preparar siguiente lote
        buffer_count = 0;
        
        while (buffer_count < BUFFER_SIZE && 
               fgets(word_buffer[buffer_count], MAX_PASSWORD, dict)) {
            word_buffer[buffer_count][strcspn(word_buffer[buffer_count], "\n")] = 0;
            buffer_count++;
        }
        
        if (buffer_count > 0) {
            // Enviar cantidad de palabras
            MPI_Send(&buffer_count, 1, MPI_INT, worker_rank, TAG_WORK_ASSIGN, 
                     MPI_COMM_WORLD);
            
            // Contar hashes no encontrados
            int remaining_hashes = 0;
            for (int i = 0; i < num_hashes; i++) {
                if (!found_flags[i]) remaining_hashes++;
            }
            
            // Enviar cantidad de hashes
            MPI_Send(&remaining_hashes, 1, MPI_INT, worker_rank, TAG_WORK_ASSIGN, 
                     MPI_COMM_WORLD);
            
            // Enviar hashes no encontrados
            for (int i = 0; i < num_hashes; i++) {
                if (!found_flags[i]) {
                    // Enviar longitud del hash
                    int hash_len = strlen(hashes[i]) + 1;
                    MPI_Send(&hash_len, 1, MPI_INT, worker_rank, TAG_WORK_ASSIGN, MPI_COMM_WORLD);
                    // Enviar hash
                    MPI_Send(hashes[i], hash_len, MPI_CHAR, worker_rank, 
                             TAG_WORK_ASSIGN, MPI_COMM_WORLD);
                }
            }
            
            // Enviar palabras
            for (int i = 0; i < buffer_count; i++) {
                // Enviar longitud de la palabra
                int word_len = strlen(word_buffer[i]) + 1;
                MPI_Send(&word_len, 1, MPI_INT, worker_rank, TAG_WORK_ASSIGN, MPI_COMM_WORLD);
                // Enviar palabra
                MPI_Send(word_buffer[i], word_len, MPI_CHAR, worker_rank, 
                         TAG_WORK_ASSIGN, MPI_COMM_WORLD);
            }
        } else {
            // No hay más trabajo
            int terminate = 0;
            MPI_Send(&terminate, 1, MPI_INT, worker_rank, TAG_TERMINATE, 
                     MPI_COMM_WORLD);
            active_workers--;
        }
    }
    
    fclose(dict);
    
    double end_time = MPI_Wtime();
    double total_time = end_time - start_time;
    
    // Recolectar estadísticas
    WorkerStats *all_stats = malloc(num_workers * sizeof(WorkerStats));
    
    for (int i = 1; i <= num_workers; i++) {
        MPI_Recv(&all_stats[i-1], sizeof(WorkerStats), MPI_BYTE, i, 
                 TAG_WORK_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    
    // Imprimir resultados
    printf("\n=== RESULTADOS FINALES ===\n");
    printf("Tiempo total: %.4f segundos\n", total_time);
    printf("Hashes crackeados: %d / %d\n", hashes_cracked, num_hashes);
    printf("Tasa de éxito: %.2f%%\n\n", 
           (float)hashes_cracked / num_hashes * 100);
    
    if (hashes_cracked > 0) {
        printf("Contraseñas encontradas:\n");
        printf("%-34s %-20s Worker\n", "Hash", "Password");
        printf("----------------------------------------------------------\n");
        for (int i = 0; i < hashes_cracked; i++) {
            printf("%-34s %-20s %d\n", 
                   results[i].hash, results[i].password, results[i].worker_rank);
        }
        printf("\n");
    }
    
    // Estadísticas por worker
    printf("=== ESTADÍSTICAS POR WORKER ===\n");
    printf("Worker  Passwords   Time (s)   Hashes   Passwords/s\n");
    printf("------  ----------  ---------  -------  -----------\n");
    
    long total_passwords = 0;
    
    for (int i = 0; i < num_workers; i++) {
        total_passwords += all_stats[i].passwords_tested;
        double rate = all_stats[i].time_spent > 0 ? 
                      all_stats[i].passwords_tested / all_stats[i].time_spent : 0;
        
        printf("  %2d    %10ld   %8.4f    %4d    %10.0f\n",
               i + 1,
               all_stats[i].passwords_tested,
               all_stats[i].time_spent,
               all_stats[i].hashes_cracked,
               rate);
    }
    
    printf("\nTotal de passwords probados: %ld\n", total_passwords);
    if (total_time > 0) {
        printf("Velocidad agregada: %.0f passwords/segundo\n", 
               total_passwords / total_time);
    }
    
    // Métricas de rendimiento
    double sequential_time = total_passwords / 1000000.0;
    double speedup = sequential_time > 0 ? sequential_time / total_time : 0;
    double efficiency = num_workers > 0 ? speedup / num_workers * 100 : 0;
    
    printf("\n=== MÉTRICAS DE RENDIMIENTO ===\n");
    printf("Speedup estimado: %.2fx\n", speedup);
    printf("Eficiencia: %.2f%%\n", efficiency);
    
    if (efficiency < 70) {
        printf("Baja eficiencia: considerar reducir número de procesos\n");
    } else if (efficiency > 85) {
        printf("Excelente eficiencia paralela\n");
    }
    
    free(all_stats);
}


/*
 * PROCESO WORKER (rank > 0)
 */
void worker_process(int rank) {
    WorkerStats stats;
    stats.passwords_tested = 0;
    stats.time_spent = 0.0;
    stats.hashes_cracked = 0;
    
    double start_time = MPI_Wtime();
    
    while (1) {
        // Solicitar trabajo
        int request = 1;
        MPI_Send(&request, 1, MPI_INT, 0, TAG_WORK_REQUEST, MPI_COMM_WORLD);
        
        // Esperar asignación
        MPI_Status status;
        int num_words;
        
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        if (status.MPI_TAG == TAG_TERMINATE) {
            int dummy;
            MPI_Recv(&dummy, 1, MPI_INT, 0, TAG_TERMINATE, 
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            break;
        }
        
        // Recibir cantidad de palabras
        MPI_Recv(&num_words, 1, MPI_INT, 0, TAG_WORK_ASSIGN, 
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        // Recibir cantidad de hashes
        int num_hashes;
        MPI_Recv(&num_hashes, 1, MPI_INT, 0, TAG_WORK_ASSIGN, 
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        // Alocar memoria para hashes
        char (*hashes)[MAX_HASH] = malloc(num_hashes * sizeof(char[MAX_HASH]));
        if (!hashes) {
            fprintf(stderr, "Worker %d: Error de memoria\n", rank);
            break;
        }
        
        // Recibir hashes
        for (int i = 0; i < num_hashes; i++) {
            // Recibir longitud del hash
            int hash_len;
            MPI_Recv(&hash_len, 1, MPI_INT, 0, TAG_WORK_ASSIGN, 
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // Recibir hash
            MPI_Recv(hashes[i], hash_len, MPI_CHAR, 0, TAG_WORK_ASSIGN, 
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        
        // Alocar memoria para palabras
        char (*words)[MAX_PASSWORD] = malloc(num_words * sizeof(char[MAX_PASSWORD]));
        if (!words) {
            fprintf(stderr, "Worker %d: Error de memoria\n", rank);
            free(hashes);
            break;
        }
        
        // Recibir palabras
        for (int i = 0; i < num_words; i++) {
            // Recibir longitud de la palabra
            int word_len;
            MPI_Recv(&word_len, 1, MPI_INT, 0, TAG_WORK_ASSIGN, 
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // Recibir palabra
            MPI_Recv(words[i], word_len, MPI_CHAR, 0, TAG_WORK_ASSIGN, 
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        
        // Procesar palabras y guardar resultados
        CrackResult found_results[BUFFER_SIZE];
        int num_found = 0;
        
        for (int i = 0; i < num_words; i++) {
            char hash[MAX_HASH];
            calculate_md5(words[i], hash);
            stats.passwords_tested++;
            
            // Verificar contra todos los hashes
            for (int j = 0; j < num_hashes; j++) {
                if (strcmp(hash, hashes[j]) == 0) {
                    // Encontrado! Guardar para enviar después
                    if (num_found < BUFFER_SIZE) {
                        strcpy(found_results[num_found].hash, hashes[j]);
                        strcpy(found_results[num_found].password, words[i]);
                        found_results[num_found].found = 1;
                        found_results[num_found].worker_rank = rank;
                        num_found++;
                        stats.hashes_cracked++;
                    }
                    break;
                }
            }
        }
        
        // Enviar todos los resultados encontrados DESPUÉS de procesar el lote
        for (int k = 0; k < num_found; k++) {
            // Enviar notificación y resultado
            int notification = -1;
            MPI_Send(&notification, 1, MPI_INT, 0, TAG_WORK_REQUEST, 
                     MPI_COMM_WORLD);
            
            // Enviar longitud y hash
            int hash_len = strlen(found_results[k].hash) + 1;
            MPI_Send(&hash_len, 1, MPI_INT, 0, TAG_WORK_RESULT, MPI_COMM_WORLD);
            MPI_Send(found_results[k].hash, hash_len, MPI_CHAR, 0, 
                     TAG_WORK_RESULT, MPI_COMM_WORLD);
            
            // Enviar longitud y password
            int password_len = strlen(found_results[k].password) + 1;
            MPI_Send(&password_len, 1, MPI_INT, 0, TAG_WORK_RESULT, MPI_COMM_WORLD);
            MPI_Send(found_results[k].password, password_len, MPI_CHAR, 0, 
                     TAG_WORK_RESULT, MPI_COMM_WORLD);
            
            // Enviar found y worker_rank
            MPI_Send(&found_results[k].found, 1, MPI_INT, 0, TAG_WORK_RESULT, 
                     MPI_COMM_WORLD);
            MPI_Send(&found_results[k].worker_rank, 1, MPI_INT, 0, TAG_WORK_RESULT, 
                     MPI_COMM_WORLD);
        }
        
        free(hashes);
        free(words);
    }
    
    double end_time = MPI_Wtime();
    stats.time_spent = end_time - start_time;
    
    // Enviar estadísticas
    MPI_Send(&stats, sizeof(WorkerStats), MPI_BYTE, 0, TAG_WORK_RESULT, 
             MPI_COMM_WORLD);
}


/*
 * MAIN
 */
int main(int argc, char *argv[]) {
    int rank, size;
    
    // Inicializar MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Verificar argumentos
    if (argc != 3) {
        if (rank == 0) {
            fprintf(stderr, "Uso: %s <archivo_hashes> <diccionario>\n", argv[0]);
            fprintf(stderr, "Ejemplo: mpirun -np 4 %s hashes.txt diccionario.txt\n", 
                    argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    
    // Verificar procesos mínimos
    if (size < 2) {
        if (rank == 0) {
            fprintf(stderr, "Error: se necesitan al menos 2 procesos\n");
            fprintf(stderr, "Ejecutar con: mpirun -np 4 %s ...\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    
    char *hash_file = argv[1];
    char *dict_file = argv[2];
    
    // Proceso maestro o worker
    if (rank == 0) {
        master_process(hash_file, dict_file, size - 1);
    } else {
        worker_process(rank);
    }
    
    // Finalizar MPI
    MPI_Finalize();
    return 0;
}
