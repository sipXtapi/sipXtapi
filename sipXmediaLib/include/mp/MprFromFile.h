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
*  @brief The "Audio from file" media processing resource
*/
class MprFromFile : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     //:Constructor
   MprFromFile(const UtlString& rName);

     //:Destructor
   virtual
   ~MprFromFile();

   typedef enum /// $$$ These need more thought and clarification...
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
    OsStatus playBuffer(const char* audioBuffer, unsigned long bufSize, 
                        int type, UtlBoolean repeat, OsProtectedEvent* notify);
      /**<
      *  @param type - can be one of following:  (need a OsSoundType)<br>
      *  0 = RAW<br>
      *  1 = muLaw
      *
      *  @param repeat - TRUE/FALSE after the fromFile reaches the end of
      *   the file, go back to the beginning and continue to play.  Note this
      *   assumes that the file was opened for read.
      *
      *  @returns the result of attempting to queue the message to this
      *  resource and/or opening the named file.
      */

      /// Play sound from buffer w/repeat option
    static OsStatus playBuffer(const UtlString& namedResource, 
                               OsMsgQ& fgQ, 
                               const char* audioBuffer, 
                               unsigned long bufSize, 
                               int type, UtlBoolean repeat, 
                               OsNotification* notify = NULL);
      /**<
      *  @param type - can be one of following:  (need a OsSoundType)<br>
      *  0 = RAW<br>
      *  1 = muLaw
      *
      *  @param repeat - TRUE/FALSE after the fromFile reaches the end of
      *   the buffer, go back to the beginning and continue to play.
      *
      *  @returns the result of attempting to queue the message to this
      *  resource and/or converting the audio buffer data.
      */

     /// Old Play from file w/file name and repeat option
   OsStatus playFile(const char* fileName, UtlBoolean repeat,
                     OsNotification* notify = NULL);
     /**<
     *  Note: if this resource is deleted before <I>stopFile</I> is called, it
     *  will close the file.
     *
     *  @param[in]  fileName - name of file from which to read raw audio data in
     *              exact format of the flowgraph (sample size, rate & number of
     *              channels).
     *  @param[in]  repeat - TRUE/FALSE after the fromFile reaches the end of
     *              the file, go back to the beginning and continue to play.  
     *              Note: This assumes that the file was opened for read.
     *  @param[in]  notify - an event to signal when state changes.  If NULL,
     *              nothing will be signaled.
     *  @retval the result of attempting to queue the message to this resource 
     *          and/or opening the named file.
     */

     /// @brief Sends an MPRM_FROMFILE_START message to the named MprFromFile resource.
   static OsStatus playFile(const UtlString& namedResource,
                            OsMsgQ& fgQ,
                            uint32_t fgSampleRate,
                            const UtlString& filename,
                            const UtlBoolean& repeat,
                            OsNotification* notify = NULL);
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
     *  @param[in]  notify - an event to signal when state changes.  If NULL,
     *              nothing will be signaled.
     *  @retval The result of attempting to queue the message to this resource.
     */

     /// Stop playing from file
   OsStatus stopFile(void);
     /**<
     *  Sends a STOP_FILE message to this resource to stop playing audio
     *  from file
     *
     *  @returns the result of attempting to queue the message to this
     *  resource.
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

   virtual UtlBoolean enable(void);
   virtual UtlBoolean disable(void);

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
      PLAY_FILE = MpFlowGraphMsg::RESOURCE_SPECIFIC_START,
      STOP_FILE
   } AddlFGMsgTypes;

   typedef enum
   {
      MPRM_FROMFILE_FINISH = MpResourceMsg::MPRM_EXTERNAL_MESSAGE_START
   } AddlResMsgTypes;

   typedef enum
   {
       PLAY_ONCE,
       PLAY_REPEAT
   } MessageAttributes;

   static const unsigned int sFromFileReadBufferSize;

   UtlString* mpFileBuffer;
   int mFileBufferIndex;
   UtlBoolean mFileRepeat;
   OsNotification* mpNotify;
   UtlBoolean mPaused;

   OsTime mLastProgressUpdate;

   /// The progress interval for sending update notifications.  
   int32_t mProgressIntervalMS; 
   /**<
   *  A progress interval of 0 means send no progress updates.
   */

     /// @brief Convert generic audio data into flowgraph audio data.
   static OsStatus genericAudioBufToFGAudioBuf(
                                             UtlString*& fgAudioBuf,
                                             const char* audioBuffer, 
                                             unsigned long bufSize, 
                                             int type);
     /**<
     *  Method to convert a generic char* audio buffer, in one of several
     *  acceptable formats, to a format that can be passed through the 
     *  flowgraph.
     *
     *  @param[in] type - can be one of following:  (need a OsSoundType)<br>
     *  0 = RAW<br>
     *  1 = muLaw
     *
     *  @param repeat - TRUE/FALSE after the fromFile reaches the end of
     *   the file, go back to the beginning and continue to play.  Note this
     *   assumes that the file was opened for read.
     *
     *  @returns the result of attempting to queue the message to this
     *  resource and/or opening the named file.
     */

     /// Read in an audio file into a new UtlString audio buffer.
   static OsStatus readAudioFile(uint32_t fgSampleRate,
                                 UtlString*& audioBuffer,
                                 const char* audioFileName,
                                 OsNotification* notify);
     /**<
     *  @param audioBuffer - a reference to a pointer that will be filled
     *   with a new buffer holding the audio data.  Ownership will then
     *   transfer to the caller.
     *
     *  @param[in]  fgSampleRate - flowgraph sample rate -- needed to check if 
     *              read file is compatible with the flowgraph rate, and for 
     *              resampling.
     *
     *  @param[in] audioFileName - the name of a file to read flowgraph
     *   audio data from.  (exact format that the FG will accept -
     *   sample size, rate, & number of channels)
     *
     *  @param[in] event - an event to signal when state changes.  If NULL,
     *  nothing will be signaled.
     *
     *  @retval OS_INVALID_ARGUMENT if the filename was null,
     *  the file was unopenable, or the file contained less than one sample.
     *  @retval OS_SUCCESS if the file was read successfully.
     */

   static UtlBoolean allocateAndResample(char*& audBuf, uint32_t& audBufSz, 
                                         uint32_t inRate, uint32_t outRate);

     /// @brief Sends a local MPRM_FROMFILE_FINISH message back to this resource.
   OsStatus finishFile();
     /**<
     *  Sends an MPRM_FROMFILE_FINISH message back to this resource. 
     *  When the message is received, this will then notify that it's finished, 
     *  and stop playing the file it has been playing.
     *
     *  @returns the result of attempting to queue the message to this resource.
     */

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame,
                                    int samplesPerSecond);

     /// Initialize things to start playing the given buffer, upon receiving request to start.
   virtual UtlBoolean handlePlay(OsNotification* pNotifier, UtlString* pBuffer,
                                 UtlBoolean repeat);

     /// Perform resetting of state, etc. upon receiving request to stop playing.
   virtual UtlBoolean handleStop(UtlBoolean finished = FALSE);

     /// Pause playback upon receiving request to pause.
   virtual UtlBoolean handlePause(void);

     /// Resume playback upon receiving request to resume.
   virtual UtlBoolean handleResume(void);

     /// Set an update period for sending progress updates.
   virtual UtlBoolean handleSetUpdatePeriod(int32_t periodMS);

     /// Handle flowgraph messages for this resource (old messaging model).
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);

     /// Handle resource messages for this resource (new messaging model - 2007).
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// Copy constructor (not implemented for this class)
   MprFromFile(const MprFromFile& rMprFromFile);

     /// Assignment operator (not implemented for this class)
   MprFromFile& operator=(const MprFromFile& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprFromFile_h_
