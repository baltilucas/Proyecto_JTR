#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAX_LINE     256
#define MAX_PASSWORD 128
#define BUFFER_SIZE  5   // en vez de 100

#define TAG_WORK_ASSIGN  2
#define TAG_TERMINATE    4

/* Concatena "Pico" al final de word (modifica en-sitio, retorna word) */
static char *process_word(char *word) {
    strcat(word, "Pico");
    return word;
}

/* ── MAESTRO (rank 0) ─────────────────────────────────────────── */
static void master_process(const char *dict_file, int num_workers) {
    FILE *dict = fopen(dict_file, "r");
    if (!dict) { fprintf(stderr, "No se puede abrir: %s\n", dict_file); MPI_Abort(MPI_COMM_WORLD, 1); }

    char words[BUFFER_SIZE][MAX_PASSWORD];
    int  active = num_workers;

    /* Dar trabajo inicial a cada worker */
    for (int w = 1; w <= num_workers; w++) {
        int n = 0;
        while (n < BUFFER_SIZE && fgets(words[n], MAX_PASSWORD, dict)) {
            words[n][strcspn(words[n], "\n")] = '\0';
            n++;
        }
        MPI_Send(&n, 1, MPI_INT, w, TAG_WORK_ASSIGN, MPI_COMM_WORLD);
        for (int i = 0; i < n; i++) {
            int len = strlen(words[i]) + 1;
            MPI_Send(&len,     1,   MPI_INT,  w, TAG_WORK_ASSIGN, MPI_COMM_WORLD);
            MPI_Send(words[i], len, MPI_CHAR, w, TAG_WORK_ASSIGN, MPI_COMM_WORLD);
        }
        if (n == 0) { MPI_Send(&n, 1, MPI_INT, w, TAG_TERMINATE, MPI_COMM_WORLD); active--; }
    }

    /* Loop dinámico: cada vez que un worker pide más, le damos o terminamos */
    while (active > 0) {
        MPI_Status st;
        int req;
        MPI_Recv(&req, 1, MPI_INT, MPI_ANY_SOURCE, TAG_WORK_ASSIGN, MPI_COMM_WORLD, &st);
        int w = st.MPI_SOURCE;

        int n = 0;
        while (n < BUFFER_SIZE && fgets(words[n], MAX_PASSWORD, dict)) {
            words[n][strcspn(words[n], "\n")] = '\0';
            n++;
        }

        if (n > 0) {
            MPI_Send(&n, 1, MPI_INT, w, TAG_WORK_ASSIGN, MPI_COMM_WORLD);
            for (int i = 0; i < n; i++) {
                int len = strlen(words[i]) + 1;
                MPI_Send(&len,     1,   MPI_INT,  w, TAG_WORK_ASSIGN, MPI_COMM_WORLD);
                MPI_Send(words[i], len, MPI_CHAR, w, TAG_WORK_ASSIGN, MPI_COMM_WORLD);
            }
        } else {
            int zero = 0;
            MPI_Send(&zero, 1, MPI_INT, w, TAG_TERMINATE, MPI_COMM_WORLD);
            active--;
        }
    }

    fclose(dict);
    printf("Maestro: distribución completada.\n");
}

/* ── WORKER (rank > 0) ────────────────────────────────────────── */
static void worker_process(int rank) {
    while (1) {
        MPI_Status st;
        int n;

        /* Probe para saber si es trabajo o terminación */
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &st);

        if (st.MPI_TAG == TAG_TERMINATE) {
            MPI_Recv(&n, 1, MPI_INT, 0, TAG_TERMINATE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            break;
        }

        MPI_Recv(&n, 1, MPI_INT, 0, TAG_WORK_ASSIGN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < n; i++) {
            char word[MAX_PASSWORD + 4]; /* +4 para "Pico" */
            int  len;
            MPI_Recv(&len,  1,   MPI_INT,  0, TAG_WORK_ASSIGN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(word,  len, MPI_CHAR, 0, TAG_WORK_ASSIGN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            process_word(word);   /* aquí va tu lógica real (hash, comparar, etc.) */
            printf("Worker %d: %s\n", rank, word); 
        }

        /* Pedir más trabajo */
        int req = 1;
        MPI_Send(&req, 1, MPI_INT, 0, TAG_WORK_ASSIGN, MPI_COMM_WORLD);
    }
}

/* ── MAIN ─────────────────────────────────────────────────────── */
int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) { fprintf(stderr, "Se necesitan al menos 2 procesos.\n"); MPI_Abort(MPI_COMM_WORLD, 1); }
    if (argc < 2) { fprintf(stderr, "Uso: %s <diccionario>\n", argv[0]); MPI_Abort(MPI_COMM_WORLD, 1); }

    if (rank == 0) master_process(argv[1], size - 1);
    else           worker_process(rank);

    MPI_Finalize();
    return 0;
}