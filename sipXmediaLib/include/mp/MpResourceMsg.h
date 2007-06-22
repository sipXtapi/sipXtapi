//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpResourceMsg_h_
#define _MpResourceMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "utl/UtlString.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message object used to communicate with the media processing task
class MpResourceMsg : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

     /// Resource message types
   typedef enum
   {
      MPRM_RESOURCE_ENABLE,            ///< All resources:  Enable resource
      MPRM_RESOURCE_DISABLE,           ///< All resources: Disable resource
      MPRM_ENABLE_ALL_NOTIFICATIONS,   ///< All resources: Enable all notifications to be sent from resources
      MPRM_DISABLE_ALL_NOTIFICATIONS,  ///< All resources: Disable all notification from being sent from resources

      MPRM_START_TONE,          ///< MprToneGen: Start a tone.
      MPRM_STOP_TONE,           ///< MprToneGen: Stop a tone.
      MPRM_FROMFILE_START,      ///< MprFromFile: Start playing a file.
      MPRM_FROMFILE_PAUSE,      ///< MprFromFile: Pause a playing file.
      MPRM_FROMFILE_STOP,       ///< MprFromFile: Stop a playing file.

      MPRM_BUF_RECORDER_START,  ///< MprBufferRecorder: Start recording to a buffer.
      MPRM_BUF_RECORDER_STOP,   ///< MprBufferRecorder: Stop recording to a buffer.

      MPRM_BRIDGE_SET_GAINS,    ///< MprBridge: Set row or column in mix matrix.

      // Connection Resource messages
      MPRM_START_RECEIVE_RTP,   ///< MpRtpInputAudioConnection: Start receiving RTP.
      MPRM_STOP_RECEIVE_RTP,    ///< MpRtpInputAudioConnection: Stop receiving RTP.
      MPRM_START_SEND_RTP,      ///< MpRtpOutputAudioConnection: Start sending RTP.
      MPRM_STOP_SEND_RTP,       ///< MpRtpOutputAudioConnection: Stop sending RTP.

      // Add new built in resource operation messages above

      // Non-builtin resource messages
      MPRM_EXTERNAL_MESSAGE_START = 128
      // Do not add new message types after this
   } MpResourceMsgType;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpResourceMsg(MpResourceMsgType msg, const UtlString& msgDestName);

     /// Copy constructor
   MpResourceMsg(const MpResourceMsg& rMpResourceMsg);

     /// Create a copy of this msg object (which may be of a derived type)
   virtual OsMsg* createCopy(void) const;

     /// Destructor
   virtual
   ~MpResourceMsg();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
   MpResourceMsg& operator=(const MpResourceMsg& rhs);

     /// Set the name of the resource this message applies to.
   void setDestResourceName(const UtlString& msgDestName);
     /**<
     *  Sets the name of the intended recipient for this message.
     */
//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Returns the type of the media resource message
   int getMsg(void) const;

     /// Get the name of the resource this message applies to.
   UtlString getDestResourceName(void) const;
     /**<
     *  Returns the name of the MpResource object that is the intended 
     *  recipient for this message.
     */

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlString mMsgDestName; ///< Intended recipient for this message
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpResourceMsg_h_
