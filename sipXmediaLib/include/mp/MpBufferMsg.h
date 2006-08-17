//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MpBufferMsg_h_
#define _MpBufferMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "mp/MpBuf.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * @brief Message object used to communicate with the media processing task.
 */
class MpBufferMsg : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /// Phone set message types
   enum MpBufferMsgType
   {
      AUD_RECORDED,
      AUD_RTP_RECV,
      AUD_RTCP_RECV,
      AUD_PLAY,
      ACK_EOSTREAM
   };

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpBufferMsg(int msg, const MpBufPtr &pTag=MpBufPtr());

     /// Copy constructor
   MpBufferMsg(const MpBufferMsg& rMpBufferMsg);

     /// Create a copy of this msg object (which may be of a derived type)
   virtual OsMsg* createCopy() const;

     /// Done with message, delete it or mark it unused
   virtual void releaseMsg();

     /// Destructor
   virtual
   ~MpBufferMsg();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
   MpBufferMsg& operator=(const MpBufferMsg& rhs);

     /// Set buffer object pointer of the buffer message
   void setTag(const MpBufPtr &p);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Return the type of the buffer message
   int getMsg() const;

     /// Return buffer object pointer from the buffer message
   MpBufPtr &getTag();

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   MpBufPtr mpTag; ///< The descriptor of the buffers

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpBufferMsg_h_
