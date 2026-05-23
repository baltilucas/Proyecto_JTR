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
