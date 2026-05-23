# John the Ripper - OpenMP Edition

Implementación paralela de John the Ripper usando OpenMP para la **Unidad II: Programación Paralela en Memoria Compartida**.

## 📋 Descripción

Este proyecto implementa un password cracker educativo que utiliza OpenMP para paralelizar el ataque de diccionario a hashes MD5. Es parte del proyecto trimestral del curso de Programación Paralela y Distribuida.

## 🎯 Objetivos de Aprendizaje

- Comprender el modelo Fork-Join de OpenMP
- Implementar paralelización de loops con `#pragma omp for`
- Gestionar variables compartidas y privadas correctamente
- Implementar sincronización con critical sections y atomics
- Medir y analizar speedup y eficiencia
- Optimizar rendimiento en memoria compartida

## 🛠️ Requisitos

### Software

- GCC 7.0+ con soporte OpenMP (`-fopenmp`)
- OpenSSL development libraries
- Make
- Sistema operativo: Linux, macOS, o WSL en Windows

### Instalación de Dependencias

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential libssl-dev
```

**macOS (Homebrew):**
```bash
brew install gcc openssl
```

**Verificar instalación:**
```bash
make check-openmp
```

## 📦 Estructura del Proyecto

```
unidad2_openmp/
├── jtr_openmp.c              # Programa principal
├── md5.c                     # Implementación MD5
├── md5.h                     # Header MD5
├── Makefile                  # Build system
├── diccionario_lovecraft.txt # Diccionario de passwords
├── README.md                 # Este archivo
└── Laboratorio2_Guia.md      # Guía del laboratorio
```

## 🚀 Compilación y Ejecución

### Compilar

```bash
make
```

### Ejecución Básica

```bash
./jtr_openmp <hash_md5> <diccionario>
```

**Ejemplo:**
```bash
./jtr_openmp 5f4dcc3b5aa765d61d8327deb882cf99 diccionario_lovecraft.txt
```

### Controlar Número de Threads

```bash
# Usar 4 threads
export OMP_NUM_THREADS=4
./jtr_openmp <hash> <diccionario>

# O directamente
OMP_NUM_THREADS=8 ./jtr_openmp <hash> <diccionario>
```

### Test Automático

```bash
make test
```

### Benchmark

```bash
make benchmark
```

Esto ejecutará el programa con 1, 2, 4 y 8 threads para comparar rendimiento.

## 📊 Ejemplos de Uso

### Ejemplo 1: Password Simple

```bash
# Hash MD5 de "password"
./jtr_openmp 5f4dcc3b5aa765d61d8327deb882cf99 diccionario_lovecraft.txt
```

**Output esperado:**
```
╔════════════════════════════════════════════════════════════╗
║     JOHN THE RIPPER - OpenMP Edition (Lovecraftiano)      ║
║         Programación Paralela y Distribuida 2025          ║
╚════════════════════════════════════════════════════════════╝

[+] Sistema:
    Procesadores disponibles: 8
    Threads configurados:     8

[+] Cargando diccionario: diccionario_lovecraft.txt
    Palabras cargadas: 112

[.] Iniciando ataque paralelo...

[.] Progreso:  50.00% | Velocidad:   150000 p/s | Tiempo:   0.37 s

[+] ¡Password encontrado por thread 3!

╔════════════════════════════════════════════════════════════╗
║                        RESULTADOS                          ║
╚════════════════════════════════════════════════════════════╝

  [✓] PASSWORD ENCONTRADO: password

  Hash objetivo:    5f4dcc3b5aa765d61d8327deb882cf99
  Diccionario:      112 palabras
  Threads usados:   8
  Intentos totales: 56
  Tiempo total:     0.001 segundos
  Velocidad:        56000 passwords/segundo

  Eficiencia:       50.0%
```

### Ejemplo 2: Password Lovecraftiano

```bash
# Hash MD5 de "cthulhu"
./jtr_openmp 8b1a9953c4611296a827abf8c47804d7 diccionario_lovecraft.txt
```

### Ejemplo 3: Benchmark de Rendimiento

```bash
# Comparar 1 vs 8 threads
echo "=== 1 Thread ==="
OMP_NUM_THREADS=1 ./jtr_openmp 8b1a9953c4611296a827abf8c47804d7 diccionario_lovecraft.txt

echo ""
echo "=== 8 Threads ==="
OMP_NUM_THREADS=8 ./jtr_openmp 8b1a9953c4611296a827abf8c47804d7 diccionario_lovecraft.txt
```

## 🔍 Hashes de Ejemplo para Testing

| Password | Hash MD5 |
|----------|----------|
| password | `5f4dcc3b5aa765d61d8327deb882cf99` |
| cthulhu | `8b1a9953c4611296a827abf8c47804d7` |
| azathoth | `0cc175b9c0f1b6a831c399e269772661` |
| lovecraft | `18126e7bd3f84b3f3e4df094def5b7de` |
| necronomicon | `1803e453f438ff7c5c4ea7e1e3d44e5e` |

## 📈 Análisis de Rendimiento

### Métricas Importantes

1. **Speedup:** S(p) = T(1) / T(p)
2. **Eficiencia:** E(p) = S(p) / p × 100%
3. **Velocidad:** Passwords/segundo

### Generar Datos para Análisis

```bash
# Crear archivo de resultados
echo "Threads,Tiempo(s),Velocidad(p/s)" > resultados.csv

