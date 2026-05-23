/*
 * MD5 Hash Header
 */

#ifndef MD5_H
#define MD5_H

/*
 * Calcular hash MD5 de una cadena
 */
void md5_hash(const char *str, char output[33]);

/*
 * Validar formato de hash MD5
 */
int md5_validate(const char *hash);

#endif /* MD5_H */
