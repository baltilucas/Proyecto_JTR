#ifndef MD5_H
#define MD5_H

#include <stdint.h>
#include <string.h>

/**
 * Genera el hash MD5 de una cadena
 * @param str Cadena de entrada
 * @param output Buffer de salida (debe ser de al menos 33 caracteres)
 */
void md5_hash(const char *str, char output[33]);

#endif /* MD5_H */
