//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 


#ifndef _MpRemoteVideoTask_h_
#define _MpRemoteVideoTask_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "os/OsMsg.h"
#include "mp/MpRtpBuf.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MprDejitter;
class MpdH264;
class MpvoGdi;
class OsTimer;

/// This thread display video stream, coming from remote party.
class MpRemoteVideoTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

   MpRemoteVideoTask(MprDejitter* pDejitter, void *hwnd=NULL);

   virtual
   ~MpRemoteVideoTask();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

   void setRemoteVideoWindow(const void *hwnd);

   void step();

   OsStatus stop();

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   void *getRemoteVideoWindow() const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   typedef enum
   {
      VIDEO_TICK
   } MsgType;

   MprDejitter *mpDejitter; ///< We will get RTP packet from this resource.
   MpdH264     *mpDecoder;  ///< Encoder for captured frames.
   MpvoGdi     *mpVideoOut; ///< Video output system.
   MpRtpBufPtr  mpRtpPacket;///< Storage for not consumed RTP packet between
                            ///< calls to step().
   UINT         mTimestamp; ///< Timestamp of previous packet.
   bool         mTimestampInitialized; ///< true, if we already initialized
                            ///< mTimestamp. I.e. we have received at least one
                            ///< RTP packet.

   OsTimer     *mpTimer;    ///< Timer for frame ticks.

     /// Handles an incoming message
   virtual
   UtlBoolean handleMessage(OsMsg& rMsg);
     /**<
     *  If the message is not one that the object is prepared to process,
     *  the handleMessage() method in the derived class should return FALSE
     *  which will cause the OsMessageTask::handleMessage() method to be
     *  invoked on the message.
     */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Main loop of the task.
   virtual int run(void* pArg);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpRemoteVideoTask_h_
