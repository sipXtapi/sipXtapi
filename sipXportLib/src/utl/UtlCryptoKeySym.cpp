//  
// Copyright (C) 2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2008 Mutualink, Inc. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef HAVE_SSL

#include "utl/UtlCryptoKeySym.h"

#include "utl/UtlRandom.h"

#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/pem.h>

UtlCryptoKeySym::UtlCryptoKeySym()
: mpCipher(EVP_des_cbc())
, mKeyLen(EVP_CIPHER_key_length(mpCipher))
, mIvLen(EVP_CIPHER_iv_length(mpCipher))
, mpKey(0)
, mpIv(0)
{
}

UtlCryptoKeySym::~UtlCryptoKeySym()
{
   clearKey();
}

void UtlCryptoKeySym::clearKey()
{
   if (mpKey)
   {
      delete mpKey;
      mpKey = 0;
   }
   if (mpIv)
   {
      delete mpIv;
      mpIv = 0;
   }

   UtlCryptoKey::clearKey();
}

int UtlCryptoKeySym::generateKey()
{
   // Make sure we don't leave any previous keys hanging around
   clearKey();

   // TODO: Need to do real keys & IVs here!
   UtlRandom r;

   mpKey = new UtlCryptoData(mKeyLen);
   for (int i=0; i < mKeyLen; i++)
      (*mpKey)[i] = r.rand();

   if (mIvLen)
   {
      mpIv = new UtlCryptoData(mIvLen);
      for (int i=0; i < mIvLen; i++)
         (*mpIv)[i] = r.rand();
   }

   setKeyType(KEY_SYMMETRIC);

   return 0;
}

int UtlCryptoKeySym::loadBinaryKey(const unsigned char* pSrc, int srcLen)
{
   // Make sure we don't leave any previous keys hanging around
   clearKey();

   if (!pSrc || srcLen < mKeyLen + mIvLen)
      return -1;

   // First load the Key
   mpKey = new UtlCryptoData(mKeyLen);
   for (int i=0; i < mKeyLen; i++)
      (*mpKey)[i] = *pSrc++;

   // Then load the IV if needed
   if (mIvLen)
   {
      mpIv = new UtlCryptoData(mIvLen);
      for (int i=0; i < mIvLen; i++)
         (*mpIv)[i] = *pSrc++;
   }

   setKeyType(KEY_SYMMETRIC);

   return 0;
}

UtlCryptoData* UtlCryptoKeySym::getBinaryKey() const
{
   // Make sure we have a key (and IV if needed) before proceeding
   if (!mpKey || (!mpIv && mIvLen))
      return 0;

   UtlCryptoData* pData = new UtlCryptoData(mKeyLen + mIvLen);
   unsigned char* pDest = pData->data();

   // First write the key value
   for (int i=0; i < mKeyLen; i++)
      *pDest++ = (*mpKey)[i];

   // Then write the IV if needed
   if (mIvLen)
   {
      for (int i=0; i < mIvLen; i++)
         *pDest++ = (*mpIv)[i];
   }

   return pData;
}


int UtlCryptoKeySym::getMaxEncryptedSize(int srcLen) const
{
   return srcLen + EVP_CIPHER_block_size(mpCipher);
}


