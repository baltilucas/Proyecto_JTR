# Comandos John the reaper

## Terraform

Para utilizar el terrafrom que crea el cluster se necesita crear una clave ssh, los comandos son:

```
ssh-keygen -t rsa -b 4096 -f ./cluster_key -N ""
```

Por defecto crea 8 instancias, si quiere modificarse usar la variable

```
terraform apply -var="noi=4"
```

para enviar un archivo, cómo el JTR a una instancia de ip `<ip>`

```
scp -i "lab_paralela.pem" /parseo_texto_local.c ubuntu@ec2-<ip>compute-1.amazonaws.com:~/
```

Para enviar carpeta completa se añade el `-r` 

```
scp -i "lab_paralela.pem" -r /ruta/de/tu/carpeta ubuntu@ec2-<ip>.compute-1.amazonaws.com:~/
```

para compilar el archivo se usa

```
mpicc -o <nombre_salida> ejemplo.c
```

y para enviar el binario compilado a todos se usa (cambiar `<archivo>`

```
for ip in $(awk '{print $1}' /home/ubuntu/hostfile); do
    scp ./<archivo> ubuntu@$ip:/home/ubuntu/<archivo>
done
```
