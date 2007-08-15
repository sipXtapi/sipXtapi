//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>


#ifndef _MprBufferRecorder_h_
#define _MprBufferRecorder_h_

// SYSTEM INCLUDES
#ifdef _VXWORKS /* [ */
#   include <ioLib.h>
#endif /* _VXWORKS ] */

#if defined(_WIN32) && !defined(WINCE) /* [ */
#   include <io.h>
#endif /* _WIN32 ] */

// APPLICATION INCLUDES
#include "os/OsMutex.h"
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpAudioResource.h"
#include "mp/MpResNotificationMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

/**
*  @brief Resource for recording to a buffer.
*
*  The MprBufferRecorder is a resource that records to a user-supplied buffer.  
*  To start recording, users call the static /c startRecording method, passing 
*  usual parameters of name and queue to post to, but also a UtlString* buffer 
*  that is written to while recording.
* 
*  The buffer recorder writes to the buffer until it is full, or until it 
*  receives a stop message.
*
*  This resource has inputs, but no outputs.  Output is dropped on the floor.
*/
class MprBufferRecorder : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprBufferRecorder(const UtlString& rName, 
                     int samplesPerFrame, 
                     int samplesPerSec);

     /// Destructor
   virtual
   ~MprBufferRecorder();

   typedef enum
   {
      RECORD_FINISHED,
      RECORD_STOPPED,
      RECORDING,
      RECORD_IDLE,
      NO_INPUT_DATA
   }Completion;

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     ///
   static OsStatus startRecording(const UtlString& namedResource, 
                                  OsMsgQ& fgQ, UtlString* buffer);

   static OsStatus stopRecording(const UtlString& namedResource, 
                                 OsMsgQ& fgQ);

   virtual UtlBoolean disable(Completion code);

   virtual UtlBoolean enable(void);

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
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   UtlString* mpBuffer;
   unsigned int mnBufferSamplesUsed;

   Completion mStatus;


     /// Record frames of data to the buffer if enabled.
   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

     /// Handle messages for this resource.
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// Copy constructor (not implemented for this class)
   MprBufferRecorder(const MprBufferRecorder& rMprBufRecorder);

     /// Private default constructor - noone should be calling this.
   MprBufferRecorder();

     /// Assignment operator (not implemented for this class)
   MprBufferRecorder& operator=(const MprBufferRecorder& rhs);

     /// Handle the message for starting recording
   UtlBoolean handleStartRecording(UtlString* buffer);

     /// Handle the message for stopping recording
   UtlBoolean handleStopRecording(void);

     /// Send a record done notification message to the flowgraph.
   OsStatus sendNotification(MpResNotificationMsg::RNMsgType type);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprBufferRecorder_h_
