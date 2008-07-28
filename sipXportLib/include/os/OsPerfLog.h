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

#ifndef _OsPerfLog_h_
#define _OsPerfLog_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsTimeLog.h"
#include "os/OsSysLog.h"

// DEFINES
#define OS_PERF_ENABLE
#undef OS_PERF_ENABLE

// MACROS
#define OS_PERF_PREFIX ""
#ifdef OS_PERF_ENABLE
#    define OS_PERF_FUNC(func) \
     OsRecursivePerfLog __osPerfFuncLog(OS_PERF_PREFIX func)
#define OS_PERF_ADD(desc) \
     __osPerfFuncLog.addEvent(desc)
#else
#   define OS_PERF_FUNC(func) 
#   define OS_PERF_ADD(desc)
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
 * An OsPerfLog is designed to capture performance information for function
 * calls.  It's destructor will automatically add data to the OsSysLog under
 * a PERF (performance) facility.
 *
 * Singleton/static instances are also included for recursive perf 
 * captures.  The default macro (above) use the static mechanism.
 */
class OsPerfLog : public OsTimeLog
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
  
/* ============================ CREATORS ================================== */
    /**
     * Constructor requiring an initial identifier
     *
     * @param szIdentifier name (e.g. function name) to included in 
     *        performance log.
     */
    OsPerfLog(const char* szIdentifier,
              OsSysLogPriority priority = PRI_DEBUG) ;

    /**
     * Default destructor
     */
    virtual ~OsPerfLog() ;

/* ============================ MANIPULATORS ============================== */

    static OsPerfLog* getInstance(const char*      szIdentifier,
                                  OsSysLogPriority priority = PRI_DEBUG,
                                  int*             piCount = NULL) ;

    static void releaseInstance(OsPerfLog* pPerfLog) ;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    OsSysLogPriority  mPriority ;
    UtlString         mIdentifier ;
    static OsMutex    sLock ;
    static OsPerfLog* spInstance ;
    static int        sRefCnt ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsPerfLog(const OsPerfLog& rOsCSemBase);
     //:Copy constructor (not implemented for this class)

   OsPerfLog& operator=(const OsPerfLog& rhs);
     //:Assignment operator (not implemented for this class) 
} ;



/**
 * Simple wrapper which uses a global instance of the perf log to mark
 * recursive calls.
 */
class OsRecursivePerfLog
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
  
/* ============================ CREATORS ================================== */

public:
    OsRecursivePerfLog(const char* szIdentifier,
                      OsSysLogPriority priority = PRI_DEBUG) ;

    virtual ~OsRecursivePerfLog() ;

/* ============================ MANIPULATORS ============================== */

    void addEvent(const char* szEvent) ;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    OsPerfLog* mpPerfLog ;
    int        miCount ;
    UtlString  mIdentifier ;
    
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsRecursivePerfLog(const OsRecursivePerfLog& rOsCSemBase);
     //:Copy constructor (not implemented for this class)

   OsRecursivePerfLog& operator=(const OsRecursivePerfLog& rhs);
     //:Assignment operator (not implemented for this class) 


} ;

#endif
