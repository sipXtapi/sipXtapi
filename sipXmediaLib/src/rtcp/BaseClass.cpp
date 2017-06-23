//
// Copyright (C) 2006-2017 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


//  Includes
#include "rtcp/BaseClass.h"
// #include <assert.h>
#include "os/OsBSem.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"

//  Use a lock to protect access to the reference count
OsBSem sMultiThreadLock(OsBSem::Q_PRIORITY, OsBSem::FULL);


static int sulTotalReferenceCount = 0;
bool CBaseClass::s_bAllowDeletes = true;
void CBaseClass::s_SetAllowDeletes(int v)
{
    s_bAllowDeletes = (v != 0);
}

/**
 *
 * Method Name:  CBaseClass() - Constructor
 *
 *
 * Inputs:       None
 *
 *
 * Outputs:      None
 *
 * Returns:      None
 *
 * Description:  The CBaseClass is an abstract class that is initialized by a
 *               derived object at construction time.
 *
 * Usage Notes:
 *
 */
CBaseClass::CBaseClass CBASECLASS_PROTO_ARGS((const char* DerivedType, int callLineNum)) :
    m_bInitialized(FALSE)
    , m_bAutomatic(FALSE)
    , m_ulReferences(1)
#ifdef RTCP_DEBUG_REFCOUNTS /* [ */
    , m_DerivedType(DerivedType)
    , m_Line(callLineNum)
#endif /* RTCP_DEBUG_REFCOUNTS ] */
{
    sulTotalReferenceCount++;
#ifdef RTCP_DEBUG_REFCOUNTS /* [ */
    OsSysLog::add(FAC_MP, PRI_DEBUG, "CBaseClass::CBaseClass:  this=%p, sRefs=%d, type=%s", this, sulTotalReferenceCount, DerivedType);
#endif /* RTCP_DEBUG_REFCOUNTS ] */
}



/**
 *
 * Method Name: ~CBaseClass() - Destructor
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Shall deallocate and/or release all resources which were
 *              acquired over the course of runtime.
 *
 * Usage Notes:
 *
 *
 */
CBaseClass::~CBaseClass(void)
{
    sulTotalReferenceCount--;
#ifdef RTCP_DEBUG_REFCOUNTS /* [ */
    OsSysLog::add(FAC_MP, PRI_DEBUG, "CBaseClass::~CBaseClass: this=%p, sRefs=%d, m_ulReferences=%ld", this, sulTotalReferenceCount, m_ulReferences);
#endif /* RTCP_DEBUG_REFCOUNTS ] */
    if (m_bAutomatic) m_ulReferences--;
    assert(0 == m_ulReferences);
}

/**
 *
 * Method Name:  AddRef
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long
 *
 * Description: Increments the number of references to this object.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CBaseClass::AddRef CBASECLASS_PROTO_ARGS((int callLineNum))
{
    
    OsLock lock(sMultiThreadLock);

    assert(m_ulReferences > 0);

    //  Increment Reference Count
    m_ulReferences++;
    sulTotalReferenceCount++;
#ifdef RTCP_DEBUG_REFCOUNTS /* [ */
    if (callLineNum > 0) OsSysLog::add(FAC_MP, PRI_DEBUG, "BaseClass: %p->AddRef(%d),  count: %2ld", this, callLineNum, m_ulReferences);
#endif /* RTCP_DEBUG_REFCOUNTS ] */
    return(m_ulReferences);

}


/**
 *
 * Method Name:  Release
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long
 *
 * Description: Decrements the number of references to this object.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CBaseClass::Release CBASECLASS_PROTO_ARGS((int callLineNum))
{
    
    OsLock lock(sMultiThreadLock);

    assert(m_ulReferences > 0);

    //  Decrement Reference Count
    sulTotalReferenceCount--;
    m_ulReferences--;

    if (callLineNum > 0) OsSysLog::add(FAC_MP, PRI_DEBUG, "BaseClass: %p->Release(%d), count: %2d", this, callLineNum, m_ulReferences);
    return(m_ulReferences);

}

#ifdef __pingtel_on_posix__

/* See below for an explanation of these - they're implemented in terms of
* the OS abstraction layer under POSIX (as should the vxWorks ones, but they
* don't yet). Really, the WIN32 version ought to as well, but it seems that
* we've just defined this particular bit of code in terms of that API so
* there's no need to reinvent the wheel there. */

