//  
// Copyright (C) 2006-2019 SIPez LLC.  All rights reserved.
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
#include "utl/CircularBufferPtr.h"
#include "os/OsProtectEvent.h"

// DEFINES
// Note: only tested with 1-4 channels
#ifndef MAXIMUM_RECORDER_CHANNELS
#  define MAXIMUM_RECORDER_CHANNELS 4
#endif

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class MpEncoderBase;
class MpResamplerBase;
struct SipxOpusWriteObject;
struct OpusHead;

/// The "Recorder" media processing resource
class MprRecorder : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    /// These match the WAVE compression format codes in the RIFF header for convenience
   typedef enum {
      UNINITIALIZED_FORMAT = -1,
      RAW_PCM_16 = 0,

      // WAV codec codes:
      WAV_PCM_16 = 1,
      WAV_ALAW = 6,
      WAV_MULAW = 7,
      WAV_GSM = 49, // 0x31

      // Ogg codecs:
      OGG_OPUS = 501
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
                             int silenceLength = -1,
                             UtlBoolean append = FALSE,
                             int numChannels = 1);
     /**<
     *  @param[in] namedResource - resource name to send command to.
     *  @param[in] fgQ - flowgraph queue to send command to.
     *  @param[in] filename - filename to record to.
     *  @param[in] recFormat - output format type (e.g. WAV_PCM_16)
     *  @param[in] time - max number of milliseconds to record, or 0 for no limit
     *  @param[in] silenceLength - length of silence (in milliseconds) after
     *             which recording is automatically stopped. This feature is
     *             disabled if -1 is passed.
     *  @param[in] append - TRUE/FALSE if recording file exist, file should be 
     *             appended as opposed to replaced.  NOTE: It is an error 
     *             condition to append if the record format is not the same as 
     *             the original file.  Note: when stopping and starting in rapid
     *             succession, application SHOULD wait for stop notification before
     *             starting and appending recording to the same file or there is risk
     *             of the multiple file descriptors being open on the same file with
     *             unpredictable results.
     *  @param[in] numChannels - the number of channels to record (>1 not supported for all formats)
     *
     *  @returns OS_SUCCESS if file was successfully opened (and if append, format is the same)
     *           OS_FAILED if existing file was of a different audio format than requested or
     *                     unable to determine existing file audio format, or error opening
     *                     file for read and write.
     */

     /// Start recording to a buffer with given parameters.
   static OsStatus startBuffer(const UtlString& namedResource, 
                               OsMsgQ& fgQ,
                               MpAudioSample *pBuffer,
                               int bufferSize,
                               int time = 0,
                               int silenceLength = -1,
                               int numChannels = 1);
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
     *  @param[in] numChannels - the number of channels to record (>1 not supported for all formats)
     */

     /// Pause recording if it is already recording.
   static OsStatus pause(const UtlString& namedResource, OsMsgQ& flowgraphQueue);
     /**<
     *  @param[in] namedResource - resource name to send command to.
     *  @param[in] flowgraphQueue - flowgraph queue to send command to.
     */

     /// Resume recording if it is already recording.
   static OsStatus resume(const UtlString& namedResource, OsMsgQ& flowgraphQueue);
     /**<
     *  @param[in] namedResource - resource name to send command to.
     *  @param[in] flowgraphQueue - flowgraph queue to send command to.
     */

     /// Start recording to a circular-buffer with given parameters.
   static OsStatus startCircularBuffer(const UtlString& namedResource, 
                                       OsMsgQ& fgQ,
                                       CircularBufferPtr & buffer,
                                       RecordFileFormat recordingFormat,
                                       unsigned long recordingBufferNotificationWatermark,
                                       int numChannels = 1);
     /**<
     *  @param[in] namedResource - resource name to send command to.
     *  @param[in] fgQ - flowgraph queue to send command to.
     *  @param[in] buffer - circular buffer store the samples into.
     *  @param[in] recordingBufferNotificationWatermark - buffer usage level (in samples) to send notifications for.
     *  @param[in] numChannels - the number of channels to record (>1 not supported for all formats)
     */

     /// Stop recording if it has not stopped automatically yet.
   static OsStatus stop(const UtlString& namedResource, OsMsgQ& fgQ);
     /**<
     *  @param[in] namedResource - resource name to send command to.
     *  @param[in] fgQ - flowgraph queue to send command to.
     */

