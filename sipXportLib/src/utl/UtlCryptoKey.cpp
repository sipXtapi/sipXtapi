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
#include <ctype.h>

#include "utl/UtlCryptoKey.h"

#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/pem.h>

#ifdef WIN32
#  pragma comment (lib, "libeay32.lib")
#  pragma comment (lib, "ssleay32.lib")
#endif

// Static initialization
const EVP_MD* UtlCryptoKey::spMdAlg = EVP_sha1();


UtlCryptoKey::UtlCryptoKey()
: mKeyType(KEY_INVALID), mLastErr(0)
{
}

UtlCryptoKey::~UtlCryptoKey()
{
   clearKey();
}


void UtlCryptoKey::clearKey()
{
   mKeyType = KEY_INVALID;
   mLastErr = 0;
}

int UtlCryptoKey::importFromFile(const char* pFilename)
{
   return -1;
}

int UtlCryptoKey::loadBinaryKey(const unsigned char* pSrc, int srcLen)
{
   return -1;
}

UtlCryptoData* UtlCryptoKey::getBinaryKey() const
{
   return 0;
}

UtlCryptoData* UtlCryptoKey::encrypt(const unsigned char* pSrc, int srcLen) const
{
   // Allocate a destination buffer
   int destLen = getMaxEncryptedSize(srcLen);
   UtlCryptoData* pDest = new UtlCryptoData(destLen);
   if (!pDest)
      return 0;

   // Encrypt the data into the buffer
   if (encrypt(pSrc, srcLen, pDest->data(), &destLen) == 0)
   {
      delete pDest;
      return 0;
   }

   // Set the size of the destination buffer to the returned size
   pDest->resize(destLen);
   return pDest;
}

UtlCryptoData* UtlCryptoKey::decrypt(const unsigned char* pSrc, int srcLen) const
{
   // Allocate a destination buffer
   int destLen = getMaxDecryptedSize(srcLen);
   UtlCryptoData* pDest = new UtlCryptoData(destLen);
   if (!pDest)
      return 0;

   // Decrypt the data into the buffer
   if (decrypt(pSrc, srcLen, pDest->data(), &destLen) == 0)
   {
      delete pDest;
      return 0;
   }

   // Set the size of the destination buffer to the returned size
   pDest->resize(destLen);
   return pDest;
}


UtlCryptoData* UtlCryptoKey::sign(const unsigned char* pSrc, int srcLen) const
{
   // Allocate a destination buffer
   int destLen = getMaxSignatureSize(srcLen);
   UtlCryptoData* pDest = new UtlCryptoData(destLen);
   if (!pDest)
      return 0;

   // Sign the data & write signature into the buffer
   if (sign(pSrc, srcLen, pDest->data(), &destLen) == 0)
   {
      delete pDest;
      return 0;
   }

   // Set the size of the destination buffer to the returned size
   pDest->resize(destLen);
   return pDest;
}

int UtlCryptoKey::getMaxSignatureSize(int srcLen) const
{
   return 0;
}

int UtlCryptoKey::sign(const unsigned char* pSrc,
                       int srcLen,
                       unsigned char* pDest,
                       int* pDestLen) const
{
    if (pDestLen)
        *pDestLen = 0;
    return 0;
}

int UtlCryptoKey::verify(const unsigned char* pSrc,
                         int srcLen,
                         const unsigned char* pSig,
                         int sigLen) const
{
   return -1;
}

unsigned long UtlCryptoKey::setLastError(unsigned long err) const
{
   // Allow mLastErr to be set by const methods as well
   // (technically a violation of the const principle, but mLastErr isn't
   //  really part of the state of the key; it's more of a convenience thing)
   mLastErr = err;
   return err;
}

