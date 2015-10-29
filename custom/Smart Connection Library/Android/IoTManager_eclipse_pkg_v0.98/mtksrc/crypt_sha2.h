/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************
    Module Name:
    SHA2

    Abstract:
    FIPS 180-2: Secure Hash Standard (SHS)
    
    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
    Eddy        2008/11/24      Create SHA1
    Eddy        2008/07/23      Create SHA256
***************************************************************************/

#ifndef __CRYPT_SHA2_H__
#define __CRYPT_SHA2_H__
#include "types.h"

/* Algorithm options */
#define SHA1_SUPPORT
//#define SHA256_SUPPORT

#define cpu2be64(x) SWAP64((x))

#define cpu2be32(x) SWAP32((x))
//#define cpu2be64(x) ((UINT64)(x))
//#define cpu2be32(x) ((UINT32)(x))


#define SWAP32(x) \
    ((UINT32)( \
    (((UINT32)(x) & (UINT32) 0x000000ffUL) << 24) | \
    (((UINT32)(x) & (UINT32) 0x0000ff00UL) <<  8) | \
    (((UINT32)(x) & (UINT32) 0x00ff0000UL) >>  8) | \
    (((UINT32)(x) & (UINT32) 0xff000000UL) >> 24) ))

#define SWAP64(x) \
    ((UINT64)( \
    (UINT64)(((UINT64)(x) & (UINT64) 0x00000000000000ffULL) << 56) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0x000000000000ff00ULL) << 40) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0x0000000000ff0000ULL) << 24) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0x00000000ff000000ULL) <<  8) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0x000000ff00000000ULL) >>  8) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0x0000ff0000000000ULL) >> 24) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0x00ff000000000000ULL) >> 40) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0xff00000000000000ULL) >> 56) ))


#ifdef SHA1_SUPPORT
#define SHA1_BLOCK_SIZE    64 /* 512 bits = 64 bytes */
#define SHA1_DIGEST_SIZE   20 /* 160 bits = 20 bytes */
typedef struct _SHA1_CTX_STRUC {
    UINT32 HashValue[5];  /* 5 = (SHA1_DIGEST_SIZE / 32) */
    UINT64 MessageLen;    /* total size */
    UINT8  Block[SHA1_BLOCK_SIZE];
    UINT   BlockLen;
} SHA1_CTX_STRUC, *PSHA1_CTX_STRUC;

VOID RT_SHA1_Init (
    SHA1_CTX_STRUC *pSHA_CTX);
VOID RT_SHA1_Hash (
    SHA1_CTX_STRUC *pSHA_CTX);
VOID RT_SHA1_Append (
    SHA1_CTX_STRUC *pSHA_CTX, 
    const UINT8 *Message, 
    UINT MessageLen);
VOID RT_SHA1_End (
    SHA1_CTX_STRUC *pSHA_CTX, 
    UINT8 *DigestMessage);
VOID RT_SHA1 (
    const UINT8 *Message, 
    UINT MessageLen, 
    UINT8 *DigestMessage);
#endif /* SHA1_SUPPORT */

#if 0
#ifdef SHA256_SUPPORT
#define SHA256_BLOCK_SIZE   64 /* 512 bits = 64 bytes */
#define SHA256_DIGEST_SIZE  32 /* 256 bits = 32 bytes */
typedef struct _SHA256_CTX_STRUC {
    UINT32 HashValue[8];  /* 8 = (SHA256_DIGEST_SIZE / 32) */
    UINT64 MessageLen;    /* total size */
    UINT8  Block[SHA256_BLOCK_SIZE];
    UINT   BlockLen;
} SHA256_CTX_STRUC, *PSHA256_CTX_STRUC;

VOID RT_SHA256_Init (
    SHA256_CTX_STRUC *pSHA_CTX);
VOID RT_SHA256_Hash (
    SHA256_CTX_STRUC *pSHA_CTX);
VOID RT_SHA256_Append (
    SHA256_CTX_STRUC *pSHA_CTX, 
    const UINT8 Message[], 
    UINT MessageLen);
VOID RT_SHA256_End (
    SHA256_CTX_STRUC *pSHA_CTX, 
    UINT8 DigestMessage[]);
VOID RT_SHA256 (
    const UINT8 Message[], 
    UINT MessageLen,
    UINT8 DigestMessage[]);
#endif /* SHA256_SUPPORT */
#endif

#endif /* __CRYPT_SHA2_H__ */

