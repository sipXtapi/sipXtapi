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

#include "utl/UtlCryptoKeyRsa.h"

#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/pem.h>


UtlCryptoKeyRsa::UtlCryptoKeyRsa()
: mpRsa(0)
{
}

UtlCryptoKeyRsa::~UtlCryptoKeyRsa()
{
   clearKey();
}

void UtlCryptoKeyRsa::clearKey()
{
   if (mpRsa)
   {
      RSA_free(mpRsa);
      mpRsa = 0;
   }

   UtlCryptoKey::clearKey();
}

int UtlCryptoKeyRsa::generateKey()
{
   // Make sure we don't leave any previous keys hanging around
   clearKey();

   int numKeyBits = 2048;
   unsigned long keyExponent = 65537;
   mpRsa = RSA_generate_key(numKeyBits, keyExponent, NULL, 0);

   if (mpRsa)
   {
      setKeyType(KEY_PRIVATE);
      return setLastError(0);
   }
   else
   {
      return setLastError(ERR_get_error());
   }
}

int UtlCryptoKeyRsa::importFromFile(const char* pFilename)
{
   // Make sure we don't leave any previous keys hanging around
   clearKey();

   FILE *pFile = fopen(pFilename, "rt");
   if (!pFile)
      return -1;

   // First try to read a public key
   mpRsa = PEM_read_RSA_PUBKEY(pFile, 0, 0, 0);
   if (mpRsa)
      setKeyType(KEY_PUBLIC);
   else
   {
      // If that failed, try to read a private key
      fseek(pFile, 0, SEEK_SET);
      mpRsa = PEM_read_RSAPrivateKey(pFile, 0, 0, 0);
      if (mpRsa)
         setKeyType(KEY_PRIVATE);
   }

   fclose(pFile);

   if (isValid())
      return setLastError(0);
   else
      return -1;
}

inline int UtlCryptoKeyRsa::getMaxEncryptedSize(int srcLen) const
{
   return RSA_size(mpRsa);
}


int UtlCryptoKeyRsa::encrypt(const unsigned char* pSrc,
                             int srcLen,
                             unsigned char* pDest,
                             int* pDestLen) const
{
   if (isPrivate())
      return encryptPrivate(pSrc, srcLen, pDest, pDestLen);
   else if (isPublic())
      return encryptPublic(pSrc, srcLen, pDest, pDestLen);

   if (pDestLen)
      *pDestLen = 0;
   return 0;   // TODO: Set lastError?
}

inline int UtlCryptoKeyRsa::getMaxDecryptedSize(int srcLen) const
{
   return RSA_size(mpRsa);
}


int UtlCryptoKeyRsa::decrypt(const unsigned char* pSrc,
                             int srcLen,
                             unsigned char* pDest,
                             int* pDestLen) const
{
   if (isPrivate())
      return decryptPrivate(pSrc, srcLen, pDest, pDestLen);
   else if (isPublic())
      return decryptPublic(pSrc, srcLen, pDest, pDestLen);

   if (pDestLen)
      *pDestLen = 0;
   return 0;   // TODO: Set lastError?
}


int UtlCryptoKeyRsa::encryptPrivate(const unsigned char* pSrc,
                                    int srcLen,
                                    unsigned char* pDest,
                                    int* pDestLen) const
{
   if (!pSrc || !srcLen || !pDest || !pDestLen ||
      *pDestLen < getMaxEncryptedSize(srcLen))
      return 0;

   // Encrypt the data into the destination buffer
   int destLen = RSA_private_encrypt(srcLen, pSrc, pDest,
      mpRsa, RSA_PKCS1_PADDING);
   if (destLen == -1)
   {
      setLastError(ERR_get_error());
      *pDestLen = 0;
      return 0;
   }

   setLastError(0);
   *pDestLen = destLen;
   return destLen;
}

int UtlCryptoKeyRsa::decryptPrivate(const unsigned char* pSrc,
                                    int srcLen,
                                    unsigned char* pDest,
                                    int* pDestLen) const
{
   if (!pSrc || !srcLen || !pDest || !pDestLen ||
      *pDestLen < getMaxDecryptedSize(srcLen))
      return 0;

   // Decrypt the data into the destination buffer
   int destLen = RSA_private_decrypt(srcLen, pSrc, pDest,
      mpRsa, RSA_PKCS1_PADDING);
   if (destLen == -1)
   {
      setLastError(ERR_get_error());
      *pDestLen = 0;
      return 0;
   }

   setLastError(0);
   *pDestLen = destLen;
   return destLen;
}

