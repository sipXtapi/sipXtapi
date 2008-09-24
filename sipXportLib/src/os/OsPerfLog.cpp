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
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
// Author: Bob Andreasen
//

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsPerfLog.h"
#include "os/OsSysLog.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define MAX_NUM_EVENT           64   /*< Max num of events to allow */
#define PRINT_EVENTS_STDOUT     1

// STATIC VARIABLE INITIALIZATIONS
OsMutex OsPerfLog::sLock(OsMutex::Q_FIFO) ;
OsPerfLog* OsPerfLog::spInstance = NULL ;
int OsPerfLog::sRefCnt = 0 ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

OsPerfLog::OsPerfLog(const char*      szIdentifier, 
                     OsSysLogPriority priority) 
    : OsTimeLog(MAX_NUM_EVENT)
{
    assert(szIdentifier != NULL) ;

    mPriority = priority ;
    mIdentifier = szIdentifier ;

    UtlString beginMsg = mIdentifier + " {" ;
    addEvent(beginMsg) ;
}


OsPerfLog::~OsPerfLog()
{
    UtlString endMsg = mIdentifier + " }" ;
    addEvent(endMsg) ;

    UtlString results ;
    getLogStringCSV(results) ;

    OsSysLog::add(FAC_PERF, mPriority, "%s", results.data()) ;
#if PRINT_EVENTS_STDOUT
    printf("%s", results.data()) ;
#endif
}

/* ============================ MANIPULATORS ============================== */

OsPerfLog* OsPerfLog::getInstance(const char*      szIdentifier,
                                  OsSysLogPriority priority, 
                                  int*             piCount) 
{
    OsLock lock(sLock) ;

    if (spInstance)
    {
        assert(sRefCnt > 0) ;
        sRefCnt++ ;
        if (piCount)
            *piCount = sRefCnt ;
    }
    else
    {
        assert(sRefCnt == 0) ;
        spInstance = new OsPerfLog(szIdentifier, priority) ;
        sRefCnt++ ;

        if (piCount)
            *piCount = sRefCnt ;
    }

    return spInstance ;
}


void OsPerfLog::releaseInstance(OsPerfLog* pPerfLog)
{
    OsLock lock(sLock) ;

    assert (pPerfLog == spInstance) ;
    sRefCnt-- ;
    assert(sRefCnt >= 0) ;
    if (sRefCnt == 0)
    {
        assert(spInstance != NULL) ;
        delete spInstance ;
        spInstance = NULL ; 
    }
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */




OsRecursivePerfLog::OsRecursivePerfLog(const char* szIdentifier,
                                       OsSysLogPriority priority)
{

    assert(szIdentifier != NULL) ;
    mIdentifier = szIdentifier ;
    mpPerfLog = OsPerfLog::getInstance(szIdentifier, priority, &miCount) ;
    assert(mpPerfLog != NULL) ;
    if (mpPerfLog)
    {
        if (miCount > 1)
        {
            UtlString beginEvent = mIdentifier + " {" ;
            addEvent(beginEvent) ;
        }
    }
}


OsRecursivePerfLog::~OsRecursivePerfLog() 
{
    if (mpPerfLog)
    {
        if (miCount > 1)
        {
            UtlString endEvent = mIdentifier + " }" ;
            addEvent(endEvent) ;
        }
        OsPerfLog::releaseInstance(mpPerfLog) ;
    }
}


void OsRecursivePerfLog::addEvent(const char* szEvent) 
{
    if (mpPerfLog)
    {
        UtlString event ;

        for (int i=1; i<miCount; i++)
        {
            event.append("  ") ;
        }
        event.append(szEvent) ;
        mpPerfLog->addEvent(event) ;
    }
}


