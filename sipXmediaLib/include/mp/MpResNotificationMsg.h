//  
// Copyright (C) 2007-2009 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpResNotificationMsg_h_
#define _MpResNotificationMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "utl/UtlString.h"
#include "mp/MpTypes.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

  /// Message notification object used to communicate information from resources
  /// outward towards the flowgraph, and up through to users above mediaLib and beyond.
class MpResNotificationMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /// Phone set message types
   typedef enum
   {
      MPRNM_MESSAGE_INVALID, ///< Message type is invalid (similar to NULL)
      MPRNM_MESSAGE_ALL = MPRNM_MESSAGE_INVALID, ///< Select all message types (used in enabling/disabling)

      MPRNM_FROMFILE_STARTED,
      MPRNM_FROMFILE_PAUSED,
      MPRNM_FROMFILE_RESUMED,
      MPRNM_FROMFILE_STOPPED,
      MPRNM_FROMFILE_FINISHED,
      MPRNM_FROMFILE_ERROR,
      MPRNM_FROMFILE_PROGRESS,  ///< Value for MprnProgressMsg notifications.
      MPRNM_RECORDER_STARTED,   ///< Recording started.
      MPRNM_RECORDER_STOPPED,   ///< Recording stopped by manual command.
      MPRNM_RECORDER_FINISHED,  ///< Recording stopped automatically.
      MPRNM_RECORDER_ERROR,     ///< Recording stopped because of an error.
      MPRNM_DTMF_RECEIVED,      ///< Value for MprnDTMFMsg notifications.
      MPRNM_DELAY_SPEECH_STARTED,
      MPRNM_DELAY_NO_DELAY,
      MPRNM_DELAY_QUIESCENCE,
      MPRNM_RX_STREAM_ACTIVITY, ///< Value for MprnRtpStreamActivityMsg notifications.
      MPRNM_ENERGY_LEVEL,       ///< Audio energy level (MprnIntMsg)
      MPRNM_VOICE_STARTED,
      MPRNM_VOICE_STOPPED
   } RNMsgType;

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor
   MpResNotificationMsg(RNMsgType msgType, 
                        const UtlString& namedResOriginator,
                        MpConnectionID connId = MP_INVALID_CONNECTION_ID,
                        int streamId = -1);

   /// Copy constructor
   MpResNotificationMsg(const MpResNotificationMsg& rMpResNotifyMsg);

   /// Create a copy of this msg object (which may be of a derived type)
   virtual OsMsg* createCopy(void) const;

   /// Destructor
   virtual ~MpResNotificationMsg();

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /// Assignment operator
   MpResNotificationMsg& operator=(const MpResNotificationMsg& rhs);

   /// Set the name of the resource this message applies to.
   void setOriginatingResourceName(const UtlString& resOriginator);
   /**<
   *  Sets the name of the intended recipient for this message.
   */

   /// Set the connection ID that this notification is associated with.
   void setConnectionId(MpConnectionID connId);

   /// @brief Set the stream number inside the connection this notification
   /// is associated with.
   void setStreamId(int streamId);

   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

   /// Returns the type of the media resource notification message
   int getMsg() const;

   /// Get the name of the resource that originated this message.
   UtlString getOriginatingResourceName() const;
   /**<
   *  Returns the name of the MpResource object that originated this
   *  message.
   */

   /// Get the connection ID that this message is associated with.
   MpConnectionID getConnectionId() const;

   /// @brief Get the stream number inside the connection this notification
   /// is associated with.
   int getStreamId() const;

   //@}

   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlString mMsgOriginatorName; ///< Name of the resource that originated this message.
   MpConnectionID mConnectionId; ///< The ID of the connection this notification
                                 ///< is associated with.
   int mStreamId;                ///< The ID of the stream inside connection
                                 ///< this notification is associated with.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpResNotificationMsg_h_
