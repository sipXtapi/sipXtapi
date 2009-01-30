//  
// Copyright (C) 2006-2009 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MprRecorder_h_
#define _MprRecorder_h_

// SYSTEM INCLUDES
#ifdef _VXWORKS /* [ */
#   include <ioLib.h>
#endif /* _VXWORKS ] */

#if defined(_WIN32) && !defined(WINCE) /* [ */
#   include <io.h>
#endif /* _WIN32 ] */

// APPLICATION INCLUDES
#include "os/OsMutex.h"
#include "mp/MpResourceMsg.h"
#include "mp/MpAudioResource.h"
#include "os/OsProtectEvent.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

/// The "Recorder" media processing resource
class MprRecorder : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   typedef enum {
      UNINITIALIZED_FORMAT = -1,
      RAW_PCM_16 = 0,
      WAV_PCM_16
   } RecordFileFormat;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprRecorder(const UtlString& rName);

     /// Destructor
   virtual
   ~MprRecorder();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Start recording with given parameters.
   static OsStatus start(const UtlString& namedResource, 
                         OsMsgQ& fgQ,
                         const char *filename,
                         RecordFileFormat recFormat,
                         int time = 0,
                         int silenceLength = -1,
                         OsEvent* event = NULL);
     /**<
     *  @param[in] namedResource - resource name to send command to.
     *  @param[in] fgQ - flowgraph queue to send command to.
     *  @param[in] filename - filename to record to.
     *  @param[in] recFormat - output format type (e.g. WAV_PCM_16)
     *  @param[in] time - max number of milliseconds to record, or 0 for no limit
     *  @param[in] silenceLength - length of silence (in milliseconds) after
     *             which recording is automatically stopped. This feature is
     *             disabled if -1 is passed.
     *  @param[in] event - an optional OsEvent to signal on completion (DEPRECATED!).
     */

     /// Stop recording if it has not stopped automatically yet.
   static OsStatus stop(const UtlString& namedResource, OsMsgQ& fgQ);
     /**<
     *  @param[in] namedResource - resource name to send command to.
     *  @param[in] fgQ - flowgraph queue to send command to.
     */

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

   typedef enum
   {
      MPRM_START = MpResourceMsg::MPRM_EXTERNAL_MESSAGE_START,
      MPRM_STOP
   } AddlMsgTypes;

   typedef enum
   {
      FINISHED_AUTO,
      FINISHED_MANUAL,
      FINISHED_ERROR
   } FinishCause;

   typedef enum
   {
      STATE_IDLE,      ///< Recording is not running
      STATE_RECORDING, ///< Recording is running
      STATE_PAUSED     ///< Recording is running, but paused (NOT IMPLEMENTED YET!)
   } State;

   int mFileDescriptor;
   RecordFileFormat mRecFormat;
   uint32_t mTotalBytesWritten;
   uint32_t mTotalSamplesWritten;
   int mConsecutiveInactive;
   int mSilenceLength;
   OsEvent* mpEvent;
   int mFramesToRecord;
   State mState;            ///< Internal recorder state.

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
   MprRecorder(const MprRecorder& rMprRecorder);

     /// Assignment operator (not implemented for this class)
   MprRecorder& operator=(const MprRecorder& rhs);

     /// Handle MPRM_START message.
   UtlBoolean handleStart(int file, RecordFileFormat recFormat, int time,
                          int silenceLength, OsEvent* event);

     /// Handle MPRM_STOP message.
   UtlBoolean handleStop();

     /// @copydoc MpResource::handleDisable()
   virtual UtlBoolean handleDisable();

     /// Recording has been stopped with given cause.
   UtlBoolean finish(FinishCause cause);

     /// Write out standard 16bit 8k sampled WAV Header
   static UtlBoolean writeWAVHeader(int handle, unsigned long samplesPerSecond = 8000);

     /// Update WAV-file's header with correct recorded length.
   static UtlBoolean updateWaveHeaderLengths(int handle);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprRecorder_h_
