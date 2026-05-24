#!/bin/bash

# 1. Validación e IP con guiones
DIRECCION_INSTANCIA=$1
if [ -z "$DIRECCION_INSTANCIA" ]; then
    echo "❌ Error: Debes pasar la IP como parámetro. Ejemplo: ./script.sh 200.23.34.1"
    exit 1
fi
IP_CON_GUIONES=${DIRECCION_INSTANCIA//./-}

# 2. Selección interactiva de archivos de la ruta actual
echo "Selecciona el archivo a enviar (introduce el número):"
select FILE in *; do
    if [ -n "$FILE" ]; then
        echo "📂 Has seleccionado: $FILE"
        break
    else
        echo "❌ Opción inválida. Intenta de nuevo."
    fi
done

# 3. Envío y Verificación del éxito de scp
echo "Enviando archivo..."
scp -i "lab_paralela.pem" "$FILE" ubuntu@ec2-"$IP_CON_GUIONES".compute-1.amazonaws.com:~/

# $? captura el código de salida del comando anterior (0 significa éxito)
if [ $? -eq 0 ]; then
    echo "✅ ¡Éxito! El archivo '$FILE' se envió correctamente."
else
    echo "❌ Error: El envío falló. Revisa tu conexión o la clave PEM."
fi
