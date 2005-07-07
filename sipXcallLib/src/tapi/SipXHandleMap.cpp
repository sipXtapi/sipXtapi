// $Id$
//
// Copyright (C) 2005 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPXTAPI_EXCLUDE /* [ */

// SYSTEM INCLUDES
#ifdef _WIN32
#include <windows.h>
#endif

// APPLICATION INCLUDES
#include "utl/UtlVoidPtr.h"
#include "utl/UtlInt.h"
#include "tapi/SipXHandleMap.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipXHandleMap::SipXHandleMap()
    : mLock(OsMutex::Q_FIFO)
    , mNextHandle(1)
{
}

// Destructor
SipXHandleMap::~SipXHandleMap()
{
}

/* ============================ MANIPULATORS ============================== */


void SipXHandleMap::lock()
{
    mLock.acquire() ;
}


void SipXHandleMap::unlock() 
{
    mLock.release() ;
}


SIPXHANDLE SipXHandleMap::allocHandle(const void* pData) 
{

    SIPXHANDLE hCall = mNextHandle++ ;

    lock() ;
    insertKeyAndValue(new UtlInt(hCall), new UtlVoidPtr((void*) pData)) ;
    unlock() ;

    return hCall ;
}

const void* SipXHandleMap::findHandle(SIPXHANDLE handle) 
{
    lock() ;

    const void* pRC = NULL ;
    UtlInt key(handle) ;
    UtlVoidPtr* pValue ;

    pValue = (UtlVoidPtr*) findValue(&key) ;
    if (pValue != NULL)
    {
        pRC = pValue->getValue() ;
    }

    unlock() ;

    return pRC ;

}


const void* SipXHandleMap::removeHandle(SIPXHANDLE handle) 
{
    lock() ;

    const void* pRC = NULL ;
    UtlInt key(handle) ;
    UtlVoidPtr* pValue ;
        
    pValue = (UtlVoidPtr*) findValue(&key) ;
    if (pValue != NULL)
    {
        pRC = pValue->getValue() ;                
        destroy(&key) ;
    }

    unlock() ;
    
    return pRC ;
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

#endif /* ] SIPXTAPI_EXCLUDE */
