/*
 * MD5 Hash Algorithm
 * Header file
 */

#ifndef MD5_H
#define MD5_H

#include <stdint.h>

/* MD5 context structure */
typedef struct {
    uint32_t state[4];        /* state (ABCD) */
    uint32_t count[2];        /* number of bits, modulo 2^64 (lsb first) */
    unsigned char buffer[64]; /* input buffer */
} MD5_CTX;

/* Function prototypes */
void MD5Init(MD5_CTX *);
void MD5Update(MD5_CTX *, unsigned char *, unsigned int);
void MD5Final(unsigned char [16], MD5_CTX *);

#endif /* MD5_H */
