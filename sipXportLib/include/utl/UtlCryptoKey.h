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

#ifndef _UtlCryptoKey_h_
#define _UtlCryptoKey_h_

#include "utl/UtlString.h"
#include "utl/UtlCryptoData.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
typedef struct rsa_st        RSA;
typedef struct evp_cipher_st EVP_CIPHER;
typedef struct env_md_st     EVP_MD;

// DEFINES
// MACROS
// STATIC VARIABLE INITIALIZATIONS


/**
*  @brief A generic cryptography key base class
*/
class UtlCryptoKey
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

     /// What type of Key is this?
   enum KeyType
   {
      KEY_INVALID,    ///< This key is not currently valid
      KEY_SYMMETRIC,  ///< This is a symmetric key
      KEY_PRIVATE,    ///< This is a private key (also includes a public key)
      KEY_PUBLIC,     ///< This is only a public key
      NUM_KEY_TYPES   ///< Number of key types defined
   };

/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Constructor
   UtlCryptoKey();

     /// Destructor
   virtual ~UtlCryptoKey();

   // Key loading & retrieval

     /// Generates a new RSA private/public key pair
   virtual int generateKey() = 0;
     /**<
     * @return 0 on success, non-0 error code on failure
     */

     /// Imports a key from the given file
   virtual int importFromFile(const char* pFilename);
     /**<
     * @return 0 on success, non-0 error code on failure
     */

     /// Loads a binary key value
   virtual int loadBinaryKey(const unsigned char* pSrc,
                             int srcLen);

     /// Retrieves a binary key value (for later use by loadBinaryKey)
   virtual UtlCryptoData* getBinaryKey() const;

   // Encryption & decryption

     /// Returns the max encrypted size of srcLen bytes from encrypt()
   virtual int getMaxEncryptedSize(int srcLen) const = 0;

     /// Encrypts the given source data with the current key
   virtual int encrypt(const unsigned char* pSrc,
                       int srcLen,
                       unsigned char* pDest,
                       int* pDestLen) const = 0;
     /**<
     *  @param[in]  pSrc - Source data
     *  @param[in]  srcLen - Source data len
     *  @param[out] pDest - Destination buffer
     *  @param[in,out] pDestLen - Size of destination buffer and
     *                 actual encrypted size after operation
     *
     *  @return Length of the encrypted data (0 on error)
     */

     /// Encrypts the given source data with the current key
   virtual UtlCryptoData* encrypt(const unsigned char* pSrc,
                                  int srcLen) const;
     /**<
     *  @param[in]  pSrc - Source data
     *  @param[in]  srcLen - Source data len
     *
     *  @return UtlCryptoData allocated object with encrypted data
     *  NOTE: User must free returned object
     */

     /// Returns the max decrypted size of srcLen bytes from decrypt()
   virtual int getMaxDecryptedSize(int srcLen) const = 0;


     /// Decrypts the given source data with the current key
   virtual int decrypt(const unsigned char* pSrc,
                       int srcLen,
                       unsigned char* pDest,
                       int* pDestLen) const = 0;
     /**<
     *  @param[in]  pSrc - Source data
     *  @param[in]  srcLen - Source data len
     *  @param[out] pDest - Destination buffer
     *  @param[in,out] pDestLen - Size of destination buffer and
     *                 actual decrypted size after operation
     *
     *  @return Length of the decrypted data (0 on error)
     */

     /// Decrypts the given source data with the current key
   virtual UtlCryptoData* decrypt(const unsigned char* pSrc,
                                  int srcLen) const;
     /**<
     *  @param[in]  pSrc - Source data
     *  @param[in]  srcLen - Source data len
     *
     *  @return UtlCryptoData allocated object with decrypted data
     *  NOTE: User must free returned object
     */

     /// Signing & verifying
   virtual int getMaxSignatureSize(int srcLen) const;

     /// Signs source data
   virtual int sign(const unsigned char* pSrc,
                    int srcLen,
                    unsigned char* pDest,
                    int* pDestLen) const;
     /**<
     *  @param[in]  pSrc - Source data
     *  @param[in]  srcLen - Source data len
     *  @param[out] pDest - Destination buffer
     *  @param[in,out] pDestLen - Size of destination buffer and
     *                 actual decrypted size after operation
     *
     *  @return Length of the decrypted data (0 on error)
     */

     /// Signs source data
   virtual UtlCryptoData* sign(const unsigned char* pSrc,
                               int srcLen) const;
     /**<
     *  @param[in]  pSrc - Source data
     *  @param[in]  srcLen - Source data len
     *
     *  @return Valid signature
     *  NOTE: User must free returned object
     */

     /// Verifies that the signature is valid for the source data
   virtual int verify(const unsigned char* pSrc,
                      int srcLen,
                      const unsigned char* pSig,
                      int sigLen) const;
     /**<
     *  @param[in]  pSrc - Source data
     *  @param[in]  srcLen - Source data len
     *  @param[in]  pSig - Signature data
     *  @param[in]  sigLen - Signature data len
     *
     *  @return 0 if signature is valid, non-0 if not
     */

   //@}

   /* ============================== ACCESSORS =============================== */
   ///@name Accessors
   //@{

   inline bool isValid() const;
   inline bool isPrivate() const;
   inline bool isPublic() const;
   inline bool isSymmetric() const;
   inline KeyType getKeyType() const;
   inline unsigned long getLastError() const;

   virtual UtlString output() const;

   //@}
   // STATICS

     /// Returns the digest algorithm type that computeDigest() will return
   static int getDigestAlgType();

     /// Returns the max size of a digest that computeDigest() will return
   static int getMaxDigestSize(int srcLen);

     /// Computes message digest (MD) of given data
   static int computeDigest(const unsigned char* pSrc,
                            int srcLen,
                            unsigned char* pDest,
                            int* pDestLen);
     /**<
     *  @param[in]  pSrc - Source data
     *  @param[in]  srcLen - Source data len
     *  @param[in]  pDest - Destination buffer
     *  @param[out] pDestLen - Size of destination buffer and
     *              actual size after operation
     *
     *  @return Valid signature
     */

     /// Computes message digest (MD) of given data
   static UtlCryptoData* computeDigest(const unsigned char* pSrc,
                                       int srcLen);
     /**<
     *  @param[in]  pSrc - Source data
     *  @param[in]  srcLen - Source data len
     *
     *  @return Valid signature
     */

     /// Returns the max base64-encoded size of srcLen bytes from base64Encode
   static int getMaxBase64EncodedSize(int srcLen);

     /// Encodes the given binary data in base64 format
   static int base64Encode(const unsigned char* pSrc,
                           int srcLen,
                           unsigned char* pDest,
                           int* pDestLen);
     /**<
     *  @param[in]  pSrc - Source data
     *  @param[in]  srcLen - Source data len
     *  @param[in]  pDest - Destination buffer
     *  @param[out] pDestLen - Size of destination buffer and
     *              actual size after operation
     *
     *  @return written characters
     */

     /// Encodes the given binary data in base64 format
   static UtlString base64Encode(const unsigned char* pSrc,
                                 int srcLen);
     /**<
     *  @param[in]  pSrc - Source data
     *  @param[in]  srcLen - Source data len
     *
     *  @return Encoded data
     */

     /// Returns the max decoded size of srcLen bytes from base64Decode
   static int getMaxBase64DecodedSize(int srcLen);

     /// Decodes the given base64 data into binary format
   static int base64Decode(const unsigned char* pSrc,
                           int srcLen,
                           unsigned char* pDest,
                           int* pDestLen);
     /**<
     *  @param[in]  pSrc - Source data
     *  @param[in]  srcLen - Source data len
     *  @param[in]  pDest - Destination buffer
     *  @param[out] pDestLen - Size of destination buffer and
     *              actual size after operation
     *
     *  @return written characters
     */

     /// Decodes the given base64 data into binary format 
   static int base64Decode(const UtlString& pSrc,
                           unsigned char* pDest,
                           int* pDestLen);
     /**<
     *  @param[in]  pSrc - Source data
     *  @param[in]  pDest - Destination buffer
     *  @param[out] pDestLen - Size of destination buffer and
     *              actual size after operation
     *
     *  @return Length of the returned decoded data (0 on error)
     */

     /// Decodes the given base64 data into binary format
   static UtlCryptoData* base64Decode(const unsigned char* pSrc,
                                      int srcLen);
     /**<
     *  @param[in]  pSrc - Source data
     *  @param[in]  srcLen - length of source data
     *
     *  @return Decoded data
     */

     /// Decodes the given base64 data into binary format
   static UtlCryptoData* base64Decode(const UtlString& pSrc);
     /**<
     *  @param[in]  pSrc - Source data
     *
     *  @return Decoded data
     */