void InitializeCriticalSection(CRITICAL_SECTION *csSynchronized)
{
//  Create an OS abstraction layer binary semaphore
    *csSynchronized = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);
}

void EnterCriticalSection(CRITICAL_SECTION *csSynchronized)
{
#if TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG,
            "EnterCriticalSection entered semiphore: %p",
            csSynchronized);
#endif
//  Attempt to gain exclusive access to a protected resource by taking
//  its semaphore
    if(*csSynchronized)
    {
        OsTime timeOut(1, 0);
        OsStatus status = OS_FAILED;
        do
        {
            status = (*csSynchronized)->acquire(timeOut);
            if(status != OS_SUCCESS)
            {
                OsSysLog::add(FAC_MP, PRI_ERR,
                    "RTCP EnterCriticalSection acquire returned: %d dead lock semiphore: %p",
                    status,
                    csSynchronized);
            }

        } while(status != OS_SUCCESS);
    }
}

void LeaveCriticalSection(CRITICAL_SECTION *csSynchronized)
{
#if TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG,
            "LeaveCriticalSection entered semiphore: %p",
            csSynchronized);
#endif
//  Attempt to relinquish exclusive access to a protected resource by giving
//  back its semaphore
    if(*csSynchronized)
        (*csSynchronized)->release();
}

void DeleteCriticalSection(CRITICAL_SECTION *csSynchronized)
{
//  Free all blocking
    if(*csSynchronized)
        delete *csSynchronized;
    *csSynchronized = NULL;
}
#endif /* __pingtel_on_posix__ ] */

#ifdef _VXWORKS /* [ */

/*|><|************************************************************************
*
* Function Name: InitializeCriticalSection
*
*
* Inputs:       CRITICAL_SECTION *csSynchronized
*
* Outputs:      None
*
* Returns:      None
*
* Description:  This is a compatability function that maps the creation of a
*               vxWorks semaphore to a complementary call under Win32.
*
* Usage Notes:
*
*
*************************************************************************|<>|*/
void InitializeCriticalSection(CRITICAL_SECTION *csSynchronized)
{

//  Create a binary vxWorks Semaphore
    *csSynchronized = semBCreate(SEM_Q_FIFO, SEM_FULL);

}

/*|><|*************************************************************************
*
* Function Name: EnterCriticalSection
*
*
* Inputs:       CRITICAL_SECTION *csSynchronized
*
* Outputs:      None
*
* Returns:      None
*
* Description:  This is a compatability function that maps the creation of a
*               vxWorks semaphore to a complementary call under Win32.
*
* Usage Notes:
*
*
*************************************************************************|<>|*/
void EnterCriticalSection(CRITICAL_SECTION *csSynchronized)
{

//  Attempt to gain exclusive access to a protected resource by taking
//  its semaphore
    if(*csSynchronized)
        semTake(*csSynchronized, WAIT_FOREVER);

}

/*|><|*************************************************************************
*
* Function Name: LeaveCriticalSection
*
*
* Inputs:       CRITICAL_SECTION *csSynchronized
*
* Outputs:      None
*
* Returns:      None
*
* Description:  This is a compatability function that maps the creation of a
*               vxWorks semaphore to a complementary call under Win32.
*
* Usage Notes:
*
*
*************************************************************************|<>|*/
void LeaveCriticalSection(CRITICAL_SECTION *csSynchronized)
{

//  Attempt to relinquish exclusive access to a protected resource by giving
//  back its semaphore
    if(*csSynchronized)
        semGive(*csSynchronized);

}
/*|><|*************************************************************************
*
* Function Name: DeleteCriticalSection
*
*
* Inputs:       CRITICAL_SECTION *csSynchronized
*
* Outputs:      None
*
* Returns:      None
*
* Description:  This is a compatability function that maps the creation of a
*               vxWorks semaphore to a complementary call under Win32.
*
* Usage Notes:
*
*
*************************************************************************|<>|*/
void DeleteCriticalSection(CRITICAL_SECTION *csSynchronized)
{

//  Free all blocking
    if(*csSynchronized)
    {
        semFlush(*csSynchronized);
        semDelete(*csSynchronized);
    }

}
#endif /* _VXWORKS ] */

