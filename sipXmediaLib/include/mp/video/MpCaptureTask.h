//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 


#ifndef _MpCaptureTask_h_
#define _MpCaptureTask_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsTask.h"
#include "os/OsMsgQ.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MprToNet;
class MpVideoEncoder;
class MpRemoteVideoTask;
class MpVideoStreamParams;

/// @brief This task get frames from message queue (coming from capture device),
/// encode it and send to RtpWriter.
class MpCaptureTask : public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

   MpCaptureTask(OsMsgQ *pMsgQ, MprToNet *pRtpWriter, MpRemoteVideoTask *pTimer, const MpVideoStreamParams* pCaptureParams);

   virtual
   ~MpCaptureTask();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

   OsStatus stop();

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsMsgQ            *mpMsgQueue;  ///< Queue of messages carrying captured data.
   MprToNet          *mpRtpWriter; ///< Sink for generated RTP frames.
   MpVideoEncoder    *mpEncoder;   ///< Encoder for captured frames.

   MpRemoteVideoTask *mpTimer; ///< Hack to generate timer ticks for remote video renderer.

     /// Main loop of the task.
   virtual int run(void* pArg);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpCaptureTask_h_