UtlString UtlCryptoKey::output() const
{
   UtlString str;
   str.append("KeyType=");

   switch (mKeyType)
   {
   case UtlCryptoKey::KEY_INVALID:
      str.append("(invalid)");
      break;
   case UtlCryptoKey::KEY_SYMMETRIC:
      str.append("Symmetric");
      break;
   case UtlCryptoKey::KEY_PRIVATE:
      str.append("Private");
      break;
   case UtlCryptoKey::KEY_PUBLIC:
      str.append("Public");
      break;
   default:
      str.appendFormat("(unknown value %d!)", mKeyType);
   }

   return str;
}

int UtlCryptoKey::getDigestAlgType()
{
   return EVP_MD_type(spMdAlg);
}

inline int UtlCryptoKey::getMaxDigestSize(int srcLen)
{
   return EVP_MD_size(spMdAlg);
}

int UtlCryptoKey::computeDigest(const unsigned char* pSrc,
                                int srcLen,
                                unsigned char* pDest,
                                int* pDestLen)
{
   if (!pSrc || !srcLen || !pDest || !pDestLen ||
      *pDestLen < getMaxDigestSize(srcLen))
      return 0;

   *pDestLen = 0;

   EVP_MD_CTX mdctx;
   EVP_MD_CTX_init(&mdctx);

   OpenSSL_add_all_digests();

   if (!EVP_DigestInit_ex(&mdctx, spMdAlg, NULL))
   {
      osPrintf("*****EVP_DigestInit_ex failed");
      EVP_MD_CTX_cleanup(&mdctx);
      return 0;
   }

   if (!EVP_DigestUpdate(&mdctx, pSrc, srcLen))
   {
      osPrintf("*****EVP_DigestUpdate failed");
      EVP_MD_CTX_cleanup(&mdctx);
      return 0;
   }

   if (!EVP_DigestFinal_ex(&mdctx, pDest, (unsigned*)pDestLen))
   {
      osPrintf("*****EVP_DigestFinal_ex failed");
      EVP_MD_CTX_cleanup(&mdctx);
      *pDestLen = 0;
      return 0;
   }

   EVP_MD_CTX_cleanup(&mdctx);
   return *pDestLen;
}

UtlCryptoData* UtlCryptoKey::computeDigest(const unsigned char* pSrc, int srcLen)
{
   // Allocate a destination buffer
   int destLen = getMaxDigestSize(srcLen);
   UtlCryptoData* pDest = new UtlCryptoData(destLen);
   if (!pDest)
      return 0;

   // Compute digest and write it into the buffer
   if (computeDigest(pSrc, srcLen, pDest->data(), &destLen) == 0)
   {
      delete pDest;
      return 0;
   }

   // Set the size of the destination buffer to the returned size
   pDest->resize(destLen);
   return pDest;
}

inline int UtlCryptoKey::getMaxBase64EncodedSize(int srcLen)
{
   int byteTriplets = srcLen/3 + (srcLen%3 ? 1 : 0);
   int padBytes = srcLen%3 ? (3 - srcLen%3) : 0;
   int encodedBytes = byteTriplets * 4 + padBytes;
   int crlfs = (encodedBytes / 76) + 1;

   return encodedBytes + crlfs;
}


const char UtlCryptoKey::sBase64Chars[] = 
   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
   "abcdefghijklmnopqrstuvwxyz"
   "0123456789+/";

int UtlCryptoKey::getBase64Idx(unsigned char c)
{
   if (c >= 'A' && c <= 'Z')
      return c - 'A';
   else if (c >= 'a' && c <= 'z')
      return c - 'a' + 26;
   else if (c >= '0' && c <= '9')
      return c - '0' + 52;
   else if (c == '+')
      return 62;
   else if (c == '/')
      return 63;

   return -1;
}

