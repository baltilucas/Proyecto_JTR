terraform {
  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "~> 5.0"
    }
  }
}

provider "aws" {
  region = "us-east-1"
}

variable "noi" {
  type        = number
  description = "Cantidad de instancias a crear para el cluster MPI"
  default     = 8
  sensitive   = false  
}

resource "aws_instance" "mpi_cluster" {
  count         = var.noi
  ami           = "ami-04b70fa74e45c3917"
  instance_type = "t3.micro"
  key_name      = "lab_paralela"
  
  vpc_security_group_ids = [aws_security_group.allow_ssh.id]

  user_data = <<-EOF
              #!/bin/bash
              apt-get update -y
              DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential openmpi-bin libopenmpi-dev openssh-server

              mkdir -p /home/ubuntu/.ssh
              chmod 700 /home/ubuntu/.ssh

              echo "${file("./cluster_key.pub")}" >> /home/ubuntu/.ssh/authorized_keys
              
              echo "${file("./cluster_key")}" > /home/ubuntu/.ssh/id_rsa

              echo "${file("./comp_and_dist.sh")}" > /home/ubuntu/

              chmod +x comp_and_dist.sh

              chmod 600 /home/ubuntu/.ssh/id_rsa

              echo "Host *" > /home/ubuntu/.ssh/config
              echo "    StrictHostKeyChecking no" >> /home/ubuntu/.ssh/config
              
              chown -R ubuntu:ubuntu /home/ubuntu/.ssh
              EOF

  provisioner "file" {
    source      = "./diccionario.txt"
    destination = "/home/ubuntu/diccionario.txt"

    # Terraform necesita conectarse por SSH para subirlo, usamos la clave privada local
    connection {
      type        = "ssh"
      user        = "ubuntu"
      private_key = file("./cluster_key")
      host        = self.public_ip
    }
  }

  provisioner "file" {
    source      = "./comp_and_dist.sh"
    destination = "/home/ubuntu/comp_and_dist.sh"
       connection {
      type        = "ssh"
      user        = "ubuntu"
      private_key = file("./cluster_key")
      host        = self.public_ip
    }
  }

  provisioner "remote-exec" {
    inline = [
      "chmod +x /home/ubuntu/comp_and_dist.sh",
    ]
       connection {
      type        = "ssh"
      user        = "ubuntu"
      private_key = file("./cluster_key")
      host        = self.public_ip
    }
  }



  tags = {
    Name = "MPI-Node-${count.index}"
  }
}

resource "aws_security_group" "allow_ssh" {
  name        = "ssh_JTR"
  description = "Allow SSH inbound traffic"

  ingress {
    from_port   = 22
    to_port     = 22
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  egress {
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }

  ingress {
    description = "Allow all internal cluster traffic"
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    self        = true 
  }
}

output "hostfile_content" {
  value = join("\n", [for ip in aws_instance.mpi_cluster[*].private_ip : "${ip} slots=1"])
}

resource "null_resource" "generate_hostfile" {
  depends_on = [aws_instance.mpi_cluster]

  count = length(aws_instance.mpi_cluster)

  provisioner "remote-exec" {
    connection {
      type        = "ssh"
      user        = "ubuntu"
      private_key = file("./cluster_key") # Uses your local private key
      host        = aws_instance.mpi_cluster[count.index].public_ip
    }

    inline = [
      "echo '${join("\n", [for ip in aws_instance.mpi_cluster[*].private_ip : "${ip} slots=1"])}' > /home/ubuntu/hostfile",
      "chmod 644 /home/ubuntu/hostfile"
    ]
  }
}

output "master_node_public_ip" {
  description = "Dirección IP pública del mpi-cluster-0 (Nodo Maestro)"
  value       = aws_instance.mpi_cluster[0].public_ip
}

output "ssh_connect_master" {
  description = "Comando para conectarte directo al nodo maestro"
  value       = "ssh -i \"lab_paralela.pem\" ubuntu@${aws_instance.mpi_cluster[0].public_ip}"
}
