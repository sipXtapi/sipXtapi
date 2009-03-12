//  
// Copyright (C) 2007-2009 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

#ifndef _MiNotification_h_
#define _MiNotification_h_

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

/**
*  @brief Message notification class used to communicate media notification events. 
*
*  To determine the source of the notification use the following information:
*  1) Test getConnectionId() - if it is >=1, then notification is sent from
*     the selected input or output connection. If it is equal to -1, then
*     notification is sent from local part (mic and speaker sides).
*  2) If notification belongs to input part of connection, then getStreamId()
*     will give you a number of stream in the connection (starting from 0).
*     In other cases (output part of connection or local flowgraph part)
*     stream ID equals -1.
*  3) getSourceId() returns the name of the resource which sent this notification.
*     Use this value to distinguish between several possible senders inside
*     one part of the flowgraph, e.g. mic part from speaker part.
*/
class MiNotification : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

     /// Media notification message types
   typedef enum
   {
      MI_NOTF_MESSAGE_INVALID, ///< Message type is invalid (similar to NULL)

      MI_NOTF_PLAY_STARTED,
      MI_NOTF_PLAY_PAUSED,
      MI_NOTF_PLAY_RESUMED,
      MI_NOTF_PLAY_STOPPED,
      MI_NOTF_PLAY_FINISHED,
      MI_NOTF_PLAY_ERROR,
      MI_NOTF_PROGRESS,           ///< Value for MiProgressNotf notifications.
      MI_NOTF_RECORD_STARTED,     ///< Recording started.
      MI_NOTF_RECORD_STOPPED,     ///< Recording stopped manually (MiIntNotf bears number of recorded samples).
      MI_NOTF_RECORD_FINISHED,    ///< Recording stopped automatically (MiIntNotf bears number of recorded samples).
      MI_NOTF_RECORD_ERROR,       ///< Recording stopped because of an error.
      MI_NOTF_DTMF_RECEIVED,      ///< Value for MiDtmfNotf notifications.
      MI_NOTF_DELAY_SPEECH_STARTED,
      MI_NOTF_DELAY_NO_DELAY,
      MI_NOTF_DELAY_QUIESCENCE,
      MI_NOTF_RX_STREAM_ACTIVITY, ///< Value for MiRtpStreamActivityNotf notifications.
      MI_NOTF_ENERGY_LEVEL,       ///< Audio energy level (MiIntNotf)
      MI_NOTF_VOICE_STARTED,
      MI_NOTF_VOICE_STOPPED
   } NotfType;

     /// Connection ID that indicates invalid connection or no connection.
   static const int INVALID_CONNECTION_ID;

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor
   MiNotification(NotfType msgType,
                  const UtlString& sourceId,
                  int connectionId = INVALID_CONNECTION_ID,
                  int streamId = -1);

   /// Copy constructor
   MiNotification(const MiNotification& rNotf);

   /// Create a copy of this msg object (which may be of a derived type)
   virtual OsMsg* createCopy(void) const;

   /// Destructor
   virtual ~MiNotification();

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

     /// Assignment operator
   MiNotification& operator=(const MiNotification& rhs);

     /// Set the unique source identifier.
   void setSourceId(const UtlString& sourceId);
     /**<
     *  Sets the unique identifier of the thing that originated this 
     *  notification.
     */

     /// Set the connection ID that this notification is associated with.
   void setConnectionId(int connId);

     /// @brief Set the stream number inside the connection this notification
     /// is associated with.
   void setStreamId(int streamId);

   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

     /// Returns the type of the notification message
   NotfType getType(void) const;

     /// Get the unique source identifier.
   UtlString getSourceId(void) const;
     /**<
     *  Returns the unique identifier of the thing that originated this 
     *  notification.
     */

     /// Get the connection ID that this message is associated with.
   int getConnectionId() const;

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
   UtlString mSourceId; ///< Unique identifier of the thing that originated this notification.
   int mConnectionId;   ///< If applicable, stores the ID of the connection this
                        ///< notification is associated with.  (-1 if N/A)
   int mStreamId;       ///< If applicable, stores the ID of the stream inside
                        ///< connection this notification is associated with. (-1 if N/A)
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MiNotification_h_
