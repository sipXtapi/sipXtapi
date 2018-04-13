/*
 * sha1.h (Obtained from rfc 4634)
 *
 * Copyright (c) The Internet Society (2001). All Rights Reserved.
 *
 * License from RFC 4634 s1.1:
 *
 * Permission is granted for all uses, commercial and non-commercial, of
 * the sample code found in Section 8.  Royalty free license to use,
 * copy, modify and distribute the software found in Section 8 is
 * granted, provided that this document is identified in all material
 * mentioning or referencing this software, and provided that
 * redistributed derivative works do not contain misleading author or
 * version information.
 *
 * The authors make no representations concerning either the
 * merchantability of this software or the suitability of this software
 * for any particular purpose.  It is provided "as is" without express
 * or implied warranty of any kind.
 *
 *  Description:
 *      This is the header file for code which implements the Secure
 *      Hashing Algorithm 1 as defined in FIPS PUB 180-1 published
 *      April 17, 1995.
 *
 *      Many of the variable names in this code, especially the
 *      single character names, were used because those were the names
 *      used in the publication.
 *
 *      Please read the file sha1.c for more information.
 */

#ifndef _SHA1_H_
#define _SHA1_H_

/*
 * If you do not have the ISO standard stdint.h header file, then you
 * must typdef the following:
 *    name              meaning
 *  uint32_t         unsigned 32 bit integer
 *  uint8_t          unsigned 8 bit integer (i.e., unsigned char)
 *  int_least16_t    integer of >= 16 bits
 *
 */
#ifdef WIN32
typedef unsigned __int32 uint32_t ;
typedef unsigned char    uint8_t ;
typedef signed __int16   int_least16_t ;
#else
#include <stdint.h>
#endif

#ifndef _SHA_enum_
#define _SHA_enum_
enum
{
    shaSuccess = 0,
    shaNull,            /* Null pointer parameter */
    shaInputTooLong,    /* input data too long */
    shaStateError       /* called Input after Result */
}; 
#endif
#define SHA1HashSize 20

/*
 *  This structure will hold context information for the SHA-1
 *  hashing operation
 */
typedef struct SHA1Context
{
    uint32_t Intermediate_Hash[SHA1HashSize/4]; /* Message Digest  */

    uint32_t Length_Low;            /* Message length in bits      */
    uint32_t Length_High;           /* Message length in bits      */

    /* Index into message block array   */
    int_least16_t Message_Block_Index;
    uint8_t Message_Block[64];      /* 512-bit message blocks      */

    int Computed;               /* Is the digest computed?         */
    int Corrupted;             /* Is the message digest corrupted? */
} SHA1Context; 

/*
 *  Function Prototypes
 */

int SHA1Reset( SHA1Context *);
int SHA1Input( SHA1Context *, const uint8_t *, unsigned int); 
int SHA1Result( SHA1Context *, uint8_t Message_Digest[SHA1HashSize]); 

#endif