int UtlCryptoKey::base64Encode(const unsigned char* pSrc,
                               int srcLen,
                               unsigned char* pDest,
                               int* pDestLen)
{
   if (!pSrc || !srcLen || !pDest || !pDestLen ||
      *pDestLen < getMaxBase64EncodedSize(srcLen))
      return 0;

   *pDestLen = 0;

   const unsigned char* pSrcChar = pSrc;
   unsigned char* pDestTail = pDest;
   int srcLeft = srcLen;
   int tripCnt = 0;
   unsigned char triplet[3];       // Triplet of source binary chars
   unsigned char quadCode[4];      // Triplet encoded as four base64 chars

   // For every 3 source bytes (triplet), write 4 base64 chars (quadCode)
   while (srcLeft--)
   {
      triplet[tripCnt++] = *(pSrcChar++);
      if (tripCnt == 3)                       // If we have a full triplet
      {
         // For each group of 6 bits, produce one base64 char
         quadCode[0] = (triplet[0] & 0xfc) >> 2;
         quadCode[1] = ((triplet[0] & 0x03) << 4) + ((triplet[1] & 0xf0) >> 4);
         quadCode[2] = ((triplet[1] & 0x0f) << 2) + ((triplet[2] & 0xc0) >> 6);
         quadCode[3] = triplet[2] & 0x3f;

         for (int i = 0; i < 4; i++)
            *(pDestTail++) = sBase64Chars[quadCode[i]];

         tripCnt = 0;
      }
   }

   // If the last triplet is partially filled,
   if (tripCnt)
   {
      // Pad remaining bytes with 0s
      for (int i = tripCnt; i < 3; i++)
         triplet[i] = '\0';

      quadCode[0] = (triplet[0] & 0xfc) >> 2;
      quadCode[1] = ((triplet[0] & 0x03) << 4) + ((triplet[1] & 0xf0) >> 4);
      quadCode[2] = ((triplet[1] & 0x0f) << 2) + ((triplet[2] & 0xc0) >> 6);
      quadCode[3] = triplet[2] & 0x3f;

      // Write 2 b64 chars for triplet with 1 byte filled, or 3 for 2 bytes
      for (int i = 0; i < (tripCnt + 1); i++)
         *(pDestTail++) = sBase64Chars[quadCode[i]];

      // Append one '=' for each 0 added as padding above to fill out quadCode
      while (tripCnt++ < 3)
         *(pDestTail++) = '=';
   }

   *pDestLen = pDestTail - pDest;
   return *pDestLen;
}

UtlString UtlCryptoKey::base64Encode(const unsigned char* pSrc, int srcLen)
{
   // Allocate a destination string
   UtlString dest;
   int destLen = getMaxBase64EncodedSize(srcLen);
   dest.resize(destLen);   // Make sure there's enough room in the string

   // Encode data into the string
   if (base64Encode(pSrc, srcLen, (unsigned char*)dest.data(), &destLen) == 0)
      return "";

   // Set the size of the destination buffer to the returned size
   dest.resize(destLen);
   return dest;
}


int UtlCryptoKey::getMaxBase64DecodedSize(int srcLen)
{
   int quadCodes = srcLen/4 + 1;
   int encodedBytes = quadCodes * 3;

   return encodedBytes;
}

bool UtlCryptoKey::isBase64(unsigned char c)
{
   return (isalnum(c) || (c == '+') || (c == '/'));
}

