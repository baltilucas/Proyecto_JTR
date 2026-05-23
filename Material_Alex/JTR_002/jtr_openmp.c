/*
 * John the Ripper - OpenMP Edition
 * Programación Paralela y Distribuida - Unidad II
 * 
 * Implementación paralela usando OpenMP para password cracking
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include "md5.h"

#define MAX_PASSWORD_LEN 100
#define MAX_HASH_LEN 33
#define PROGRESS_INTERVAL 100000

typedef struct {
    char **dictionary;
    int dict_size;
    char target_hash[MAX_HASH_LEN];
    int found;
    char result[MAX_PASSWORD_LEN];
    long long total_attempts;
    double elapsed_time;
} CrackContext;

/*
 * Cargar diccionario desde archivo
 */
int load_dictionary(const char *filename, CrackContext *ctx) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Error abriendo diccionario");
        return -1;
    }
    
    // Contar líneas
    char buffer[MAX_PASSWORD_LEN];
    int count = 0;
    while (fgets(buffer, sizeof(buffer), fp)) {
        count++;
    }
    rewind(fp);
    
    // Alocar memoria
    ctx->dictionary = (char **)malloc(count * sizeof(char *));
    if (!ctx->dictionary) {
        fclose(fp);
        return -1;
    }
    
    // Leer palabras
    ctx->dict_size = 0;
    while (fgets(buffer, sizeof(buffer), fp)) {
        // Remover newline
        buffer[strcspn(buffer, "\r\n")] = '\0';
        
        if (strlen(buffer) > 0) {
            ctx->dictionary[ctx->dict_size] = strdup(buffer);
            ctx->dict_size++;
        }
    }
    
    fclose(fp);
    return 0;
}

/*
 * Liberar memoria del diccionario
 */
void free_dictionary(CrackContext *ctx) {
    if (ctx->dictionary) {
        for (int i = 0; i < ctx->dict_size; i++) {
            free(ctx->dictionary[i]);
        }
        free(ctx->dictionary);
    }
}

/*
 * Ataque de diccionario paralelo con OpenMP
 */
void parallel_dictionary_attack(CrackContext *ctx) {
    printf("[.] Iniciando ataque paralelo...\n\n");
    
    double start_time = omp_get_wtime();
    ctx->found = 0;
    ctx->total_attempts = 0;
    
    //Noticia, si estas leyendo el codigo revisa bien que efectos
    //tiene default(none)... pista... necesitarás un puente muy alto.
    //Si no has compilado, default(none) te obliga a declarar todas las variables
    //al compilar te darás cuenta del error: not specified in enclosing ‘parallel‘
    //este tipo de situaciones es tipica, si bien un agente puede ayudarte a corregir
    //es muy importante que tengas las nociones generales de la causa del error.
    //Muchos errores al momento de trabajar con paralelismo es simplemente que no
    //se le dedico tiempo para saber que significa una opción especifica de un 
    //codigo.
    //Lo otro, siempre fijate en las librerías que se utilizan.
    //En lenguajes más automáticos, como Rust, Go o el mismo Python, el tema librería 
    //es algo que se deja de lado, puesto que tendemos a pensar que están siempre
    //disponibles, y eso es una pesima estrategia que hace perder tiempo en encontrar
    //los problemas o peor aun, generan problemas de seguridad a futuro.
    //Debes utilizar el mismo principio de minimo permiso con las librerias,
    //las estrictamente necesarias y ver que esten siendo constantemente actualizadas.
    //Este código tipico de herramienta de generación no code, tiene todos los problemas
    //planteados anteriormente, si bien puedes tomar este código y corregirlo mediante
    //las indicaciones que te acabo de describir, pero si logras resistir la tentación
    //de usar asistentes, verás que se ponen a prueba habilidades de seguridad y programacion
    //avanzadas.
    //Buena suerte.
        long long local_attempts = 0;
        int thread_id = omp_get_thread_num();
        
        #pragma omp for schedule(dynamic, 1000) nowait
        for (int i = 0; i < ctx->dict_size; i++) {
            // Early termination si otro thread encontró
            if (ctx->found) continue;
            
            // Calcular hash MD5
            md5_hash(ctx->dictionary[i], hash);
            local_attempts++;
            
            // Verificar si coincide
            if (strcmp(hash, ctx->target_hash) == 0) {
                #pragma omp critical
                {
                    // Double-check para evitar race condition
                    if (!ctx->found) {
                        ctx->found = 1;
                        strncpy(ctx->result, ctx->dictionary[i], MAX_PASSWORD_LEN - 1);
                        ctx->result[MAX_PASSWORD_LEN - 1] = '\0';
                        
                        printf("\n[+] ¡Password encontrado por thread %d!\n", thread_id);
                    }
                }
            }
            
            // Mostrar progreso (solo thread 0, cada PROGRESS_INTERVAL)
            if (thread_id == 0 && local_attempts % PROGRESS_INTERVAL == 0) {
                double progress = (i * 100.0) / ctx->dict_size;
                double current_time = omp_get_wtime() - start_time;
                double speed = i / current_time;
                
                printf("\r[.] Progreso: %6.2f%% | Velocidad: %10.0f p/s | Tiempo: %6.2f s", 
                       progress, speed, current_time);
                fflush(stdout);
            }
        }
        
        // Acumular intentos totales
        #pragma omp atomic
        ctx->total_attempts += local_attempts;
    }
    
    double end_time = omp_get_wtime();
    ctx->elapsed_time = end_time - start_time;
    
    printf("\n\n");
}

