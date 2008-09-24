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
   typedef enum
   {
      AUD_RECORDED,  ///< Captured audio frame.
      AUD_RTP_RECV,
      AUD_RTCP_RECV,
      AUD_PLAY,
      ACK_EOSTREAM,
      VIDEO_FRAME    ///< Captured video frame.
   } MpBufferMsgType;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpBufferMsg(int msg, const MpBufPtr &pBuffer=MpBufPtr());

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

     /// Copy buffer to this message
   void setBuffer(const MpBufPtr &p);
     /**<
     *  Note, that this function create copy of the buffer! To avoid copying
     *  use ownBuffer() function. Copying is needed to avoid racing conditions
     *  in MpBuf::detach() code.
     */

     /// Own provided buffer
   void ownBuffer(MpBufPtr &p);
     /**<
     *  This function may be used to avoid buffer copying when passing buffer
     *  to message object. It owns provided buffer and invalidates it, i.e.
     *  after execution of this function MpBufPtr::isValid() will return false
     *  for passed buffer pointer.
     *
     *  Note, that buffer will not be copied only if MpBufPtr::isWritable()
     *  returns true for passed pointer. Copying is needed to avoid racing
     *  conditions in MpBuf::detach() code.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Return the type of the buffer message
   int getMsg() const;

     /// Return buffer object pointer from the buffer message
   MpBufPtr &getBuffer();

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   MpBufPtr mpBuffer; ///< Carried buffer

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpBufferMsg_h_