int UtlCryptoKey::base64Decode(const unsigned char* pSrc,
                               int srcLen,
                               unsigned char* pDest,
                               int* pDestLen)
{
   if (!pSrc || !srcLen || !pDest || !pDestLen ||
      *pDestLen < getMaxBase64DecodedSize(srcLen))
      return 0;


   *pDestLen = 0;

   unsigned char* pDestTail = pDest;
   int quadCnt = 0;
   int padCnt = 0;
   unsigned char quadCode[4];      // Group of four source base64 chars
   unsigned char triplet[3];       // Triplet of decoded binary chars

   // From every 4 source bytes (quadCodes), decode 3 binary chars (triplet)
   const unsigned char* pSrcChar;
   const unsigned char* pSrcEnd = pSrc + srcLen;
   for (pSrcChar = pSrc; pSrcChar != pSrcEnd; pSrcChar++)
   {
      // Skip any whitespace inserted in the string for formatting
      if (*pSrcChar == '\r' || *pSrcChar == '\n' ||
         *pSrcChar == ' ' || *pSrcChar == '\t')
         continue;

      // Count pad characters at end of string
      if (*pSrcChar == '=')
      {
         padCnt++;
         if (padCnt == 2)    // Max of 2 pad chars allowed
            break;
         continue;
      }
      // Any other char (or valid char after a pad char is found)
      // is illegal and marks the end of the b64 string
      if (!isBase64(*pSrcChar) || padCnt)
         break;

      quadCode[quadCnt++] = *pSrcChar;
      if (quadCnt == 4)                   // If we have a full quadCode
      {
         // For each group of 4 b64 chars, produce 3 binary chars
         for (int i = 0; i < 4; i++)
            //quadCode[i] = sBase64Chars.find(quadCode[i]);
            quadCode[i] = getBase64Idx(quadCode[i]);

         triplet[0] = (quadCode[0] << 2) + ((quadCode[1] & 0x30) >> 4);
         triplet[1] = ((quadCode[1] & 0xf) << 4) + ((quadCode[2] & 0x3c) >> 2);
         triplet[2] = ((quadCode[2] & 0x3) << 6) + quadCode[3];

         for (int i = 0; i < 3; i++)
            *(pDestTail++) = triplet[i];

         quadCnt = 0;
      }
   }

   // If the last quadCode is partially filled,
   if (quadCnt)
   {
      if (quadCnt + padCnt != 4)
         osPrintf("***base64Decode: quadCnt=%d padCnt=%d after decode, total should be 4!", quadCnt, padCnt);

      // Pad rest of quad with 'A's (so will convert to 0s in next step)
      for (int i = quadCnt; i < 4; i++)
         quadCode[i] = 'A';

      // Convert ASCII to indexes into b64 table
      for (int i = 0; i < 4; i++)
         //quadCode[i] = sBase64Chars.find(quadCode[i]);
         quadCode[i] = getBase64Idx(quadCode[i]);

      triplet[0] = (quadCode[0] << 2) + ((quadCode[1] & 0x30) >> 4);
      triplet[1] = ((quadCode[1] & 0xf) << 4) + ((quadCode[2] & 0x3c) >> 2);
      triplet[2] = ((quadCode[2] & 0x3) << 6) + quadCode[3];

      for (int i = 0; i < (quadCnt - 1); i++)
         *(pDestTail++) = triplet[i];
   }

   *pDestLen = pDestTail - pDest;
   return *pDestLen;
}

int UtlCryptoKey::base64Decode(const UtlString&   pSrc,
                               unsigned char* pDest,
                               int* pDestLen)
{
   if (!pSrc)
      return 0;

   return base64Decode((const unsigned char*)pSrc.data(), pSrc.length(),
      pDest, pDestLen);
}

UtlCryptoData* UtlCryptoKey::base64Decode(const unsigned char* pSrc, int srcLen)
{
   // Allocate a destination buffer
   int destLen = getMaxBase64DecodedSize(srcLen);
   UtlCryptoData* pDest = new UtlCryptoData(destLen);
   if (!pDest)
      return 0;

   // Decode data into the buffer
   if (base64Decode(pSrc, srcLen, pDest->data(), &destLen) == 0)
   {
      delete pDest;
      return 0;
   }

   // Set the size of the destination buffer to the returned size
   pDest->resize(destLen);
   return pDest;
}

UtlCryptoData* UtlCryptoKey::base64Decode(const UtlString& pSrc)
{
   // Allocate a destination buffer
   int destLen = getMaxBase64DecodedSize(pSrc.length());
   UtlCryptoData* pDest = new UtlCryptoData(destLen);
   if (!pDest)
      return 0;

   // Decode data into the buffer
   if (base64Decode((const unsigned char*)pSrc.data(), pSrc.length(),
      pDest->data(), &destLen) == 0)
   {
      delete pDest;
      return 0;
   }

   // Set the size of the destination buffer to the returned size
   pDest->resize(destLen);
   return pDest;
}

#endif