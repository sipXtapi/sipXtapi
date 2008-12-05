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

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlCryptoData.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// DEFINES
// MACROS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* =============================== CREATORS =============================== */
UtlCryptoData::UtlCryptoData(int initSize)
{
   mData = new unsigned char[initSize];
   length = initSize;
}

UtlCryptoData::~UtlCryptoData()
{
   delete mData;
}

/* ============================= MANIPULATORS ============================= */

/* ============================== ACCESSORS =============================== */

/* =============================== INQUIRY ================================ */

void UtlCryptoData::resize(int newSize)
{
   assert (newSize <= length);
   length = newSize;
}

UtlString UtlCryptoData::dumpHex() const
{
   UtlString str;
   for (int i=0; i <size(); i++)
      str.appendFormat("%02x ", (unsigned)((*this)[i]));

   return str;
}
