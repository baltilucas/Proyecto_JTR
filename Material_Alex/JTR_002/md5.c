/*
 * MD5 Hash Implementation
 * Wrapper around OpenSSL MD5 for password cracking
 */

#include "md5.h"
#include <openssl/md5.h>
#include <string.h>
#include <stdio.h>

/*
 * Calcular hash MD5 de una cadena
 * 
 * @param str: Cadena de entrada
 * @param output: Buffer de salida (mínimo 33 bytes para null terminator)
 */
void md5_hash(const char *str, char output[33]) {
    unsigned char digest[16];
    
    // Calcular MD5
    MD5((unsigned char*)str, strlen(str), digest);
    
    // Convertir a hexadecimal
    for (int i = 0; i < 16; i++) {
        sprintf(&output[i * 2], "%02x", (unsigned int)digest[i]);
    }
    
    output[32] = '\0';
}

/*
 * Verificar si un hash MD5 es válido
 * 
 * @param hash: Hash MD5 en formato hexadecimal
 * @return: 1 si válido, 0 si no
 */
int md5_validate(const char *hash) {
    if (strlen(hash) != 32) {
        return 0;
    }
    
    for (int i = 0; i < 32; i++) {
        char c = hash[i];
        if (!((c >= '0' && c <= '9') || 
              (c >= 'a' && c <= 'f') || 
              (c >= 'A' && c <= 'F'))) {
            return 0;
        }
    }
    
    return 1;
}
