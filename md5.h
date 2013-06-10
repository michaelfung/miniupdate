/*
 * This is an OpenSSL-compatible implementation of the RSA Data Security,
 * Inc. MD5 Message-Digest Algorithm.
 *
 * Written by Solar Designer <solar at openwall.com> in 2001, and placed
 * in the public domain.  See md5.c for more information.
 */



/*

Usage Example:

MD5_CTX myctx;
MD5_CTX *ctx = &myctx;
unsigned char digest[16];
int i;

MD5_Init(ctx);
MD5_Update(ctx, argv[1], strlen(argv[1]));
MD5_Final(digest, ctx);
printf("Digest of (%s): ", argv[1]);
for (i = 0; i < 16; i++) {
	printf("%x", digest[i]);
}
printf("\n\n");

*/


#ifndef __MD5_H
#define __MD5_H

/* Any 32-bit or wider unsigned integer data type will do */
typedef unsigned long MD5_u32plus;

typedef struct {
	MD5_u32plus lo, hi;
	MD5_u32plus a, b, c, d;
	unsigned char buffer[64];
	MD5_u32plus block[16];
} MD5_CTX;

extern void MD5_Init(MD5_CTX *ctx);
extern void MD5_Update(MD5_CTX *ctx, void *data, unsigned long size);
extern void MD5_Final(unsigned char *result, MD5_CTX *ctx);

#endif
