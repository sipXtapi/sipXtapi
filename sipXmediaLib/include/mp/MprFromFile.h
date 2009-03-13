//  
// Copyright (C) 2006-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprFromFile_h_
#define _MprFromFile_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpAudioResource.h"
#include "os/OsProtectEvent.h"
#include "mp/MpResourceMsg.h"
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
*  @brief The "Play audio from file" media processing resource
*/
class MprFromFile : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprFromFile(const UtlString& rName);

     /// Destructor
   virtual
   ~MprFromFile();

   typedef enum
   {
      PLAY_FINISHED,
      PLAY_STOPPED,
      PLAYING,
      READ_ERROR,
      PLAY_IDLE,
      INVALID_SETUP
   } Completion;

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

      /// Play sound from buffer w/repeat option
    static OsStatus playBuffer(const UtlString& namedResource, 
                               OsMsgQ& fgQ, 
                               const char* audioBuffer, 
                               unsigned long bufSize, 
                               uint32_t inRate, uint32_t fgRate, 
                               int type, UtlBoolean repeat, 
                               OsProtectedEvent* notify,
                               UtlBoolean autoStopAfterFinish = TRUE);
      /**<
      *  @param[in] fgRate - the sample rate that the flowgraph is running at
      *             (this cannot determine that because it is a static method)
      *  @param[in] type - can be one of following:  (need a OsSoundType)<br>
      *             0 = RAW<br>
      *             1 = muLaw
      *  @param[in] repeat - TRUE/FALSE after the fromFile reaches the end of
      *             the buffer, go back to the beginning and continue to play.
      *  @param[in] autoStopAfterFinish - if set to TRUE, resource will
      *             automatically transition to IDLE state immediately after
      *             FINISHED state is reached. Otherwise you need to call
      *             stopFile() to transition to IDLE state.
      *
      *  @returns the result of attempting to queue the message to this
      *           resource and/or converting the audio buffer data.
      */

     /// @brief Sends an MPRM_FROMFILE_START message to the named MprFromFile resource.
   static OsStatus playFile(const UtlString& namedResource,
                            OsMsgQ& fgQ,
                            uint32_t fgSampleRate,
                            const UtlString& filename,
                            const UtlBoolean& repeat,
                            UtlBoolean autoStopAfterFinish = TRUE);
     /**<
     *  Sends an MPRM_FROMFILE_START message to the named MprFromFile resource
     *  within the flowgraph who's queue is supplied. When the message 
     *  is received, the above resource will then begin playing the file
     *  specified.
     *
     *  @param[in]  namedResource - the name of the resource to send a message to.
     *  @param[in]  fgQ - the queue of the flowgraph containing the resource which
     *              the message is to be received by.
     *  @param[in]  fgSampleRate - flowgraph sample rate -- needed to check if 
     *              read file is compatible with the flowgraph rate, and for 
     *              resampling.
     *  @param[in]  filename - the filename of the file to start playing.
     *  @param[in]  repeat - boolean indicating whether to loop-play the file.
     *  @param[in]  autoStopAfterFinish - if set to TRUE, resource will
     *              automatically transition to IDLE state immediately after
     *              FINISHED state is reached. Otherwise you need to call
     *              stopFile() to transition to IDLE state.
     *
     *  @retval The result of attempting to queue the message to this resource.
     */

     /// @brief Sends an MPRM_FROMFILE_STOP message to the named MprFromFile resource.
   static OsStatus stopFile(const UtlString& namedResource,
                            OsMsgQ& fgQ);
     /**<
     *  Sends an MPRM_FROMFILE_STOP message to the named MprFromFile resource
     *  within the flowgraph who's queue is supplied. When the message 
     *  is received, the above resource will then stop playing the file
     *  it has been playing.
     *
     *  @param[in] namedResource - the name of the resource to send a message to.
     *  @param[in] fgQ - the queue of the flowgraph containing the resource which
     *  the message is to be received by.
     *  @returns the result of attempting to queue the message to this resource.
     */

     /// @brief Sends an MPRM_FROMFILE_PAUSE message to the named MprFromFile resource.
   static OsStatus pauseFile(const UtlString& namedResource,
                             OsMsgQ& fgQ);
     /**<
     *  Sends an MPRM_FROMFILE_PAUSE message to the named MprFromFile resource
     *  within the flowgraph who's queue is supplied. When the message 
     *  is received, the above resource will then pause the file that 
     *  has been playing.
     *
     *  @param[in] namedResource - the name of the resource to send a message to.
     *  @param[in] fgQ - the queue of the flowgraph containing the resource which
     *  the message is to be received by.
     *  @returns the result of attempting to queue the message to this resource.
     */

     /// @brief Sends an MPRM_FROMFILE_RESUME message to the named MprFromFile resource.
   static OsStatus resumeFile(const UtlString& namedResource,
                             OsMsgQ& fgQ);
     /**<
     *  Sends an MPRM_FROMFILE_RESUME message to the named MprFromFile resource
     *  within the flowgraph who's queue is supplied. When the message 
     *  is received, the above resource, if in a paused state, will then resume 
     *  playing it's buffer.
     *
     *  @param[in] namedResource - the name of the resource to send a message to.
     *  @param[in] fgQ - the queue of the flowgraph containing the resource which
     *  the message is to be received by.
     *  @returns the result of attempting to queue the message to this resource.
     */

     /// @brief Sends an MPRM_FROMFILE_SEND_PROGRESS message to the named MprFromFile resource.
   static OsStatus sendProgressPeriod(const UtlString& namedResource,
                                      OsMsgQ& fgQ,
                                      int32_t updatePeriodMS);
     /**<
     *  Sends an MPRM_FROMFILE_SEND_PROGRESS message to the named MprFromFile resource
     *  within the flowgraph who's queue is supplied. When the message 
     *  is received, the above resource will then begin to send progress updates
     *  at the interval specified by \p updatePeriodMS.
     *  
     *  To stop progress updates, you must call this function again with an 
     *  update period of 0.
     *
     *  @param[in] namedResource - the name of the resource to send a message to.
     *  @param[in] fgQ - the queue of the flowgraph containing the resource which
     *  the message is to be received by.
     *  @param[in] updatePeriodMS - the period to wait between sending progress updates.
     *  @returns the result of attempting to queue the message to this resource.
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

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   typedef enum
   {
      MPRM_FROMFILE_START = MpResourceMsg::MPRM_EXTERNAL_MESSAGE_START,
                                ///< Start playing.
      MPRM_FROMFILE_PAUSE,      ///< Pause playing.
      MPRM_FROMFILE_RESUME,     ///< Resume playing that was paused.
      MPRM_FROMFILE_SEND_PROGRESS, ///< Set progress updates period.
      MPRM_FROMFILE_STOP,       ///< Stop playing.
      MPRM_FROMFILE_ERROR       ///< Report failed operation
   } AddlMsgTypes;

   typedef enum
   {
      STATE_IDLE,      ///< Playing is stopped
      STATE_PLAYING,   ///< Playing is in process
      STATE_PAUSED,    ///< Playing is in process, but paused
      STATE_FINISHED   ///< Playing has finished, but stopFile() hasn't called yet
   } State;

   static const unsigned int sFromFileReadBufferSize;

   UtlString* mpFileBuffer;
   int mFileBufferIndex;
   UtlBoolean mFileRepeat;
   State mState;
   UtlBoolean mAutoStopAfterFinish; ///< If set to TRUE, resource will automatically
                                ///< transition to IDLE state immediately after
                                ///< FINISHED state is reached. Otherwise you need
                                ///< to call stopFile() to transition to IDLE state.

   OsTime mLastProgressUpdate;
   int32_t mProgressIntervalMS; ///< The progress interval for sending update
                                ///< notifications. A progress interval of 0
                                ///< means send no progress updates.

     /// @brief Convert generic audio data into flowgraph audio data.
   static OsStatus genericAudioBufToFGAudioBuf(
                                             UtlString*& fgAudioBuf,
                                             const char* audioBuffer, 
                                             unsigned long bufSize, 
                                             uint32_t inRate,
                                             uint32_t fgRate,
                                             int type);
     /**<
     *  Method to convert a generic char* audio buffer, in one of several
     *  acceptable formats, to a format that can be passed through the 
     *  flowgraph.
     *  
     *  @param[out] fgAudioBuf - a pointer to a string to store a newly allocated
     *              string with the processed audio suitable for the flowgraph.
     *  @param[in] audioBuffer - the audio data to re-work for the flowgraph.
     *  @param[in] bufSize - the size of /p audioBuffer, in bytes.
     *  @param[in] inRate - the sample rate that /p audioBuffer was recorded at.
     *  @param[in] fgRate - the sample rate that the flowgraph is running at
     *             (this cannot determine that because it is a static method)
     *  @param[in] type - can be one of following:  (need a OsSoundType)<br>
     *  0 = RAW<br>
     *  1 = muLaw
     */

     /// Read in an audio file into a new UtlString audio buffer.
   static OsStatus readAudioFile(uint32_t fgSampleRate,
                                 UtlString*& audioBuffer,
                                 const char* audioFileName);
     /**<
     *  @param audioBuffer - a reference to a pointer that will be filled
     *   with a new buffer holding the audio data.  Ownership will then
     *   transfer to the caller.
     *
     *  @NOTE WARNING: This allocates a buffer for the whole file -- thus,
     *                 files read in should not be huge.  This does occur
     *                 outside of the media task, so the time it takes to 
     *                 resample should not incur any latency, but memory 
     *                 utilization with large files may become an issue.
     *  @NOTE TODO: Replace this whole file reading code with some that 
     *              happens in a separate file-reading thread that can 
     *              happen while mediaTask is going on, so files of 
     *              extremely large length can be used.
     *  
     *  @param[in]  fgSampleRate - flowgraph sample rate -- needed to check if 
     *              read file is compatible with the flowgraph rate, and for 
     *              resampling.
     *
     *  @param[in] audioFileName - the name of a file to read flowgraph
     *   audio data from.  (exact format that the FG will accept -
     *   sample size, rate, & number of channels)
     *
     *  @retval OS_INVALID_ARGUMENT if the filename was null,
     *  the file was unopenable, or the file contained less than one sample.
     *  @retval OS_SUCCESS if the file was read successfully.
     */

     /// @brief allocate enough space for the resampled data, and resample data passed in.
   static UtlBoolean allocateAndResample(const char* inAudBuf, 
                                         const uint32_t inAudBufSz, 
                                         const uint32_t inRate, 
                                         char*& outAudBuf, 
                                         uint32_t& outAudBufSz, 
                                         const uint32_t outRate);
     /**<
     *  Allocate enough space to store a resampled version of the audio passed in
     *  as /p audBuf.  Indicate the sample rate of /p audBuf with /p inRate,
     *  and resample it to a rate of /p outRate.
     *  
     *  @param[in] inAudBuf - the audio buffer to resample - assumes 16bit signed.
     *  @param[in] inAudBufSz - the size of the audio buffer, in bytes.
     *  @param[in] inRate - the sample rate that audBuf was recorded at.
     *  @param[out] outAudBuf - the audio buffer to allocate and store resampled
     *              audio into. (16bit signed)
     *  @param[out] outAudBufSz - the place to store the size of the resampled 
     *              audio buffer, in bytes.
     *  @param[in] outRate - the sample rate that audBuf will be resampled to.
     */

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond);

     /// Initialize things to start playing the given buffer, upon receiving request to start.
   UtlBoolean handlePlay(UtlString* pBuffer, UtlBoolean repeat,
                         UtlBoolean autoStopAfterFinish);

     /// Handle playback finish when the end of file/buffer is reached.
   UtlBoolean handleFinish();

     /// Perform resetting of state, etc. upon receiving request to stop playing.
   UtlBoolean handleStop();

     /// Pause playback upon receiving request to pause.
   UtlBoolean handlePause();

     /// Resume playback upon receiving request to resume.
   UtlBoolean handleResume();

     /// Set an update period for sending progress updates.
   UtlBoolean handleSetUpdatePeriod(int32_t periodMS);

     /// Handle resource messages for this resource (new messaging model - 2007).
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// Copy constructor (not implemented for this class)
   MprFromFile(const MprFromFile& rMprFromFile);

     /// Assignment operator (not implemented for this class)
   MprFromFile& operator=(const MprFromFile& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprFromFile_h_
