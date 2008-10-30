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

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifndef _UtlCryptoKeyRsa_h_
#define _UtlCryptoKeyRsa_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlCryptoKey.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Rsa private/public keypairs.
*/
class UtlCryptoKeyRsa : public UtlCryptoKey
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Constructor
   UtlCryptoKeyRsa();

     /// Destructor
   ~UtlCryptoKeyRsa();

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

   int generateKey();

   int importFromFile(const char* pFilename);

   int getMaxEncryptedSize(int srcLen) const;

     /// @copydoc UtlCryptoKey::encrypt
   int encrypt(const unsigned char* pSrc,
               int srcLen,
               unsigned char* pDest,
               int* pDestLen) const;

   //UtlCryptoData* encrypt(const unsigned char* pSrc, int srcLen) const
   //{ return UtlCryptoKey::encrypt(pSrc, srcLen); }

   int getMaxDecryptedSize(int srcLen) const;

     /// @copydoc UtlCryptoKey::decrypt
   int decrypt(const unsigned char* pSrc,
               int srcLen,
               unsigned char* pDest,
               int* pDestLen) const;

     /// Returns the max size of a signature that sign() will return
   int getMaxSignatureSize(int srcLen) const;

     /// @copydoc UtlCryptoKey::sign
   int sign(const unsigned char* pSrc,
            int srcLen,
            unsigned char* pDest,
            int* pDestLen) const;

     /// @copydoc UtlCryptoKey::verify
   int verify(const unsigned char* pSrc,
              int srcLen,
              const unsigned char* pSig,
              int sigLen) const;

     /// Decrypts the given source data with the current PRIVATE key
   int encryptPrivate(const unsigned char* pSrc,
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
     *  @return Length of the encrypted data (0 on error)
     */

     /// Decrypts the given source data with the current PRIVATE key
   int decryptPrivate(const unsigned char* pSrc,
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

     /// Encrypts the given source data with the current PUBLIC key
   int encryptPublic(const unsigned char* pSrc,
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
     *  @return Length of the encrypted data (0 on error)
     */

     /// Decrypts the given source data with the current PUBLIC key
   int decryptPublic(const unsigned char* pSrc,
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

//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{

//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{

     /// Get humane-readable describe
   UtlString output() const;


//@}

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:


/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:

   void clearKey();
   RSA* mpRsa;      ///< Internal (transparent) openssl struct
};


/* ============================ INLINE METHODS ============================ */

#endif  // _UtlCryptoKeyRsa_h_
