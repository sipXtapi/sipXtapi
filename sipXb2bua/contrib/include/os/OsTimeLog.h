//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsTimeLog_h_
#define _OsTimeLog_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsTime.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlString;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class OsTimeLog
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsTimeLog(int maxEventCount = 100);
     //:Default constructor

   OsTimeLog(const OsTimeLog& rOsTimeLog);
     //:Copy constructor

   virtual
   ~OsTimeLog();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsTimeLog& operator=(const OsTimeLog& rhs);
     //:Assignment operator

   void addEvent(const char* eventName);
   //: Adds an event to the log for the current time

   void addEvent(const char* eventName, OsTime* eventTime);
   //: Adds an event to the log for the given time
   // Note: eventTime must be allocated off the heap and is
   // freed by the OsTimeLog destructor.

   void dumpLog() const;
   //: Dumps the log out using osPrintf

/* ============================ ACCESSORS ================================= */

   void getLogString(UtlString& logString) const;
   //: Get log with column headers and rows of named events with lapse and incremental time
   // i.e.:
   // Name          Time        Incremental Time
   // CREATED   0.000000        N/A
   // SENDING   0.160000        0.160000
   // SENDING   0.771000        0.771000
   // SENDING   1.823000        1.823000
   // SENDING   3.866000        2.866000
   // SENDING   7.931000        4.931000
   // SENDING   11.987000       4.987000

   UtlBoolean getEventTime(const char* eventName, OsTime& time) const;
   //: Get the named event time

   UtlBoolean getEventTime(int eventIndex, OsTime& time) const;
   //: get the event time indicated by the index
   // Note: the first event index = 0

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    UtlString** mpaEventNames;
    OsTime** mpaEventTimes;
    int mMaxEventCount;
    int mNumEvents;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsTimeLog_h_