int UtlCryptoKeyRsa::encryptPublic(const unsigned char* pSrc,
                                   int srcLen,
                                   unsigned char* pDest,
                                   int* pDestLen) const
{
   if (!pSrc || !srcLen || !pDest || !pDestLen ||
      *pDestLen < getMaxEncryptedSize(srcLen))
      return 0;

   // Encrypt the data into the destination buffer
   int destLen = RSA_public_encrypt(srcLen, pSrc, pDest,
      mpRsa, RSA_PKCS1_PADDING);
   if (destLen == -1)
   {
      setLastError(ERR_get_error());
      *pDestLen = 0;
      return 0;
   }

   setLastError(0);
   *pDestLen = destLen;
   return destLen;
}

int UtlCryptoKeyRsa::decryptPublic(const unsigned char* pSrc,
                                   int srcLen,
                                   unsigned char* pDest,
                                   int* pDestLen) const
{
   if (!pSrc || !srcLen || !pDest || !pDestLen ||
      *pDestLen < getMaxDecryptedSize(srcLen))
      return 0;

   // Decrypt the data into the destination buffer
   int destLen = RSA_public_decrypt(srcLen, pSrc, pDest,
      mpRsa, RSA_PKCS1_PADDING);
   if (destLen == -1)
   {
      setLastError(ERR_get_error());
      *pDestLen = 0;
      return 0;
   }

   setLastError(0);
   *pDestLen = destLen;
   return destLen;
}


int UtlCryptoKeyRsa::sign(const unsigned char* pSrc,
                          int srcLen,
                          unsigned char* pDest,
                          int* pDestLen) const
{
   if (!pSrc || !srcLen || !pDest || !pDestLen ||
      *pDestLen < getMaxSignatureSize(srcLen))
   {
      if (pDestLen)
         *pDestLen = 0;
      return 0;
   }

   *pDestLen = 0;

   // First, compute the Message Digest (MD) of the source data
   int mdLen = EVP_MAX_MD_SIZE;
   unsigned char md[EVP_MAX_MD_SIZE];
   if (computeDigest(pSrc, srcLen, &md[0], &mdLen) == 0)
      return 0;

   // Next, sign the Message Digest & return that
   if (!RSA_sign(getDigestAlgType(), &md[0], mdLen,
      pDest, (unsigned*)pDestLen, mpRsa))
   {
      osPrintf("*****RSA_sign failed");
      return 0;
   }

   setLastError(0);
   return *pDestLen;
}

int UtlCryptoKeyRsa::verify(const unsigned char* pSrc,
                            int srcLen,
                            const unsigned char* pSig,
                            int sigLen) const
{
   // First, compute the Message Digest (MD) of the source data
   int mdLen = EVP_MAX_MD_SIZE;
   unsigned char md[EVP_MAX_MD_SIZE];
   if (computeDigest(pSrc, srcLen, &md[0], &mdLen) == 0)
      return -1;

   // Next, see if the signature matches the MD
   if (!RSA_verify(getDigestAlgType(), &md[0], mdLen,
      const_cast<unsigned char*>(pSig), sigLen, mpRsa))
   {
      return -1;
   }

   return 0;
}

UtlString UtlCryptoKeyRsa::output() const
{
   // Write any base class details first
   UtlString str = UtlCryptoKey::output();

   BIO* pBio = BIO_new(BIO_s_mem());

   RSA_print(pBio, mpRsa, 8 /*indent*/ );
   BUF_MEM* pBufMem;
   BIO_get_mem_ptr(pBio, &pBufMem);
   pBufMem->data[pBufMem->length] = 0;

   str.append(pBufMem->data);

   BIO_free_all(pBio);

   return str;
}


int UtlCryptoKeyRsa::getMaxSignatureSize(int srcLen) const
{
   return RSA_size(mpRsa);
}

#endif
