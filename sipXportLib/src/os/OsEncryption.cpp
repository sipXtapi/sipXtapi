//
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2006-2017 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include "os/OsIntTypes.h"
#include <assert.h>
#include <ctype.h>
#include <time.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#include <netdb.h>
#endif

#ifdef HAVE_SSL
#define WIN32_LEAN_AND_MEAN
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#ifdef WIN32
#  if OPENSSL_VERSION_NUMBER >= 0x10100000L
      // OpenSSL 1.1.x naming convention (usually matches 3.x)
#     pragma comment (lib, "libcrypto.lib")
#     pragma comment (lib, "libssl.lib")
#  else
      // Legacy OpenSSL 1.0.x and older
#     pragma comment (lib, "libeay32.lib")
#     pragma comment (lib, "ssleay32.lib")
#  endif
#endif // WIN32
#endif // HAVE_SSL

#ifdef _VXWORKS
#include <resolvLib.h>
#endif

#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif

// APPLICATION INCLUDES
#include "os/OsExcept.h"
#include "os/OsNameDb.h"
#include "os/OsEncryption.h"
#include "os/OsSocket.h"
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS
static const char gSalt[] =
{
    (char)0xc9, (char)0x36, (char)0x78, (char)0x99,
    (char)0x52, (char)0x3e, (char)0xea, (char)0xf2
};

// STATIC VARIABLE INITIALIZATIONS
UtlBoolean OsEncryption::sIgnoreEncryption = FALSE;

// Wrapper around encryption, currently OpenSSL, specifically DES w/MD5
// See:
//   http://www.catfive.org/cgi-bin/man2web?program=EVP_CipherInit&section=3
// For OpenSLL API information


/* //////////////////////////// PUBLIC //////////////////////////////////// */
OsEncryption::OsEncryption(void)
{
    mSalt = (unsigned char *)gSalt;
    mSaltLen = sizeof(gSalt);
    memset(&mKey, 0, sizeof(mKey));
    mKeyLen = 0;
    mData = NULL;
    mDataLen = 0;
    mResults = NULL;
    mResultsLen = 0;
    memset(&mHeader, 0, sizeof(mHeader));
    mHeaderLen = 0;
#if defined (OSENCRYPTION)
    mAlgorithm = NULL;
    mContext = NULL;
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    mContext = EVP_CIPHER_CTX_new();
#else
    mContext = (EVP_CIPHER_CTX*)malloc(sizeof(EVP_CIPHER_CTX));
    if (mContext) EVP_CIPHER_CTX_init(mpContext);
#endif
    memset(&mContext, 0, sizeof(mContext));
#endif
}


OsEncryption::~OsEncryption(void)
{
    release(); // open encryption algorithms
#if defined (OSENCRYPTION)
    if (mContext)
    {
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
       EVP_CIPHER_CTX_free(mContext);
#else
       EVP_CIPHER_CTX_cleanup(mContext);
       free(mContext);
#endif
       mContext = NULL;
    }
#endif
}


void OsEncryption::setKey(const unsigned char* key, int keyLen)
{
    if (key != NULL)
    {
        mKeyLen = (OE_MAX_KEY_LEN < keyLen ? OE_MAX_KEY_LEN : keyLen);
        memcpy(mKey, key, mKeyLen);
    }
}

void OsEncryption::setResultsHeader(const unsigned char* header, int headerLen)
{
    mHeaderLen = (OE_MAX_RESULTS_HEADER_LEN < headerLen ? OE_MAX_RESULTS_HEADER_LEN : headerLen);
    memcpy(mHeader, header, mHeaderLen);
}

// Does not make a copy
void OsEncryption::setDataPointer(unsigned char* data, int dataLen)
{
    mData = data;
    mDataLen = dataLen;
}

int OsEncryption::getDataLen(void)
{
    return mDataLen;
}

unsigned char *OsEncryption::getDataPointer(void)
{
    return mData;
}

// Null terminiated incase this helps
unsigned char *OsEncryption::getResults(void)
{
    return mResults;
}

int OsEncryption::getResultsLen(void)
{
    return mResultsLen;
}

OsStatus OsEncryption::decrypt(void)
{
    return crypto(DECRYPT);
}

OsStatus OsEncryption::encrypt(void)
{
    return crypto(ENCRYPT);
}

// Call this after you're done with encryption
OsStatus OsEncryption::release(void)
{
    OsStatus retval = OS_FAILED;

#if defined(OSENCRYPTION)
    // TODO: Analyze performance v.s. memory gains to never release this
    if (mAlgorithm != NULL)
    {
        X509_ALGOR_free(mAlgorithm);
        mAlgorithm = NULL;
    }

    if (mResults != NULL)
    {
        OPENSSL_free(mResults);
        mResults = NULL;
        mResultsLen = 0;
    }

    retval = OS_SUCCESS;
#endif

    return retval;
}

