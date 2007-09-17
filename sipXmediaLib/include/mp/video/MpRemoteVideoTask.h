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
class MpVideoDecoder;
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

     /// Constructor
   MpRemoteVideoTask(MprDejitter* pDejitter, void *hwnd=NULL);

     /// Destructor
   virtual
   ~MpRemoteVideoTask();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Set window handle for remote video (Asynchronous).
   void setRemoteVideoWindow(const void *hwnd);

     /// Start processing incoming video stream. (Synchronous)
   OsStatus startProcessing();
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success.
     *  @returns <b>OS_FAILED</b> - Something gone bad.
     */

     /// Stop processing incoming video stream. (Synchronous)
   OsStatus stopProcessing();
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success.
     *  @returns <b>OS_FAILED</b> - Something gone bad.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Return remote video display handle.
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
      SET_REMOTE_VIDEO_WINDOW  ///< Message type for setRemoteVideoWindow().
   } MsgType;

   MprDejitter    *mpDejitter; ///< We will get RTP packet from this resource.
   MpVideoDecoder *mpDecoder;  ///< Encoder for captured frames.
   MpvoGdi        *mpVideoOut; ///< Video output system.
   MpRtpBufPtr    mpRtpPacket;///< Storage for not consumed RTP packet between
                            ///< calls to step().
   UINT           mTimestamp; ///< Timestamp of previous packet.
   bool           mStreamInitialized; ///< true, if we already initialized
                            ///< mTimestamp. I.e. we have received at least one
                            ///< RTP packet.

   OsTimer        *mpTimer;    ///< Timer for frame ticks.

     /// Handles an incoming message
   virtual
   UtlBoolean handleMessage(OsMsg& rMsg);
     /**<
     *  If the message is not one that the object is prepared to process,
     *  the handleMessage() method in the derived class should return FALSE
     *  which will cause the OsMessageTask::handleMessage() method to be
     *  invoked on the message.
     */

     /// Handler for SET_REMOTE_VIDEO_WINDOW message.
   OsStatus handleSetRemoteVideoWindow(const void *hwnd);

     /// Handler for frame tick message.
   OsStatus handleFrameTick();
     /**<
     *  This handler will be called when timer message arrive. All frame
     *  processing is done here.
     */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpRemoteVideoTask_h_
