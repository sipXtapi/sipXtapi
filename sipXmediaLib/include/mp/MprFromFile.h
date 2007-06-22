//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
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
   MprFromFile(const UtlString& rName, int samplesPerFrame, int samplesPerSec);

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
                               OsNotification* notify);
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
                     OsNotification* event = NULL);
     /**<
     *  Note: if this resource is deleted before <I>stopFile</I> is called, it
     *  will close the file.
     *
     *  @param fileName - name of file from which to read raw audio data in
     *   exact format of the flowgraph (sample size, rate & number of channels).
     *
     *  @param repeat - TRUE/FALSE after the fromFile reaches the end of
     *   the file, go back to the beginning and continue to play.  Note this
     *   assumes that the file was opened for read.
     *
     *  @param event - an event to signal when state changes.  If NULL,
     *  nothing will be signaled.
     *
     *  @returns the result of attempting to queue the message to this
     *  resource and/or opening the named file.
     */

     /// @brief Sends an MPRM_FROMFILE_START message to the named MprFromFile resource.
   static OsStatus playFile(const UtlString& namedResource,
                            OsMsgQ& fgQ,
                            const UtlString& filename,
                            const UtlBoolean& repeat,
                            OsNotification* evt);
     /**<
     *  Sends an MPRM_FROMFILE_START message to the named MprFromFile resource
     *  within the flowgraph who's queue is supplied. When the message 
     *  is received, the above resource will then begin playing the file
     *  specified.
     *
     *  @param namedResource - the name of the resource to send a message to.
     *  @param fgQ - the queue of the flowgraph containing the resource which
     *  the message is to be received by.
     *  @param filename - the filename of the file to start playing.
     *  @param repeat - boolean indicating whether to loop-play the file.
     *  @param remote - boolean indicating whether to play this file remotely or locally.
     *  @returns the result of attempting to queue the message to this resource.
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
     *  @param namedResource - the name of the resource to send a message to.
     *  @param fgQ - the queue of the flowgraph containing the resource which
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
     *  @param namedResource - the name of the resource to send a message to.
     *  @param fgQ - the queue of the flowgraph containing the resource which
     *  the message is to be received by.
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
   } AddlMsgTypes;

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

     /// Read in an audio file into a new UtlString audio buffer.
   static OsStatus readAudioFile(UtlString*& audioBuffer,
                                 const char* audioFileName,
                                 OsNotification* notify);
     /**<
     *  @param audioBuffer - a reference to a pointer that will be filled
     *   with a new buffer holding the audio data.  Ownership will then
     *   transfer to the caller.
     *
     *  @param audioFileName - the name of a file to read flowgraph
     *   audio data from.  (exact format that the FG will accept -
     *   sample size, rate, & number of channels)
     *
     *  @param event - an event to signal when state changes.  If NULL,
     *  nothing will be signaled.
     *
     *  @returns OS_INVALID_ARGUMENT if the filename was null,
     *  the file was unopenable, or the file contained less than one sample.
     *  @returns OS_SUCCESS if the file was read succesfully.
     */

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

   virtual UtlBoolean handleSetup(MpFlowGraphMsg& rMsg);
   virtual UtlBoolean handleStop(void);

     /// Handle flowgraph messages for this resource.
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);

     /// Handle resource messages for this resource.
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// Copy constructor (not implemented for this class)
   MprFromFile(const MprFromFile& rMprFromFile);

     /// Assignment operator (not implemented for this class)
   MprFromFile& operator=(const MprFromFile& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprFromFile_h_