/* //////////////////////////// PRIVATE/PROTECTED //////////////////////////////////// */
OsStatus OsEncryption::init(Direction direction)
{
    OsStatus retval = OS_FAILED;
#if defined(OSENCRYPTION)
    release();

    if (mKeyLen > 0 && mKey != NULL && mDataLen > 0 && mData != NULL)
    {
        ERR_clear_error();

        // 1. Initializing algorithms is handled automatically in 1.1.0+
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        SSLeay_add_all_algorithms();
#endif

        // 2. Setup the PBE algorithm
        mAlgorithm = PKCS5_pbe_set(NID_pbeWithMD5AndDES_CBC,
                                   PKCS5_DEFAULT_ITER, mSalt, mSaltLen);

        if (mAlgorithm != NULL)
        {
            // 3. Ensure the context is initialized/reset
            if (mContext == NULL)
            {
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
                mContext = EVP_CIPHER_CTX_new();
#else
                mpContext = (EVP_CIPHER_CTX*)malloc(sizeof(EVP_CIPHER_CTX));
                if (mpContext) EVP_CIPHER_CTX_init(mpContext);
#endif
            }
            else
            {
                // If it already exists, just reset it for a new operation
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
                EVP_CIPHER_CTX_reset(mContext);
#else
                EVP_CIPHER_CTX_cleanup(mpContext);
                EVP_CIPHER_CTX_init(mpContext);
#endif
            }

            if (mContext != NULL && 
                EVP_PBE_CipherInit(mAlgorithm->algorithm, (const char *)mKey, mKeyLen,
                                   mAlgorithm->parameter, mContext, (int)direction))
            {
                // 4. Use the context pointer to get block size
                int blockSize = EVP_CIPHER_CTX_block_size(mContext);
                int allocLen = mDataLen + mHeaderLen + blockSize + 1; 
                
                mResults = (unsigned char *)OPENSSL_malloc(allocLen);
                if (mResults == NULL)
                {
                    OsSysLog::add(FAC_AUTH, PRI_ERR, "Could not allocate cryption buffer(size=%d)",
                                  allocLen);
                }
                else
                {
                    retval = OS_SUCCESS;
                }
            }
            else
            {
                OsSysLog::add(FAC_AUTH, PRI_ERR, "Could not initialize cipher");
            }
        }
        else
        {
            OsSysLog::add(FAC_AUTH, PRI_ERR, "Could not initialize cryption algorithm");
        }
    }
    else
    {
        OsSysLog::add(FAC_AUTH, PRI_ERR, "No encryption key(%d) or data(%d) set.\n",
            mKeyLen, mDataLen);
    }
#endif

    return retval;
}

OsStatus OsEncryption::crypto(Direction direction)
{
    OsStatus retval = init(direction);

#if defined(OSENCRYPTION)
    if (retval == OS_SUCCESS)
    {
        if (sIgnoreEncryption)
        {
            memcpy(mResults, mData, mDataLen);
            mResultsLen = mDataLen;
        }
        else
        {
            retval = OS_FAILED;
            unsigned char *in = mData;
            int inLen = mDataLen;
            unsigned char *out = mResults;
            int outLen = 0;

            if (mHeaderLen > 0)
            {
                if (direction == ENCRYPT)
                {
                    // copy in header
                    memcpy(out, mHeader, mHeaderLen);
                    out += mHeaderLen;
                    outLen += mHeaderLen;
                }
                else
                {
                    // ignore header
                    in += mHeaderLen;
                    inLen -= mHeaderLen;
                }
            }

            int outLenPart1 = 0;
            if (EVP_CipherUpdate(mContext, out, &outLenPart1, in, inLen))
            {
                out += outLenPart1;
                int outLenPart2 = 0;
                if (EVP_CipherFinal(mContext, out, &outLenPart2))
                {
                    outLen += outLenPart1 + outLenPart2;
                    retval = OS_SUCCESS;
                    mResults[outLen] = 0;
                    mResultsLen = outLen;
                }
            }
        }
    }

    if (retval != OS_SUCCESS)
    {
        openSslError();
        release();
    }

#endif
   return retval;
}


UtlBoolean OsEncryption::openSslError(void)
{
#if defined(OSENCRYPTION)
    unsigned long err = ERR_get_error();
    if (err != 0)
    {
        // 1. Loading strings is automatic in 1.1.0 and 3.x. 
        // Only call these on legacy 1.0.x versions.
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        ERR_load_crypto_strings();
        ERR_load_ERR_strings();
#endif

        char errbuff[256];
        errbuff[0] = 0;
        ERR_error_string_n(err, errbuff, sizeof(errbuff));

        // 2. ERR_func_error_string is deprecated in 3.x and usually returns NULL.
        // We use a guard to handle the display gracefully.
        const char* libStr    = ERR_lib_error_string(err);
        const char* reasonStr = ERR_reason_error_string(err);
        
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        const char* funcStr = "N/A (OpenSSL 3.x)";
#else
        const char* funcStr = ERR_func_error_string(err);
#endif

        osPrintf("OpenSSL ERROR:\n\tlib:%s\n\tfunction:%s\n\treason:%s\n\tfull:%s\n",
            libStr ? libStr : "unknown",
            funcStr ? funcStr : "unknown",
            reasonStr ? reasonStr : "unknown",
            errbuff);

        // 3. Freeing strings is also unnecessary/automatic in modern versions.
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        ERR_free_strings();
#endif

        return TRUE;
    }
#endif

    return FALSE;
}
