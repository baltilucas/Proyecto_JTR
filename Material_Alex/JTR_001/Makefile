CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
LIBS = -lcrypto -lssl

TARGET = jtr
SOURCES = jtr.c md5.c
HEADERS = md5.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) -g $(CFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)
	@echo "✓ Compilación exitosa"

clean:
	rm -f $(TARGET) *.o
	@echo "✓ Archivos limpiados"

test: $(TARGET)
	@echo "Probando con password 'cthulhu'..."
	./$(TARGET) 8b1a9953c4611296a827abf8c47804d7 diccionario.txt

help:
	@echo "Comandos disponibles:"
	@echo "  make          - Compilar el programa"
	@echo "  make clean    - Limpiar archivos compilados"
	@echo "  make test     - Ejecutar prueba básica"
	@echo "  make help     - Mostrar esta ayuda"

.PHONY: all clean test help
