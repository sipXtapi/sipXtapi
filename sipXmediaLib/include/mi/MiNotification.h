// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
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

  /// Message notification class used to communicate media notification events. 
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
      MI_NOTF_PROGRESS,
      MI_NOTF_RECORD_STOPPED,
      MI_NOTF_RECORD_FINISHED,
      MI_NOTF_RECORD_NOINPUTDATA,
      MI_NOTF_DTMF_RECEIVED,
   } NotfType;

     /// Connection ID that indicates invalid connection or no connection.
   static const int INVALID_CONNECTION_ID;

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor
   MiNotification(NotfType msgType, 
                  const UtlString& sourceId,
                  int connectionId = INVALID_CONNECTION_ID);

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

     /// Store the ID of the connection that this notification is associated with.
   void setConnectionId(int connId);

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

     /// Get the ID of the connection that this notification is associated with.
   int getConnectionId() const;

   //@}

   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlString mSourceId; ///< Unique identifier of the thing that originated this message.
   int mConnectionId;   ///< If applicable, stores the ID of the connection this
                        ///< message is associated with.  (-1 if N/A)
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MiNotification_h_
