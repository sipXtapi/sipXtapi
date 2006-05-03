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

// APPLICATION INCLUDES
#include "cp/CSeqManager.h"
#include "utl/UtlVoidPtr.h"
#include "utl/UtlHashMapIterator.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CSeqManager::CSeqManager()
    : mGuard(OsMutex::Q_FIFO)
{
    miCSeq = (abs(mRandomGenerator.rand()) % 65535) ;
}

// Destructor
CSeqManager::~CSeqManager()
{
    OsLock lock(mGuard) ;
    UtlHashMapIterator iterator(mHashMap);    
    UtlString* pKey;   

    // Clean out hashmap    
    while (pKey = (UtlString*)iterator())
    {
        UtlVoidPtr* pValue = (UtlVoidPtr*) mHashMap.findValue(pKey) ;
        assert(pValue != NULL) ;
        if (pValue)
        {        
            CSEQ_CONTEXT* pContext = (CSEQ_CONTEXT*) pValue->getValue() ;
            mHashMap.destroy(pKey) ;
            delete pContext ;
        }
    }
}

/* ============================ MANIPULATORS ============================== */

bool CSeqManager::startTransaction(const char* szIdentifier, int& iCSeq)
{
    OsLock lock(mGuard) ;
    bool bOK = false ;

    assert(szIdentifier != NULL) ;
    CSEQ_CONTEXT* pContext = getContext(szIdentifier) ;
    assert(pContext != NULL) ;
    if (pContext)
    {
        iCSeq = pContext->iCSeq = miCSeq  ;
        miCSeq++ ;

        if (!pContext->bInTransaction)
        {
            bOK = true ;
            pContext->bInTransaction = true ;            
        }
        else
        {
#ifdef CSEQ_MANAGER_STATE_CHECKS
            assert(false) ;
#endif
        }
    }

    return bOK ;
}


bool CSeqManager::endTransaction(const char* szIdentifier)
{
    OsLock lock(mGuard) ;
    bool bOK = false ;

    assert(szIdentifier != NULL) ;

    CSEQ_CONTEXT* pContext = getContext(szIdentifier) ;
    assert(pContext != NULL) ;
    if (pContext)
    {
        if (pContext->bInTransaction)
        {
            bOK = true ;
            pContext->bInTransaction = false ;
        }
        else
        {
#ifdef CSEQ_MANAGER_STATE_CHECKS
            assert(false) ;
#endif
        }
    }

    return bOK ;
}


void CSeqManager::dumpState(UtlString& state) 
{
    OsLock lock(mGuard) ;
    UtlHashMapIterator iterator(mHashMap);
    UtlVoidPtr* pKey;   
    
    // Clean out hashmap
    while (pKey = (UtlVoidPtr*)iterator())
    {
        char cTemp[512] ;
        CSEQ_CONTEXT* pContext = (CSEQ_CONTEXT*) pKey->getValue();

        sprintf(cTemp, "Identifier: %s, CSeq=%d, inProgress=%d\n",               
                pContext->identifier.data(),
                pContext->iCSeq,
                pContext->bInTransaction) ;

        state.append(cTemp) ;
    }
}


/* ============================ ACCESSORS ================================= */
int CSeqManager::getCSeqNumber(const char* szIdentifier) 
{
    OsLock lock(mGuard) ;
    int iCSeq = 0 ;

    assert(szIdentifier != NULL) ;
    CSEQ_CONTEXT* pContext = getContext(szIdentifier) ;
    assert(pContext != NULL) ;
    if (pContext)
    {
        iCSeq = pContext->iCSeq ;
    }

    return iCSeq ;
}

/* ============================ INQUIRY =================================== */

bool CSeqManager::isInTransaction(const char* szIdentifier)
{
    OsLock lock(mGuard) ;
    bool bInTransaction = false ;

    assert(szIdentifier != NULL) ;
    CSEQ_CONTEXT* pContext = getContext(szIdentifier) ;
    assert(pContext != NULL) ;
    if (pContext)
    {
        bInTransaction = pContext->bInTransaction ;
    }

    return bInTransaction ;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

CSEQ_CONTEXT* CSeqManager::getContext(const char* szIdentifier)
{
    CSEQ_CONTEXT* pContext = NULL ;

    assert(szIdentifier != NULL) ;
    
    UtlString key(szIdentifier) ;
    UtlVoidPtr* pValue = (UtlVoidPtr*) mHashMap.findValue(&key) ;
    if (pValue)
    {
        pContext = (CSEQ_CONTEXT*) pValue->getValue() ;
        assert(pContext != NULL) ;
    }
    else
    {
        pContext = new CSEQ_CONTEXT ;
        mHashMap.insertKeyAndValue(new UtlString(szIdentifier),
                new UtlVoidPtr(pContext)) ;

        pContext->bInTransaction = false ;
        pContext->iCSeq = miCSeq ;
        pContext->identifier = szIdentifier ;
    }

    return pContext ;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


