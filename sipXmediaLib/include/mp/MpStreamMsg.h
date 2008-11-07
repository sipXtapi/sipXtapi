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
   MpStreamMsg(int msg, UtlString& target, StreamHandle handle, void* pPtr1=NULL,
               void* pPtr2=NULL, intptr_t int1=-1, intptr_t int2=-1);

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
   void setInt1(intptr_t i);

     /// Sets integer 2 of the stream message
   void setInt2(intptr_t i);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Returns the type of the stream  message
   int getMsg() const;
   
     /// Return the target id of the stream message
   UtlString getTarget() const;

     /// Return stream handle of stream msg
   StreamHandle getHandle() const;

     /// Return pointer 1 (void*) of the stream message
   void* getPtr1() const;

     /// Return pointer 2 (void*) of the stream message
   void* getPtr2() const;

     /// Return integer 1 of the media stream message
   intptr_t getInt1() const;

     /// Return integer 2 of the media stream message
   intptr_t getInt2() const;

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
   intptr_t     mInt1;     ///< Message integer 1
   intptr_t     mInt2;     ///< Message integer 2

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpStreamMsg_h_