for threads in 1 2 4 8 16; do
    echo "Testeando con $threads threads..."
    output=$(OMP_NUM_THREADS=$threads ./jtr_openmp <hash> <dict> | grep "Tiempo total")
    # Parsear y agregar a CSV
done
```

## 🎓 Conceptos de OpenMP Implementados

### 1. Región Paralela

```c
#pragma omp parallel default(none) shared(ctx)
{
    // Código ejecutado por todos los threads
}
```

### 2. Paralelización de Loop

```c
#pragma omp for schedule(dynamic, 1000) nowait
for (int i = 0; i < ctx->dict_size; i++) {
    // Iteraciones distribuidas dinámicamente
}
```

### 3. Variables Compartidas y Privadas

```c
// Compartida: ctx (todos los threads la ven)
// Privada: hash, local_attempts (cada thread su copia)
```

### 4. Sincronización con Critical

```c
#pragma omp critical
{
    // Solo un thread a la vez
    if (!ctx->found) {
        ctx->found = 1;
        strcpy(ctx->result, password);
    }
}
```

### 5. Operaciones Atómicas

```c
#pragma omp atomic
ctx->total_attempts += local_attempts;
```

### 6. Early Termination

```c
if (ctx->found) continue;  // Terminar si otro thread encontró
```

## 🐛 Debugging y Troubleshooting

### Problema: Compilación Falla

**Error:** `fatal error: omp.h: No such file or directory`

**Solución:**
```bash
# Verificar que GCC tenga OpenMP
gcc --version
gcc -fopenmp --version

# En macOS, usar GCC de Homebrew en lugar de Apple Clang
export CC=/usr/local/bin/gcc-13
make clean && make
```

### Problema: Error de Linking con OpenSSL

**Error:** `undefined reference to MD5`

**Solución:**
```bash
# Instalar librerías de desarrollo
sudo apt-get install libssl-dev

# Verificar que está instalado
pkg-config --libs openssl
```

### Problema: Rendimiento Bajo

**Síntomas:** Speedup < 2 con 8 threads

**Diagnóstico:**
1. Verificar que hay suficientes palabras en diccionario
2. Aumentar chunk size en schedule(dynamic)
3. Verificar false sharing
4. Usar `OMP_PROC_BIND=true` para afinidad

## 📚 Recursos Adicionales

### Documentación

- [OpenMP Official](https://www.openmp.org/)
- [GCC OpenMP](https://gcc.gnu.org/onlinedocs/libgomp/)
- [OpenMP Tutorial](https://computing.llnl.gov/tutorials/openMP/)

### Profiling

```bash
# Con gprof
gcc -pg -fopenmp jtr_openmp.c md5.c -o jtr_openmp -lcrypto
./jtr_openmp <hash> <dict>
gprof jtr_openmp gmon.out > analysis.txt

# Con perf (Linux)
perf record -g ./jtr_openmp <hash> <dict>
perf report
```

## 📝 Entregables del Laboratorio

1. **Código Fuente:**
   - jtr_openmp.c modificado y optimizado
   - Comentarios explicando decisiones de diseño

2. **Informe de Rendimiento:**
   - Gráfica speedup vs número de threads
   - Gráfica eficiencia vs número de threads
   - Análisis de resultados

3. **Optimizaciones:**
   - Identificar cuellos de botella
   - Implementar al menos 2 optimizaciones
   - Documentar mejoras obtenidas

4. **Comparación:**
   - Versión secuencial vs OpenMP
   - Diferentes scheduling strategies
   - Impacto del tamaño del chunk

## 🏆 Criterios de Evaluación

| Criterio | Puntos | Descripción |
|----------|--------|-------------|
| **Corrección** | 30 | Código compila y funciona correctamente |
| **Rendimiento** | 30 | Speedup > 6x con 8 threads |
| **Análisis** | 25 | Gráficas y análisis de resultados |
| **Documentación** | 15 | Comentarios y README |
| **TOTAL** | 100 | |

**Bonus (+10):**
- Implementar scheduling adaptativo
- Añadir soporte para SHA-256
- Implementar reglas de mutación paralelas

## 🔗 Próximos Pasos

Después de completar esta unidad:
- **Unidad III:** MPI - Versión distribuida en cluster
- **Unidad IV:** CUDA - Aceleración con GPU

## 👥 Autor

Material del curso **Programación Paralela y Distribuida 2025**

## 📄 Licencia

Este proyecto es material educativo para uso académico.

---

**¡Buena suerte con la paralelización!** 🚀
