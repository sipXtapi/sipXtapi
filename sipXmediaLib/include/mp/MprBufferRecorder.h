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
   MprBufferRecorder(const UtlString& rName);

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

     /// Send message to start recording.
   static OsStatus startRecording(const UtlString& namedResource, OsMsgQ& fgQ,
                                  int ms, UtlString* buffer);
     /**<
     *  Recording is done to a passed \a buffer for \a ms milliseconds.
     *  Capacity of \a buffer is increased to hold full recording. Length of
     *  \a buffer is set initially to 0 and then increased after every recorded
     *  frame.
     *
     *  @param[in] ms - Number of milliseconds to record.
     *  @param[in] buffer - Buffer to record to.
     */

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
   unsigned int mSamplesToRecord;
   unsigned int mBufferSamplesUsed;

   Completion mStatus;


     /// Record frames of data to the buffer if enabled.
   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame,
                                    int samplesPerSecond);

     /// Handle messages for this resource.
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// Copy constructor (not implemented for this class)
   MprBufferRecorder(const MprBufferRecorder& rMprBufRecorder);

     /// Private default constructor - noone should be calling this.
   MprBufferRecorder();

     /// Assignment operator (not implemented for this class)
   MprBufferRecorder& operator=(const MprBufferRecorder& rhs);

     /// Handle the message for starting recording
   UtlBoolean handleStartRecording(int ms, UtlString* buffer);

     /// Handle the message for stopping recording
   UtlBoolean handleStopRecording(void);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprBufferRecorder_h_
