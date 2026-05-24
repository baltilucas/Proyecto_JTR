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

Por defecto, el terraform envia el archivo comp_and_dist.sh que compila el archivo si corresponde y lo distribuye en el cluster en caso que compile o sea un .txt

para enviar archivos basta con usar `enviar_archivos.sh` donde el uso es `./enviar_archivos.sh 200.23.34.1` donde el número al lado es la ip del nodo maestro, rank 0 y que está en el output del terraform, también para conectarse via ssh, el output es inmediato.

Para correr

```
mpirun --hostfile /home/ubuntu/hostfile -np 8 ./`<file>`
```
