//
// Copyright (C) 2006-2013 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsNotification_h_
#define _OsNotification_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <os/OsIntTypes.h>
#include <os/OsTime.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Abstract base class for event notifications

class OsNotification
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   //:Default constructor
   OsNotification() :
      mSignalTimeout(OsTime::OS_INFINITY)
    { };

   virtual
      ~OsNotification() { };
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus signal(const intptr_t eventData) = 0;
     //:Signal the occurrence of the event

   /// Set timeout to use when signalling if supported by derived class
   virtual void setTimeout(const OsTime& signalTimeout)
   {
       mSignalTimeout = signalTimeout;
   };

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   OsTime mSignalTimeout;   ///< Used by derived classes that support a timeout on signal

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsNotification(const OsNotification& rOsNotification);
     //:Copy constructor (not implemented for this class)

   OsNotification& operator=(const OsNotification& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsNotification_h_

