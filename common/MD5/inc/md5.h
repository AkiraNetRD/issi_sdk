/*
 * CopperGate Concurrent Versions System:
 * --------------------------------------
 * $RCSfile: md5.h,v $
 * $Source: /SDK/common/signature/inc/md5.h,v $
 * $Revision: 1.2 $
 * $Date: 2009/11/11 13:08:20 $
 *
 * Copyright © 2010 CopperGate Communications Ltd.
 */

#ifndef MD5_H
#define MD5_H

#ifdef __alpha
typedef unsigned int uint32;
#else
typedef unsigned long uint32;
#endif

typedef struct
{
        uint32 buf[4];
        uint32 bits[2];
        unsigned char in[64];
} MD5Context;

void MD5Init(MD5Context *ctx);
void MD5Update(MD5Context *ctx, unsigned char *buf, unsigned len);
void MD5Final(unsigned char digest[16], MD5Context *ctx);
void MD5Transform(uint32 buf[4], uint32 in[16]);

/*
 * This is needed to make RSAREF happy on some MS-DOS compilers.
 */
typedef MD5Context MD5_CTX;

#endif /* !MD5_H */
