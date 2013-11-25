//  
// Copyright (C) 2006-2013 SIPez LLC.  All rights reserved.
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
class MpEncoderBase;
class MpResamplerBase;

/// The "Recorder" media processing resource
class MprRecorder : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    /// These match the WAVE compression format codes in the RIFF header for convenience
   typedef enum {
      UNINITIALIZED_FORMAT = -1,
      RAW_PCM_16 = 0,
      WAV_PCM_16 = 1,
      WAV_GSM = 31
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

     /// Start recording to a file with given parameters.
   static OsStatus startFile(const UtlString& namedResource, 
                             OsMsgQ& fgQ,
                             const char *filename,
                             RecordFileFormat recFormat,
                             int time = 0,
                             int silenceLength = -1);
     /**<
     *  @param[in] namedResource - resource name to send command to.
     *  @param[in] fgQ - flowgraph queue to send command to.
     *  @param[in] filename - filename to record to.
     *  @param[in] recFormat - output format type (e.g. WAV_PCM_16)
     *  @param[in] time - max number of milliseconds to record, or 0 for no limit
     *  @param[in] silenceLength - length of silence (in milliseconds) after
     *             which recording is automatically stopped. This feature is
     *             disabled if -1 is passed.
     */

     /// Start recording to a buffer with given parameters.
   static OsStatus startBuffer(const UtlString& namedResource, 
                               OsMsgQ& fgQ,
                               MpAudioSample *pBuffer,
                               int bufferSize,
                               int time = 0,
                               int silenceLength = -1);
     /**<
     *  @param[in] namedResource - resource name to send command to.
     *  @param[in] fgQ - flowgraph queue to send command to.
     *  @param[in] pBuffer - memory buffer to record to.
     *  @param[in] bufferSize - \p pBuffer size (in samples).
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
      MPRM_START_FILE = MpResourceMsg::MPRM_EXTERNAL_MESSAGE_START,
      MPRM_START_BUFFER,
      MPRM_STOP
   } AddlMsgTypes;

   typedef enum
   {
      FINISHED_AUTO,   ///< Recording has finished automatically.
      FINISHED_MANUAL, ///< Recording has stopped manually.
      FINISHED_ERROR   ///< Recording has stopped because of error.
   } FinishCause;

   typedef enum
   {
      STATE_IDLE,      ///< Recording is not running
      STATE_RECORDING, ///< Recording is running
      STATE_PAUSED     ///< Recording is running, but paused (NOT IMPLEMENTED YET!)
   } State;

   typedef enum
   {
      TO_UNDEFINED = -1,
      TO_FILE,         ///< Record to a file.
      TO_BUFFER        ///< Record to a buffer.
   } RecordDestination;

   State mState;            ///< Internal recorder state.
   RecordDestination mRecordDestination; ///< Where to store recorded samples.
   int mFramesToRecord;
   int mSamplesRecorded;
   int mConsecutiveInactive;
   int mSilenceLength;

///@name File-related variables
//@{
   int mFileDescriptor;     ///< File descriptor to write to.
   RecordFileFormat mRecFormat; ///< Should data be written in WAV or RAW PCM format.
//@}

///@name Buffer-related variables
//@{
   MpAudioSample *mpBuffer; ///< Buffer to write data to. End of the buffer
                            ///< Is determined by mSamplesRecorded variable.
   int mBufferSize;         ///< mpBuffer size.
//@}

   MpEncoderBase* mpEncoder; ///< encoder for non-PCM formats saved to file

   MpResamplerBase* mpResampler; ///< Resampler for encoding to file

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame,
                                    int samplesPerSecond);

     /// Handle MPRM_START_FILE message.
   UtlBoolean handleStartFile(int file, RecordFileFormat recFormat, int time,
                              int silenceLength);

     /// Handle MPRM_START_BUFFER message.
   UtlBoolean handleStartBuffer(MpAudioSample *pBuffer, int bufferSize, int time,
                                int silenceLength);

     /// Handle MPRM_STOP message.
   UtlBoolean handleStop();

     /// @copydoc MpResource::handleDisable()
   virtual UtlBoolean handleDisable();

     /// Handle messages for this resource.
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// Prepare for recording.
   void startRecording(int time, int silenceLength);

     /// Recording has been stopped with given cause.
   UtlBoolean finish(FinishCause cause);

     /// Close file if it is opened and  update WAV header if needed.
   void closeFile();

     /// Write silence to the file
   inline int writeFileSilence(int numSamples);

     /// Write given speech data to the file
   inline int writeFileSpeech(const MpAudioSample *pBuffer, int numSamples);

     /// Write silence to the buffer
   inline int writeBufferSilence(int numSamples);

     /// Write given speech data to the buffer
   inline int writeBufferSpeech(const MpAudioSample *pBuffer, int numSamples);

     /// Write out standard 16bit WAV Header
   static UtlBoolean writeWAVHeader(int handle, RecordFileFormat format, uint32_t samplesPerSecond = 8000);

     /// Update WAV-file's header with correct recorded length.
   static UtlBoolean updateWaveHeaderLengths(int handle);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor (not implemented for this class)
   MprRecorder(const MprRecorder& rMprRecorder);

     /// Assignment operator (not implemented for this class)
   MprRecorder& operator=(const MprRecorder& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprRecorder_h_
