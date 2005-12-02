// $Id: //depot/OPENDEV/sipXphone/src/pinger/PingerMsg.cpp#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "pinger/PingerMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Message object used to communicate phone set information

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PingerMsg::PingerMsg(int msgSubType, const char* pStrParam, int intParam)
:  OsMsg(OsMsg::PINGER_MSG, msgSubType),
   mIntParam(intParam)
{
   if (pStrParam)
   {
      int len = strlen(pStrParam);
      mpStringParam = new char[len + 1];
		
      if (mpStringParam)
         strcpy(mpStringParam, pStrParam);
   }
   else
   {
      mpStringParam = NULL;
   }
}

// Copy constructor
PingerMsg::PingerMsg(const PingerMsg& rPingerMsg)
:  OsMsg(rPingerMsg)
{
   mIntParam = rPingerMsg.mIntParam;

   mpStringParam = NULL;

   if (rPingerMsg.mpStringParam)
   {
      int len = strlen(rPingerMsg.mpStringParam);

      mpStringParam = new char[len+1];

      if (mpStringParam)
         strcpy(mpStringParam, rPingerMsg.mpStringParam);
   }
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* PingerMsg::createCopy(void) const
{
   return new PingerMsg(*this);
}

// Destructor
PingerMsg::~PingerMsg()
{
   if (mpStringParam)
   {
      delete[] mpStringParam;
      mpStringParam = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PingerMsg& 
PingerMsg::operator=(const PingerMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);       // assign fields for parent class

   mIntParam     = rhs.mIntParam;
   if (mpStringParam)
   {
      delete[] mpStringParam;
      mpStringParam = NULL;
   }

   if (rhs.mpStringParam)
   {
      int len = strlen(rhs.mpStringParam);

      mpStringParam = new char[len+1];

      if (mpStringParam)
         strcpy(mpStringParam, rhs.mpStringParam);
   }

   return *this;
}

// Set parameter1 of the phone set message
void PingerMsg::setIntParam(int param)
{
   mIntParam = param;
}

// Set string parameter of the phone set message
void PingerMsg::setStringParam(const char* param)
{
   if (param)
   {
      if (mpStringParam)
         delete[] mpStringParam;

      int len = strlen(param);

      mpStringParam = new char[len+1];

      if (mpStringParam)
         strcpy(mpStringParam, param);
   }
}

/* ============================ ACCESSORS ================================= */

// Return parameter1 of the message
int PingerMsg::getIntParam()
{
   return mIntParam;
}

// Return string parameter of the message
void PingerMsg::getStringParam(UtlString& stringData)
{
   stringData.remove(0);
   if (mpStringParam)
      stringData.append(mpStringParam);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

