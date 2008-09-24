//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
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
      MPRNM_FROMFILE_PROGRESS,
      MPRNM_BUFRECORDER_STOPPED,
      MPRNM_BUFRECORDER_FINISHED,
      MPRNM_BUFRECORDER_NOINPUTDATA,
      MPRNM_DTMF_RECEIVED,
      // MPRNM_MIXER_NEWFOCUS,

      // Add new built in resource notification messages above

      // Non-builtin resource notification messages
      MPRNM_EXTERNAL_MESSAGE_START = 128
      // Do not add new message types after this
   } RNMsgType;

     /// Connection ID that indicates invalid connection or no connection.
   static const MpConnectionID INVALID_CONNECTION_ID;

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor
   MpResNotificationMsg(RNMsgType msgType, 
                        const UtlString& namedResOriginator,
                        MpConnectionID connId = INVALID_CONNECTION_ID);

   /// Copy constructor
   MpResNotificationMsg(const MpResNotificationMsg& rMpResNotifyMsg);

   /// Create a copy of this msg object (which may be of a derived type)
   virtual OsMsg* createCopy(void) const;

   /// Destructor
   virtual
      ~MpResNotificationMsg();

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

   /// Set the connection ID that this message is associated with.
   void setConnectionId(MpConnectionID connId);

   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

   /// Returns the type of the media resource notification message
   int getMsg(void) const;

   /// Get the name of the resource that originated this message.
   UtlString getOriginatingResourceName(void) const;
   /**<
   *  Returns the name of the MpResource object that originated this
   *  message.
   */

   /// Get the connection ID that this message is associated with.
   MpConnectionID getConnectionId() const;

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
   MpConnectionID mConnectionId; ///< The ID of the connection this is associated with.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpResNotificationMsg_h_
