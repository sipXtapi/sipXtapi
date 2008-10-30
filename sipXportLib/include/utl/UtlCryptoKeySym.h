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

#ifndef _UtlCryptoKeySym_h_
#define _UtlCryptoKeySym_h_

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
*  @brief  A symmetric key implementation of UtlCryptoKey
*/
class UtlCryptoKeySym : public UtlCryptoKey
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Constructor
   UtlCryptoKeySym();

     /// Destructor
   ~UtlCryptoKeySym();

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

     /// Generates a new key.
   int generateKey();
     /**<
     *  @NOTE: Don't use it due the not strong algorithm, for testing only
     *  @return 0 on success, non-0 error code on failure
     */
   
     /// Loads a binary key value
   int loadBinaryKey(const unsigned char* pSrc, int srcLen);
     /**<
     *  @return 0 on success
     */

     /// Retrieves a binary key value (for later use by loadBinaryKey)
   UtlCryptoData* getBinaryKey() const;

     /// Returns the max encrypted size of srcLen bytes from encrypt()
   int getMaxEncryptedSize(int srcLen) const;
   
     /// @copydoc UtlCryptoKey::encrypt
   int encrypt(const unsigned char* pSrc,
               int srcLen,
               unsigned char* pDest,
               int* pDestLen) const;

   int getMaxDecryptedSize(int srcLen) const;

     /// @copydoc UtlCryptoKey::decrypt
   int decrypt(const unsigned char* pSrc,
               int srcLen,
               unsigned char* pDest,
               int* pDestLen) const;

//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{

//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{

     /// Return total key length
   int getTotalKeyLength();

     /// Dump key to memeory
   UtlBoolean getBinaryKey(char* buffer, int bufferLength);

     /// Get humane-readable describe
   UtlString output() const;


//@}

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:


/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:

   void clearKey();

   const EVP_CIPHER* const mpCipher;
   const int mKeyLen;
   const int mIvLen;
   UtlCryptoData* mpKey;
   UtlCryptoData* mpIv;
};


/* ============================ INLINE METHODS ============================ */



#endif  // _UtlCryptoKeyRsa_h_