/*
 * Mostrar banner
 */
void print_banner() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     JOHN THE RIPPER - OpenMP Edition (Lovecraftiano)      ║\n");
    printf("║         Programación Paralela y Distribuida 2025          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
}

/*
 * Mostrar resultados
 */
void print_results(CrackContext *ctx) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                        RESULTADOS                          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    if (ctx->found) {
        printf("  [✓] PASSWORD ENCONTRADO: %s\n\n", ctx->result);
    } else {
        printf("  [✗] Password NO encontrado en diccionario\n\n");
    }
    
    printf("  Hash objetivo:    %s\n", ctx->target_hash);
    printf("  Diccionario:      %d palabras\n", ctx->dict_size);
    printf("  Threads usados:   %d\n", omp_get_max_threads());
    printf("  Intentos totales: %lld\n", ctx->total_attempts);
    printf("  Tiempo total:     %.3f segundos\n", ctx->elapsed_time);
    printf("  Velocidad:        %.0f passwords/segundo\n", 
           ctx->total_attempts / ctx->elapsed_time);
    
    // Calcular eficiencia
    if (ctx->found) {
        printf("\n  Eficiencia:       %.1f%%\n", 
               (ctx->total_attempts * 100.0) / ctx->dict_size);
    }
    
    printf("\n");
}

/*
 * Mostrar ayuda
 */
void print_usage(const char *program_name) {
    printf("Uso: %s <hash_md5> <diccionario>\n\n", program_name);
    printf("Argumentos:\n");
    printf("  hash_md5      Hash MD5 objetivo (32 caracteres hex)\n");
    printf("  diccionario   Archivo con palabras (una por línea)\n\n");
    printf("Variables de entorno:\n");
    printf("  OMP_NUM_THREADS  Número de threads a usar (default: todos los cores)\n\n");
    printf("Ejemplo:\n");
    printf("  export OMP_NUM_THREADS=8\n");
    printf("  %s 5f4dcc3b5aa765d61d8327deb882cf99 diccionario.txt\n\n", program_name);
}

/*
 * Validar hash MD5
 */
int validate_md5_hash(const char *hash) {
    if (strlen(hash) != 32) {
        return 0;
    }
    
    for (int i = 0; i < 32; i++) {
        char c = hash[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
            return 0;
        }
    }
    
    return 1;
}

/*
 * Main
 */
int main(int argc, char *argv[]) {
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Validar hash
    if (!validate_md5_hash(argv[1])) {
        fprintf(stderr, "[✗] Error: Hash MD5 inválido\n");
        fprintf(stderr, "    Debe ser 32 caracteres hexadecimales (0-9, a-f)\n\n");
        return 1;
    }
    
    CrackContext ctx = {0};
    strncpy(ctx.target_hash, argv[1], MAX_HASH_LEN - 1);
    
    // Mostrar banner
    print_banner();
    
    // Información del sistema
    printf("[+] Sistema:\n");
    printf("    Procesadores disponibles: %d\n", omp_get_num_procs());
    printf("    Threads configurados:     %d\n", omp_get_max_threads());
    printf("\n");
    
    // Cargar diccionario
    printf("[+] Cargando diccionario: %s\n", argv[2]);
    if (load_dictionary(argv[2], &ctx) != 0) {
        fprintf(stderr, "[✗] Error cargando diccionario\n");
        return 1;
    }
    printf("    Palabras cargadas: %d\n", ctx.dict_size);
    printf("\n");
    
    // Verificar que hay trabajo suficiente
    if (ctx.dict_size < omp_get_max_threads()) {
        printf("[!] Advertencia: Pocas palabras para el número de threads\n");
        printf("    Considere reducir OMP_NUM_THREADS\n\n");
    }
    
    // Ejecutar ataque
    parallel_dictionary_attack(&ctx);
    
    // Mostrar resultados
    print_results(&ctx);
    
    // Cleanup
    free_dictionary(&ctx);
    
    return ctx.found ? 0 : 1;
}
