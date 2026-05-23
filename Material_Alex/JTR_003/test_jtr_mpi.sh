#!/bin/bash

echo "=== GENERANDO ARCHIVOS DE PRUEBA ==="

# Crear hashes de prueba
cat > hashes_test.txt << 'HASHES'
5f4dcc3b5aa765d61d8327deb882cf99
098f6bcd4621d373cade4e832627b4f6
25d55ad283aa400af464c76d713c07ad
e10adc3949ba59abbe56e057f20f883e
25f9e794323b453885f5181f1b624d0b
HASHES

# Crear diccionario pequeño
cat > diccionario_test.txt << 'DICT'
admin
administrator
password
test
testing
12345
123456
123456789
qwerty
abc123
letmein
welcome
monkey
dragon
master
DICT

echo "✓ Archivos creados"
echo "  - hashes_test.txt (5 hashes)"
echo "  - diccionario_test.txt (15 palabras)"
echo ""
echo "=== COMPILANDO ==="
echo "mpicc -o jtr_mpi jtr_mpi.c md5.c -O3 -lm"
echo ""
echo "=== EJECUTAR ==="
echo "mpirun -np 4 ./jtr_mpi hashes_test.txt diccionario_test.txt"
