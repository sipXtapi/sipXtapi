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

#ifndef _UtlCryptoData_h_
#define _UtlCryptoData_h_

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Crypto data operations.
*/
class UtlCryptoData
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Constructor
   UtlCryptoData(int initSize);

     /// Destructor
   ~UtlCryptoData();

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

     /// Get first element
   inline unsigned char* data();

     /// Get \p index element
   inline unsigned char& operator[] (int index);

//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{


//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{

     /// Get \p index element for read-only operation
   inline unsigned char operator[] (int index) const;

     /// Get size of data
   inline int size() const;

     /// Shrink to actual size
   void resize(int newSize);

     /// Dump data in humane-readable hex format
   UtlString dumpHex() const;

//@}

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:

/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:
   unsigned char* mData;
   int length;

};

/* ============================ INLINE METHODS ============================ */

unsigned char* UtlCryptoData::data()
{
   return mData;
}

unsigned char& UtlCryptoData::operator[] (int index)
{
   assert((0 <= index) && (index < length));
   return mData[index];
}

unsigned char UtlCryptoData::operator[] (int index) const
{
   assert((0 <= index) && (index < length));
   return mData[index];
}

int UtlCryptoData::size() const
{
   return length;
}

#endif  // _UtlCryptoData_h_
