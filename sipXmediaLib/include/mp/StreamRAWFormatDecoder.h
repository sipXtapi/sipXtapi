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

#ifndef _StreamRAWFormatDecoder_h_
#define _StreamRAWFormatDecoder_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"

#include "mp/StreamQueueingFormatDecoder.h"
#include "os/OsStatus.h"
#include "os/OsTask.h"
#include "os/OsBSem.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:A simple RAW format Decoder
class StreamRAWFormatDecoder : public StreamQueueingFormatDecoder, public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{
   StreamRAWFormatDecoder(StreamDataSource* pDataSource);
     //:Default constructor


   virtual
   ~StreamRAWFormatDecoder();
     //:Destructor

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
   virtual OsStatus init();
     //:Initializes the decoder

   virtual OsStatus free();
     //:Frees all resources consumed by the decoder

   virtual OsStatus begin();
     //:Begins decoding

   virtual OsStatus end();
     //:Ends decoding

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   virtual OsStatus toString(UtlString& string);
     //:Renders a string describing this decoder.  
     // This is often used for debugging purposes.

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

   virtual UtlBoolean isDecoding();
     //:Gets the decoding status.  TRUE indicates decoding activity, false
     //:indicates the decoder has completed.

   virtual UtlBoolean validDecoder();
     //:Determines if this is a valid decoder given the associated data 
     //:source.
     // The data source is "peeked" for data, so that the stream's
     // data is not disturbed.


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   StreamRAWFormatDecoder(const StreamRAWFormatDecoder& rStreamRAWFormatDecoder);
     //:Copy constructor (not supported)

   StreamRAWFormatDecoder& operator=(const StreamRAWFormatDecoder& rhs);
     //:Assignment operator (not supported)

   int run(void* pArgs);
     //:Thread entry point

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlBoolean mbEnd ;       // Has the decoder completed?
   OsBSem    mSemExited ;  // Have we successfully exited?
};

/* ============================ INLINE METHODS ============================ */

#endif  // _StreamRAWFormatDecoder_h_

#endif