//@}

   static OsStatus validateOpusHeader(int inFileFd, OpusHead& opusHeader);

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
      MPRM_PAUSE,
      MPRM_RESUME,
      MPRM_START_CIRCULAR_BUFFER,
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
      STATE_PAUSED    ///< Recording is running, but paused
          
   } State;

   typedef enum
   {
      TO_UNDEFINED = -1,
      TO_FILE,           ///< Record to a file.
      TO_BUFFER,         ///< Record to a buffer.
      TO_CIRCULAR_BUFFER ///< Record to a buffer.
   } RecordDestination;

   typedef enum
   {
       NO_INTERLACE,
       PRE_ENCODE_INTERLACE,
       POST_ENCODE_INTERLACE
   } SampleInterlaceStage;

   State mState;            ///< Internal recorder state.
   RecordDestination mRecordDestination; ///< Where to store recorded samples.
   int mChannels;
   int mFramesToRecord;
   int mNumFramesProcessed;
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
   int mEncodedFrames;      ///< number of audio (flowgraph) frames encoded
   int mLastEncodedFrameSize;///< Size in bytes of last encoded frame recorded

   void* mpOpusEncoder;
   void* mpOpusComments;
   struct SipxOpusWriteObject* mpOpusStreamObject;

   SampleInterlaceStage mWhenToInterlace;

   MpResamplerBase* mpResampler; ///< Resampler for encoding to file

   
///@name Circular buffer related variables
//@{
   CircularBufferPtr * mpCircularBuffer;
   unsigned long mRecordingBufferNotificationWatermark;
//@}

   int mSamplesPerLastFrame; ///< Cache frame size of last processed buffer
   int mSamplesPerSecond;    ///< Cache sample rate of last processed buffer

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame,
                                    int samplesPerSecond);

     /// Handle MPRM_START_FILE message.
   UtlBoolean handleStartFile(int file, 
                              RecordFileFormat recFormat, 
                              int time,
                              int silenceLength,
                              UtlBoolean append,
                              int numChannels);
   /**
    * @param[in] file - file handle to record to
    * @param[in] recFormat - audio format to use for record file
    * @param[in] time - maximum record length in milliseconds (0 means unlimited)
    * @param[in] silenceLength - stop recording after this amount of silence in
    *            milliseconds (-1 means ignore silence and keep recording).
    * @param[in] append - file is being appended 
    * @param[in] numChannels - channels to record to file
    */

     /// Handle MPRM_START_BUFFER message.
   UtlBoolean handleStartBuffer(MpAudioSample *pBuffer, int bufferSize, int time,
                                int silenceLength, int numChannels);

     /// Handle MPRM_START_CIRCULAR_BUFFER message.
   UtlBoolean handleStartCircularBuffer(CircularBufferPtr * buffer, 
                                        RecordFileFormat recordingFormat,
                                        unsigned long recordingBufferNotificationWatermark,
                                        int numChannels);

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
   void closeFile(const char* fromWhereLabel);

   typedef int (MprRecorder::*WriteMethod)(const char * channelBuffers[], int);

     /// Write silence to the file
   inline int writeFileSilence(int numSamples);

     /// Write given data to the specified target
   inline int writeSamples(const MpAudioSample *pBuffer[], int numSamples, WriteMethod writeMethod);

     /// Write silence to the buffer
   inline int writeBufferSilence(int numSamples);

     /// Write silence to the circular buffer
   inline int writeCircularBufferSilence(int numSamples);

     /// Write given speech data to the buffer
   inline int writeBufferSpeech(const MpAudioSample *pBuffer, int numSamples);

     /// Write out standard 16bit WAV Header
   static UtlBoolean writeWaveHeader(int handle, RecordFileFormat format, 
                                     uint32_t samplesPerSecond = 8000,
                                     int16_t numChannels = 1);

   /// Read wave header info
   static OsStatus readWaveHeader(int fileHandle,
                                  RecordFileFormat& format,
                                  uint16_t& samplesPerSecond,
                                  uint16_t& channels);
   /**
    *  @param[in] fileHandle - wave file handle opened for read and positioned at begining of file
    *  @param[out] format - wave file compression codec id
    *  @param[out] samplesPerSecond - samples per second for encoding per wave header
    *  @param[out] channels - number of channels of audio per wave header
    *
    *  @returns OS_SUCCESS if wave header was read successfully and it was valid,
    *           OS_FAILED if the file is empty,
    *           OS_INVALID if the wave header was invalid (this could be a raw audio file)
    **/

     /// Update WAV-file's header with correct recorded length.
   static UtlBoolean updateWaveHeaderLengths(int handle, RecordFileFormat format);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor (not implemented for this class)
   MprRecorder(const MprRecorder& rMprRecorder);

     /// Assignment operator (not implemented for this class)
   MprRecorder& operator=(const MprRecorder& rhs);

   int writeFile(const char* channelData[], int dataSize);
   int writeCircularBuffer(const char* channelData[], int dataSize);
   void notifyCircularBufferWatermark();
   void createEncoder(const char * mimeSubtype, unsigned int codecSampleRate);
   void prepareEncoder(RecordFileFormat recFormat, unsigned int & codecSampleRate);
   OsStatus createOpusEncoder(int channels,
                              const char* artist, 
                              const char* title);
   void deleteOpusEncoder();

   static int16_t getBytesPerSample(RecordFileFormat format);
   static int interlaceSamples(const char* samplesArrays[], int samplesPerChannel, int bytesPerSample, int channels, char* interlacedChannelSamplesArray, int interlacedArrayMaximum);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprRecorder_h_
