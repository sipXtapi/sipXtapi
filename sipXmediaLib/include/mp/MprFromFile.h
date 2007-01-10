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
//#include <stdio.h>

// APPLICATION INCLUDES
//#include "mp/dtmflib.h"
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


     /// Play from file w/file name and repeat option
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
     *  @returns the result of attempting to queue the message to this
     *  resource and/or opening the named file.
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

   UtlString* mpFileBuffer;
   int mFileBufferIndex;
   UtlBoolean mFileRepeat;
   OsNotification* mpNotify;

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

   virtual UtlBoolean handleSetup(MpFlowGraphMsg& rMsg);
   virtual UtlBoolean handleStop(void);

     /// Handle messages for this resource.
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);

     /// Copy constructor (not implemented for this class)
   MprFromFile(const MprFromFile& rMprFromFile);

     /// Assignment operator (not implemented for this class)
   MprFromFile& operator=(const MprFromFile& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprFromFile_h_
