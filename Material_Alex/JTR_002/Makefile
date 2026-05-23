# Makefile para John the Ripper - OpenMP Edition
# Programación Paralela y Distribuida

CC = gcc
CFLAGS = -O3 -Wall -Wextra -fopenmp
LIBS = -lcrypto -lssl
TARGET = jtr_openmp
SOURCES = jtr_openmp.c md5.c
OBJECTS = $(SOURCES:.c=.o)

# Colores para output
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[1;33m
NC = \033[0m # No Color

.PHONY: all clean test benchmark help

all: $(TARGET)
	@echo "$(GREEN)✓ Compilación exitosa$(NC)"
	@echo "  Ejecutar: ./$(TARGET) <hash> <diccionario>"
	@echo "  Ejemplo:  ./$(TARGET) 5f4dcc3b5aa765d61d8327deb882cf99 diccionario_lovecraft.txt"

$(TARGET): $(OBJECTS)
	@echo "$(YELLOW)[Linking]$(NC) $@"
	@$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
	@echo "$(YELLOW)[Compiling]$(NC) $<"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "$(RED)[Cleaning]$(NC)"
	@rm -f $(OBJECTS) $(TARGET)
	@echo "$(GREEN)✓ Limpieza completa$(NC)"

# Test con password conocido
test: $(TARGET)
	@echo "$(YELLOW)═══════════════════════════════════════════════════════════$(NC)"
	@echo "$(YELLOW)                    TEST BÁSICO                            $(NC)"
	@echo "$(YELLOW)═══════════════════════════════════════════════════════════$(NC)"
	@echo ""
	@echo "Hash objetivo: 5f4dcc3b5aa765d61d8327deb882cf99 (password='password')"
	@echo ""
	@./$(TARGET) 5f4dcc3b5aa765d61d8327deb882cf99 diccionario_lovecraft.txt

# Benchmark con diferentes números de threads
benchmark: $(TARGET)
	@echo "$(YELLOW)═══════════════════════════════════════════════════════════$(NC)"
	@echo "$(YELLOW)              BENCHMARK DE RENDIMIENTO                     $(NC)"
	@echo "$(YELLOW)═══════════════════════════════════════════════════════════$(NC)"
	@echo ""
	@echo "Midiendo rendimiento con 1, 2, 4, 8 threads..."
	@echo ""
	@for threads in 1 2 4 8; do \
		echo "$(GREEN)>>> Ejecutando con $$threads thread(s)...$(NC)"; \
		OMP_NUM_THREADS=$$threads ./$(TARGET) 8b1a9953c4611296a827abf8c47804d7 diccionario_lovecraft.txt; \
		echo ""; \
		echo "────────────────────────────────────────────────────────────"; \
		echo ""; \
	done
	@echo "$(GREEN)✓ Benchmark completado$(NC)"
	@echo ""
	@echo "Genera gráficas con: python3 plot_results.py"

# Verificar instalación de OpenMP
check-openmp:
	@echo "$(YELLOW)[Verificando OpenMP]$(NC)"
	@echo '#include <omp.h>' | $(CC) -fopenmp -x c - -o /dev/null 2>/dev/null && \
		echo "$(GREEN)✓ OpenMP está instalado$(NC)" || \
		echo "$(RED)✗ OpenMP NO está instalado$(NC)"
	@echo ""
	@echo '#include <openssl/md5.h>' | $(CC) -x c - -lcrypto -o /dev/null 2>/dev/null && \
		echo "$(GREEN)✓ OpenSSL está instalado$(NC)" || \
		echo "$(RED)✗ OpenSSL NO está instalado$(NC)"

# Información del sistema
info:
	@echo "$(YELLOW)═══════════════════════════════════════════════════════════$(NC)"
	@echo "$(YELLOW)              INFORMACIÓN DEL SISTEMA                      $(NC)"
	@echo "$(YELLOW)═══════════════════════════════════════════════════════════$(NC)"
	@echo ""
	@echo "Compilador:        $(shell $(CC) --version | head -n1)"
	@echo "Flags:             $(CFLAGS)"
	@echo "Librerías:         $(LIBS)"
	@echo "Cores disponibles: $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 'N/A')"
	@echo ""

# Ayuda
help:
	@echo "$(YELLOW)═══════════════════════════════════════════════════════════$(NC)"
	@echo "$(YELLOW)           MAKEFILE - John the Ripper OpenMP               $(NC)"
	@echo "$(YELLOW)═══════════════════════════════════════════════════════════$(NC)"
	@echo ""
	@echo "Targets disponibles:"
	@echo ""
	@echo "  $(GREEN)make$(NC)              - Compilar el programa"
	@echo "  $(GREEN)make test$(NC)         - Ejecutar test básico"
	@echo "  $(GREEN)make benchmark$(NC)    - Benchmark con diferentes threads"
	@echo "  $(GREEN)make check-openmp$(NC) - Verificar instalación de dependencias"
	@echo "  $(GREEN)make info$(NC)         - Información del sistema"
	@echo "  $(GREEN)make clean$(NC)        - Limpiar archivos compilados"
	@echo "  $(GREEN)make help$(NC)         - Mostrar esta ayuda"
	@echo ""
	@echo "Variables de entorno:"
	@echo ""
	@echo "  $(YELLOW)OMP_NUM_THREADS$(NC)   - Número de threads (default: todos los cores)"
	@echo "  $(YELLOW)OMP_PROC_BIND$(NC)     - Afinidad de procesador (true/false)"
	@echo "  $(YELLOW)OMP_SCHEDULE$(NC)      - Tipo de scheduling (static/dynamic/guided)"
	@echo ""
	@echo "Ejemplos de uso:"
	@echo ""
	@echo "  $(GREEN)OMP_NUM_THREADS=4 ./jtr_openmp <hash> <dict>$(NC)"
	@echo "  $(GREEN)OMP_PROC_BIND=true ./jtr_openmp <hash> <dict>$(NC)"
	@echo ""

# Dependencias
jtr_openmp.o: jtr_openmp.c md5.h
md5.o: md5.c md5.h
