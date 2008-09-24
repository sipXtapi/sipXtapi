//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <stddef.h>
#include <memory.h>
#include "sha1.h"

void hmac_sha1(const char*  pBlob,
               size_t nBlob,
               const char*  pKey,
               size_t nKey,
               char*        digest) 
{
    SHA1Context context ;
    char cTempKey[20] ;
    char iPad[64] ;
    char oPad[64] ;
    int i ;

    // If the key length is larger then 64 bytes, replace with hash of the key
    if (nKey > 64)
    {        
        SHA1Reset(&context) ;
        SHA1Input(&context, pKey, nKey) ;
        memset(cTempKey, 0, sizeof(cTempKey)) ;
        SHA1Result(&context, cTempKey) ;

        pKey = cTempKey ;
        nKey = 20 ;
    }

    // Initialize opad and ipad
    memset(iPad, 0, 64) ;
    memset(oPad, 0, 64) ;
    if (pKey)
    {
        memcpy(iPad, pKey, nKey) ;
        memcpy(oPad, pKey, nKey) ;
    }

    for (i=0; i<64; i++)
    {
        iPad[i] ^= 0x36 ;
        oPad[i] ^= 0x5c ;
    }


    // Step one using iPad
    SHA1Reset(&context) ;
    SHA1Input(&context, iPad, 64) ;
    SHA1Input(&context, pBlob, nBlob) ;    
    SHA1Result(&context, digest) ;

    // Step two using oPad
    SHA1Reset(&context) ;
    SHA1Input(&context, oPad, 64) ;
    SHA1Input(&context, digest, 20) ;    
    SHA1Result(&context, digest) ;
}
