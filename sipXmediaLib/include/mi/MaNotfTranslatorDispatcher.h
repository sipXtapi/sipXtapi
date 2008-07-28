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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
// USA. 
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

#ifndef _MaNotfConvertDispatcher_h_
#define _MaNotfConvertDispatcher_h_

// SYSTEM INCLUDES
#include <os/OsMsgDispatcher.h>

// APPLICATION INCLUDES
#include "mi/MiNotification.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Dispatcher to translate MediaLib notifications into abstracted 
*         MediaAdapterLib notifications.
*
*  This message dispatcher is used to receive notifications from the mediaLib
*  layer, then convert those messages to more abstract mediaAdapter notification
*  messages and send them off again to another, supplied dispatcher.
*  @see OsMsgDispatcher
*/
class MaNotfTranslatorDispatcher : public OsMsgDispatcher
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor
   MaNotfTranslatorDispatcher(OsMsgDispatcher* pAbstractedMsgDispatcher = NULL);

     /// Destructor
   virtual ~MaNotfTranslatorDispatcher();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
     /// Set the media interface dispatcher that will receive the converted 
     /// abstract notifications.
   virtual OsMsgDispatcher* setDispatcher(OsMsgDispatcher* pMIDispatcher);

     /// @copydoc OsMsgDispatcher::post()
   virtual OsStatus post(const OsMsg& msg);

     /// @copydoc OsMsgDispatcher::receive()
   virtual OsStatus receive(OsMsg*& rpMsg,
                            const OsTime& rTimeout=OsTime::OS_INFINITY);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get the media interface dispatcher currently set on this translator dispatcher
   virtual OsMsgDispatcher* getDispatcher() const;

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

     /// Return whether this translator has a dispatcher to receive notifications from
     /// set on it or not.
   virtual UtlBoolean hasDispatcher() const;

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   OsMsgDispatcher* mpAbstractedMsgDispatcher;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MaNotfConvertDispatcher_h_
