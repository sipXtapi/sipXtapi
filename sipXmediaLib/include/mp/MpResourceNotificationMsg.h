//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpResourceNotificationMsg_h_
#define _MpResourceNotificationMsg_h_

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
class MpResourceNotificationMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /// Phone set message types
   typedef enum
   {
      // MPRNM_MIXER_NEWFOCUS,

      // Add new built in resource operation messages above

      // Non-builtin resource notification messages
      MPRNM_EXTERNAL_MESSAGE_START = 128
      // Do not add new message types after this
   } MpResourceNotificationMsgType;

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor
   MpResourceNotificationMsg(MpResourceNotificationMsgType msg, 
                             const UtlString& namedResourceOriginator);

   /// Copy constructor
   MpResourceNotificationMsg(const MpResourceNotificationMsg& rMpResNotifyMsg);

   /// Create a copy of this msg object (which may be of a derived type)
   virtual OsMsg* createCopy(void) const;

   /// Destructor
   virtual
      ~MpResourceNotificationMsg();

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /// Assignment operator
   MpResourceNotificationMsg& operator=(const MpResourceNotificationMsg& rhs);

   /// Set the name of the resource this message applies to.
   void setOriginatingResourceName(const UtlString& resourceOriginator);
   /**<
   *  Sets the name of the intended recipient for this message.
   */
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

   //@}

   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlString mMsgOriginatorName; ///< Name of the resource that 
   ///< originated this message.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpResourceNotificationMsg_h_
