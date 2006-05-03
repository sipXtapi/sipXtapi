//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsTimeLog.h"
#include "os/OsDateTime.h"
#include "utl/UtlString.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
#ifdef _VXWORKS
extern volatile int* pOsTC;  // pointer to the high resolution timer
#endif

// CONSTANTS
#ifdef _VXWORKS
static int MICROSECS_PER_SEC = 1000000;
#endif

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsTimeLog::OsTimeLog(int maxEventCount)
{
   mMaxEventCount = maxEventCount;
   mNumEvents = 0;

   mpaEventNames = new UtlString*[mMaxEventCount];
   mpaEventTimes = new OsTime*[mMaxEventCount];
   for(int index = 0; index < mMaxEventCount; index++)
   {
       mpaEventNames[index] = NULL;
       mpaEventTimes[index] = NULL;
   }
}

// Copy constructor
OsTimeLog::OsTimeLog(const OsTimeLog& rOsTimeLog)
{
    mMaxEventCount = rOsTimeLog.mMaxEventCount;
    mNumEvents = rOsTimeLog.mNumEvents;

   mpaEventNames = new UtlString*[mMaxEventCount];
   mpaEventTimes = new OsTime*[mMaxEventCount];
   for(int index = 0; index < mMaxEventCount; index++)
   {
       if(rOsTimeLog.mpaEventNames &&
           rOsTimeLog.mpaEventNames[index])
       {
           mpaEventNames[index] = new UtlString(*(rOsTimeLog.mpaEventNames[index]));
       }
       else if(rOsTimeLog.mpaEventNames)
       {
           mpaEventNames[index] = NULL;
       }
       if(rOsTimeLog.mpaEventTimes &&
           rOsTimeLog.mpaEventTimes[index])
       {
           mpaEventTimes[index] = new OsTime(*(rOsTimeLog.mpaEventTimes[index]));
       }
       else if(rOsTimeLog.mpaEventTimes)
       {
           mpaEventTimes[index] = NULL;
       }
   }
}

// Destructor
OsTimeLog::~OsTimeLog()
{
  for(int index = 0; index < mMaxEventCount; index++)
   {
       if(mpaEventNames && mpaEventNames[index])
       {
           delete (UtlString*)mpaEventNames[index];
           mpaEventNames[index] = NULL;
       }

       if(mpaEventTimes && mpaEventTimes[index])
       {
           delete (OsTime*)mpaEventTimes[index];
           mpaEventTimes[index] = NULL;
       }
   }

  delete[] mpaEventNames;
  delete[] mpaEventTimes;
  mpaEventNames = 0;
  mpaEventTimes = 0;


}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsTimeLog&
OsTimeLog::operator=(const OsTimeLog& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   int index;

   for(index = 0; index < mMaxEventCount; index++)
   {
       if(mpaEventNames && mpaEventNames[index]) delete mpaEventNames[index];
       if(mpaEventTimes && mpaEventTimes[index]) delete mpaEventTimes[index];
   }

   // Need bigger arrays
   if(mMaxEventCount < rhs.mMaxEventCount)
   {
       if(mpaEventNames) delete[] mpaEventNames;
       if(mpaEventTimes) delete[] mpaEventTimes;

       mpaEventNames = new UtlString*[mMaxEventCount];
       mpaEventTimes = new OsTime*[mMaxEventCount];

   }

    mMaxEventCount = rhs.mMaxEventCount;
    mNumEvents = rhs.mNumEvents;

   for(index = 0; index < mMaxEventCount; index++)
   {
       if(rhs.mpaEventNames &&
           rhs.mpaEventNames[index])
       {
           mpaEventNames[index] = new UtlString(*(rhs.mpaEventNames[index]));
       }
       else if(rhs.mpaEventNames)
       {
           mpaEventNames[index] = NULL;
       }
       if(rhs.mpaEventTimes &&
           rhs.mpaEventTimes[index])
       {
           mpaEventTimes[index] = new OsTime(*(rhs.mpaEventTimes[index]));
       }
       else if(rhs.mpaEventTimes)
       {
           mpaEventTimes[index] = NULL;
       }
   }

   return *this;
}

//: Adds an event to the log for the current time
void OsTimeLog::addEvent(const char* eventName)
{
#ifdef _VXWORKS
   double scaleFactor;
   unsigned long microSecs;

   scaleFactor = 1 / 3.6864;  /* clock runs a 3.6864 MHz */
   microSecs = (unsigned long) ((*pOsTC) * scaleFactor);

   OsTime* now = new OsTime(microSecs / MICROSECS_PER_SEC,
         microSecs % MICROSECS_PER_SEC);
#else
   OsTime* now = new OsTime();
   OsDateTime::getCurTime(*now);
#endif

    addEvent(eventName, now);
}

//: Adds an event to the log for the given time
void OsTimeLog::addEvent(const char* eventName, OsTime* eventTime)
{
    if(mNumEvents < mMaxEventCount)
    {
        UtlString* name = new UtlString(eventName);
        mpaEventNames[mNumEvents] = name;
        mpaEventTimes[mNumEvents] = eventTime;
        mNumEvents++;
    }
}

//: Dumps the log out using osPrintf
void OsTimeLog::dumpLog() const
{
    UtlString log;

    getLogString(log);

    osPrintf("%s", log.data());
        log.remove(0);
}

void OsTimeLog::getLogString(UtlString& log) const
{
    int index;
    OsTime time;
    OsTime deltaTime;
    OsTime tZero;
    OsTime previousTime;
    size_t maxNameLength = 0;
    char timeString[40];

    for(index = 0; index < mMaxEventCount; index++)
    {
        if(mpaEventNames[index] &&
            mpaEventNames[index]->length() > maxNameLength)
        {
            maxNameLength = mpaEventNames[index]->length();
        }
    }

    int nameColumnTabs = ((int) (maxNameLength / 8)) + 1;

    // Put a header in after we know how long the event names are
    log.append("Name");
    for(index = 0; index < nameColumnTabs; index++)
        log.append('\t');
    log.append("Time\tIncremental Time\n");

    if(mpaEventTimes[0]) tZero = *(mpaEventTimes[0]);
    for(index = 0; index < mMaxEventCount; index++)
    {
        if(mpaEventNames[index])
        {
            log.append(mpaEventNames[index]->data());
        }

        if(mpaEventTimes[index])
        {
            time = *mpaEventTimes[index] - tZero;;
            sprintf(timeString, "\t%ld.%.6ld", time.seconds(),
                time.usecs());
            log.append(timeString);

            if(index > 0)
            {
                deltaTime = time - previousTime;
                sprintf(timeString, "\t%ld.%.6ld", deltaTime.seconds(),
                deltaTime.usecs());
                log.append(timeString);
            }
            else
            {
                log.append("\tN/A");
            }
            previousTime = time;
        }
        if(mpaEventNames[index] || mpaEventTimes[index] ||
            index < mNumEvents) log.append("\n");
    }

}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
