#!/bin/bash
echo "Selecciona el archivo a enviar (introduce el número):"
select FILE in *; do
    if [ -n "$FILE" ]; then
        echo "📂 Has seleccionado: $FILE"
        break
    else
        echo "❌ Opción inválida. Intenta de nuevo."
    fi
done

# Variable para definir qué archivo exacto vamos a copiar por SCP
FILE_TO_SEND="$FILE"

# Verifica si el archivo seleccionado termina en .c (ignora mayúsculas/minúsculas)
if [[ "$FILE" == *.c || "$FILE" == *.C ]]; then
    echo "🔨 Detectado archivo fuente de C. Compilando..."
    
    if ! mpicc -o parser "$FILE"; then
        echo "❌ Error en la compilación. Script detenido."
        exit 1
    fi
    
    # Si compila con éxito, el archivo a enviar cambia al binario generado
    FILE_TO_SEND="./parser"
fi

# Distribución hacia los nodos del cluster
echo "🚀 Enviando '$FILE_TO_SEND' a los nodos..."
for ip in $(awk '{print $1}' /home/ubuntu/hostfile); do
    scp "$FILE_TO_SEND" "ubuntu@$ip:/home/ubuntu/"
done

echo "✅ Proceso finalizado con éxito."
