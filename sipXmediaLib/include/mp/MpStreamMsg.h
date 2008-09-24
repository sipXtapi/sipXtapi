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

#ifndef _MpStreamMsg_h_
#define _MpStreamMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlString.h>

#include "os/OsMsg.h"
#include "mp/StreamDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message object used to communicate with the media processing task
class MpStreamMsg : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /// Phone set message types
   typedef enum
   {
      STREAM_REALIZE_URL,
      STREAM_REALIZE_BUFFER,
      STREAM_PREFETCH,
      STREAM_PLAY,
      STREAM_REWIND,
      STREAM_PAUSE,
      STREAM_STOP,
      STREAM_DESTROY      
   } MpStreamMsgType;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpStreamMsg(int msg, UtlString& target, StreamHandle handle, void* pPtr1=NULL, void* pPtr2=NULL,
               int int1=-1, int int2=-1);

     /// Copy constructor
   MpStreamMsg(const MpStreamMsg& rMpStreamMsg);

     /// Create a copy of this msg object (which may be of a derived type)
   virtual OsMsg* createCopy() const;

     /// Destructor
   virtual
   ~MpStreamMsg();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
   MpStreamMsg& operator=(const MpStreamMsg& rhs);

     /// Sets the target id of the stream message
   void setTarget(UtlString& target);

     /// Sets the stream handle of the stream message
   void setHandle(StreamHandle handle);

     /// Sets pointer 1 (void*) of the stream message
   void setPtr1(void* p);

     /// Sets pointer 2 (void*) of the stream message
   void setPtr2(void* p);

     /// Sets integer 1 of the stream message
   void setInt1(int i);

     /// Sets integer 2 of the stream message
   void setInt2(int i);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Returns the type of the stream  message
   int getMsg(void) const;
   
     /// Return the target id of the stream message
   UtlString getTarget(void) const;

     /// Return stream handle of stream msg
   StreamHandle getHandle(void) const;

     /// Return pointer 1 (void*) of the stream message
   void* getPtr1(void) const;

     /// Return pointer 2 (void*) of the stream message
   void* getPtr2(void) const;

     /// Return integer 1 of the media stream message
   int getInt1(void) const;

     /// Return integer 2 of the media stream message
   int getInt2(void) const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:   
   UtlString    mTarget;   ///< Target ID
   StreamHandle mHandle;   ///< Stream Handle
   void*        mpPtr1;    ///< Message pointer 1
   void*        mpPtr2;    ///< Message pointer 2
   int          mInt1;     ///< Message integer 1
   int          mInt2;     ///< Message integer 2

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpStreamMsg_h_

#endif
