//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
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
// APPLICATION INCLUDES
#include "mp/MpBufferMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Message object used to enqueue and dequeue media processing buffers

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpBufferMsg::MpBufferMsg(int msg, const MpBufPtr &pBuffer)
:  OsMsg(OsMsg::MP_BUFFER_MSG, msg)
{
   setBuffer(pBuffer);
}

// Copy constructor
MpBufferMsg::MpBufferMsg(const MpBufferMsg& rMpBufferMsg)
:  OsMsg(rMpBufferMsg)
{
   mpBuffer = rMpBufferMsg.mpBuffer;
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* MpBufferMsg::createCopy(void) const
{
   return new MpBufferMsg(*this);
}

// Done with message, delete it or mark it unused
void MpBufferMsg::releaseMsg()
{
   mpBuffer.release();
   OsMsg::releaseMsg();
}

// Destructor
MpBufferMsg::~MpBufferMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
MpBufferMsg& 
MpBufferMsg::operator=(const MpBufferMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);       // assign fields for parent class

   mpBuffer = rhs.mpBuffer;

   return *this;
}

// Copy buffer to this message
void MpBufferMsg::setBuffer(const MpBufPtr &p)
{
   // Make a copy of passed buffer
   mpBuffer = p.clone();
}

// Own provided buffer
void MpBufferMsg::ownBuffer(MpBufPtr &p)
{
   // Make sure mpBuffer is empty.
   mpBuffer.release();

   // Own passed buffer
   mpBuffer.swap(p);
}

/* ============================ ACCESSORS ================================= */

// Return the type of the media task message
int MpBufferMsg::getMsg() const
{
   return OsMsg::getMsgSubType();
}

// Return pointer of the buffer message
MpBufPtr &MpBufferMsg::getBuffer()
{
   return mpBuffer;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