protected:

   inline KeyType  setKeyType(KeyType type);
   unsigned long   setLastError(unsigned long err) const;
   virtual void    clearKey();
   
     /// Decode base64 character
   static int getBase64Idx(unsigned char c);

     /// Returns true if the given char is a base64 char
   static bool isBase64(unsigned char c);

   static const char sBase64Chars[]; ///< Set of base64 chars

private:

   KeyType mKeyType;               ///< What type of key are we?
   mutable unsigned long mLastErr; ///< Saved error from last failing method

   // STATICS
   static const EVP_MD* spMdAlg;  ///< Message Digest algorithm to use
};


/* ============================ INLINE METHODS ============================ */

bool UtlCryptoKey::isValid() const
{
   return mKeyType != KEY_INVALID;
}

bool UtlCryptoKey::isPrivate() const
{
   return mKeyType == KEY_PRIVATE;
}

bool UtlCryptoKey::isPublic() const
{
   return mKeyType == KEY_PUBLIC;
}

bool UtlCryptoKey::isSymmetric() const
{
   return mKeyType == KEY_SYMMETRIC;
}

UtlCryptoKey::KeyType UtlCryptoKey::getKeyType() const
{
   return mKeyType;
}

unsigned long UtlCryptoKey::getLastError() const
{
   return mLastErr;
}

UtlCryptoKey::KeyType UtlCryptoKey::setKeyType(KeyType type)
{
   mKeyType=type;
   return type;
}


#endif // Include guard


