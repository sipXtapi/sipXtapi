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
