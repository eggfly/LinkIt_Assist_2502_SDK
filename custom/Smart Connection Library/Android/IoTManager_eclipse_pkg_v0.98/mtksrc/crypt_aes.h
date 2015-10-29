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
    AES

    Abstract:
    RFC 3394: Advanced Encryption Standard (AES) Key Wrap Algorithm    
    RFC 3601: Counter with CBC-MAC (CCM)
    RFC 4493: The AES-CMAC Algorithm
    FIPS PUBS 197: ADVANCED ENCRYPTION STANDARD (AES)
    NIST 800-38A: Recommendation for Block Cipher Modes of Operation
    NIST 800-38C: The CCM Mode for Authentication and Confidentiality 
    
    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
    Eddy        2009/05/19      Create AES-Key Wrap
    Eddy        2009/04/20      Create AES-CMAC, AES-CCM    
    Eddy        2009/01/19      Create AES-128, AES-192, AES-256, AES-CBC
***************************************************************************/
#ifndef __CRYPT_AES_H__
#define __CRYPT_AES_H__
#include "types.h"

/* AES definition & structure */
#define AES_STATE_ROWS 4     /* Block size: 4*4*8 = 128 bits */
#define AES_STATE_COLUMNS 4
#define AES_BLOCK_SIZES (AES_STATE_ROWS*AES_STATE_COLUMNS)
#define AES_KEY_ROWS 4
#define AES_KEY_COLUMNS 8    /*Key length: 4*{4,6,8}*8 = 128, 192, 256 bits */
#define AES_KEY128_LENGTH 16
#define AES_KEY192_LENGTH 24
#define AES_KEY256_LENGTH 32
#define AES_CBC_IV_LENGTH 16

typedef struct {
    UINT8 State[AES_STATE_ROWS][AES_STATE_COLUMNS];
    UINT8 KeyWordExpansion[AES_KEY_ROWS][AES_KEY_ROWS*((AES_KEY256_LENGTH >> 2) + 6 + 1)];
} AES_CTX_STRUC, *PAES_CTX_STRUC;

/* AES operations */
VOID RT_AES_KeyExpansion(
    UINT8 *Key,
    UINT KeyLength,
    AES_CTX_STRUC *paes_ctx);

VOID RT_AES_Encrypt (
    UINT8 *PlainBlock,
    UINT PlainBlockSize,
    UINT8 *Key,
    UINT KeyLength,
    UINT8 *CipherBlock,
    UINT *CipherBlockSize);

VOID RT_AES_Decrypt (
    UINT8 *CipherBlock,
    UINT CipherBlockSize,
    UINT8 *Key,
    UINT KeyLength,
    UINT8 *PlainBlock,
    UINT *PlainBlockSize);

#if 0
/* AES Counter with CBC-MAC operations */
VOID AES_CCM_MAC (
    UINT8 Payload[],
    UINT  PayloadLength,
    UINT8 Key[],
    UINT  KeyLength,
    UINT8 Nonce[],
    UINT  NonceLength,
    UINT8 AAD[],
    UINT  AADLength,
    UINT  MACLength,
    UINT8 MACText[]);

INT AES_CCM_Encrypt (
    UINT8 PlainText[],
    UINT PlainTextLength,
    UINT8 Key[],
    UINT KeyLength,
    UINT8 Nonce[],
    UINT  NonceLength,
    UINT8 AAD[],
    UINT  AADLength,
    UINT  MACLength,
    UINT8 CipherText[],
    UINT *CipherTextLength);

INT AES_CCM_Decrypt (
    UINT8 CipherText[],
    UINT CipherTextLength,
    UINT8 Key[],
    UINT KeyLength,
    UINT8 Nonce[],
    UINT  NonceLength,
    UINT8 AAD[],
    UINT  AADLength,
    UINT  MACLength,
    UINT8 PlainText[],
    UINT *PlainTextLength);

/* AES-CMAC operations */
VOID AES_CMAC_GenerateSubKey (
    UINT8 Key[],
    UINT KeyLength,
    UINT8 SubKey1[],
    UINT8 SubKey2[]);

VOID AES_CMAC (
    UINT8 PlainText[],
    UINT PlainTextLength,
    UINT8 Key[],
    UINT KeyLength,
    UINT8 MACText[],
    UINT *MACTextLength);

/* AES-CBC operations */
VOID AES_CBC_Encrypt (
    UINT8 PlainText[],
    UINT  PlainTextLength,
    UINT8 Key[],
    UINT  KeyLength,
    UINT8 IV[],
    UINT IVLength,
    UINT8 CipherText[],
    UINT *CipherTextLength);

VOID AES_CBC_Decrypt (
    UINT8 CipherText[],
    UINT  CipherTextLength,
    UINT8 Key[],
    UINT  KeyLength,
    UINT8 IV[],
    UINT IVLength,
    UINT8 PlainText[],
    UINT *PlainTextLength);

/* AES key wrap operations */
INT AES_Key_Wrap (
    UINT8 PlainText[],
    UINT  PlainTextLength,
    UINT8 Key[],
    UINT  KeyLength,
    UINT8 CipherText[],
    UINT *CipherTextLength);
        
INT AES_Key_Unwrap (
    UINT8 CipherText[],
    UINT  CipherTextLength,
    UINT8 Key[],
    UINT  KeyLength,
    UINT8 PlainText [],
    UINT *PlainTextLength);
#endif
#endif /* __CRYPT_AES_H__ */

