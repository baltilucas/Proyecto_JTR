#include "md5.h"
#include <openssl/md5.h>
#include <stdio.h>

void md5_hash(const char *str, char output[33]) {
    unsigned char digest[16];
    MD5_CTX context;
    
    MD5_Init(&context);
    MD5_Update(&context, str, strlen(str));
    MD5_Final(digest, &context);
    
    // Convertir a hexadecimal
    for (int i = 0; i < 16; i++) {
        sprintf(&output[i*2], "%02x", (unsigned int)digest[i]);
    }
    output[32] = '\0';
}