int UtlCryptoKeySym::encrypt(const unsigned char* pSrc,
                             int srcLen,
                             unsigned char* pDest,
                             int* pDestLen) const
{
   if (!pSrc || !srcLen || !pDest || !pDestLen ||
      *pDestLen < getMaxEncryptedSize(srcLen))
      return 0;

   *pDestLen = 0;

   // Make sure we have a key (and IV if needed) before proceeding
   if (!mpKey || (!mpIv && mIvLen))
   {
      osPrintf("*****Need valid key before encrypting");
      return 0;
   }

   EVP_CIPHER_CTX ctx;
   EVP_CIPHER_CTX_init(&ctx);

   if (!EVP_EncryptInit_ex(&ctx, mpCipher, NULL,
      mpKey->data(), mpIv->data()))
   {
      osPrintf("*****EVP_EncryptInit_ex failed");
      setLastError(ERR_get_error());
      EVP_CIPHER_CTX_cleanup(&ctx);
      return 0;
   }

   // Go ahead and encrypt the src data into the dest buffer

   int bytesWritten = 0;
   unsigned char* pDestTail = pDest;
   if (!EVP_EncryptUpdate(&ctx, pDestTail, &bytesWritten, pSrc, srcLen))
   {
      osPrintf("*****EVP_EncryptUpdate failed");
      setLastError(ERR_get_error());
      EVP_CIPHER_CTX_cleanup(&ctx);
      return 0;
   }

   pDestTail += bytesWritten;
   int bytesFinal = 0;
   if (!EVP_EncryptFinal_ex(&ctx, pDestTail, &bytesFinal))
   {
      osPrintf("*****EVP_EncryptFinal_ex failed");
      setLastError(ERR_get_error());
      EVP_CIPHER_CTX_cleanup(&ctx);
      return 0;
   }

   // How many total bytes did we write to pDest?
   *pDestLen = bytesWritten + bytesFinal;

   EVP_CIPHER_CTX_cleanup(&ctx);
   setLastError(0);
   return *pDestLen;
}

int UtlCryptoKeySym::getMaxDecryptedSize(int srcLen) const
{
   return srcLen;
}

int UtlCryptoKeySym::decrypt(const unsigned char* pSrc,
                             int srcLen,
                             unsigned char* pDest,
                             int* pDestLen) const
{
   if (!pSrc || !srcLen || !pDest || !pDestLen ||
      *pDestLen < getMaxDecryptedSize(srcLen))
      return 0;

   *pDestLen = 0;

   // Make sure we have a key (and IV if needed) before proceeding
   if (!mpKey || (!mpIv && mIvLen))
   {
      osPrintf("*****Need valid key before decrypting");
      return 0;
   }

   EVP_CIPHER_CTX ctx;
   EVP_CIPHER_CTX_init(&ctx);

   if (!EVP_DecryptInit_ex(&ctx, mpCipher, NULL,
      mpKey->data(), mpIv->data()))
   {
      osPrintf("*****EVP_DecryptInit_ex failed");
      setLastError(ERR_get_error());
      EVP_CIPHER_CTX_cleanup(&ctx);
      return 0;
   }

   // Go ahead and encrypt the src data into the dest buffer

   int bytesWritten = 0;
   unsigned char* pDestTail = pDest;
   if (!EVP_DecryptUpdate(&ctx, pDestTail, &bytesWritten, pSrc, srcLen))
   {
      osPrintf("*****EVP_DecryptUpdate failed");
      setLastError(ERR_get_error());
      EVP_CIPHER_CTX_cleanup(&ctx);
      return 0;
   }

   pDestTail += bytesWritten;
   int bytesFinal = 0;
   if (!EVP_DecryptFinal_ex(&ctx, pDestTail, &bytesFinal))
   {
      osPrintf("*****EVP_DecryptFinal_ex failed");
      setLastError(ERR_get_error());
      EVP_CIPHER_CTX_cleanup(&ctx);
      return 0;
   }

   // How many total bytes did we write to pDest?
   *pDestLen = bytesWritten + bytesFinal;

   EVP_CIPHER_CTX_cleanup(&ctx);
   setLastError(0);
   return *pDestLen;
}

int UtlCryptoKeySym::getTotalKeyLength()
{
   return mKeyLen + mIvLen;
}

UtlBoolean UtlCryptoKeySym::getBinaryKey(char* buffer, int bufferLength)
{
   if (bufferLength < getTotalKeyLength())
      return FALSE;
   
   memcpy(buffer, mpKey, mKeyLen);
   memcpy(buffer + mKeyLen, mpIv, mIvLen);

   return TRUE;
}

UtlString UtlCryptoKeySym::output() const
{
   // Write any base class details first
   UtlString str = UtlCryptoKey::output();

   if (mpKey)
   {
      str.append("\n        Key: ");
      str.append(mpKey->dumpHex());
   }

   if (mpIv)
   {
      str.append("\n        IV:  ");
      str.append(mpIv->dumpHex());
   }

   return str;
}

#endif
