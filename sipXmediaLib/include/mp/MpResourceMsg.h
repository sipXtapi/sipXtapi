//  
// Copyright (C) 2007-2011 SIPez LLC.  All rights reserved.
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
      MPRM_ENABLE_ALL_NOTIFICATIONS,   ///< All resources: Enable all notifications to be sent from a resource
      MPRM_DISABLE_ALL_NOTIFICATIONS,  ///< All resources: Disable all notification from being sent a from resource

      MPRM_START_TONE,          ///< MprToneGen: Start a tone.
      MPRM_STOP_TONE,           ///< MprToneGen: Stop a tone.

      MPRM_BRIDGE_SET_GAINS,    ///< MprBridge: Set row or column in mix matrix.

      MPRM_DECODE_SELECT_CODECS,  ///< MprDecode/MprEncode: Select codecs.

      MPRM_SET_SOCKETS,         ///< MprFromNet and containers set RTP/RTCP sockets
      MPRM_RESET_SOCKETS,       ///< MprFromNet and containers stop reading sockets

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
   virtual OsMsg* createCopy() const;

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
   int getMsg() const;

     /// Get the name of the resource this message applies to.
   UtlString getDestResourceName() const;
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
