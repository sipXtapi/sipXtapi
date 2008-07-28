// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
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

#ifdef MP_STREAMING

#ifndef _StreamQueueMsg_h_
#define _StreamQueueMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "os/OsMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:A StreamQueueMsg is a container for a frames worth of data.  These Msgs are
//:copied into the StreamQueueMsg to avoid dynamic memory allocation. Msgs are
//:designed to be pooled.
class StreamQueueMsg : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   typedef enum
   {
      AudioFrame,
      EndOfFrameMarker
   } StreamQueueMsgSubTypes;

/* ============================ CREATORS ================================== */
///@name Creators
//@{
   StreamQueueMsg() ;
     //:Constructor

   StreamQueueMsg(const StreamQueueMsg& rStreamQueueMsg);
     //:Copy constructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)

   virtual
      ~StreamQueueMsg();
     //:Destructor

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
   StreamQueueMsg& operator=(const StreamQueueMsg& rhs);
     //:Assignment operator

   void setSamples(const short* pSamples);
     //:Set the sample data for this message

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   UtlBoolean getSamples(short* pSamples) const ;
     //:Get the sample data for this message

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   short mSamples[80] ;

};

/* ============================ INLINE METHODS ============================ */

#endif  /* _StreamQueueMsg_h_ */

#endif
