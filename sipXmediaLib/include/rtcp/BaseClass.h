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

//  Border Guard
#ifndef _BaseClass_h /* [ */
#define _BaseClass_h

#include "rtcp/RtcpConfig.h"
#include "os/OsSysLog.h"

//  Includes
#ifdef WIN32 /* [ */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>

#ifdef PINGTEL_PRINTF
#include "os/OsDefs.h"
#else
#define osPrintf printf
#endif

// Required for VC7 compatibility
#if _MSC_VER >= 1300
#ifndef interface
    #define interface __interface
#endif
#endif

#endif /* WIN32 ] */

#ifdef _VXWORKS /* [ */
#include <vxworks.h>
#include <stdio.h>
#include <inetLib.h>
#include <hostLib.h>
#include <string.h>
#include "os/OsDefs.h"

//  Windows Compatability
#define interface struct

//  Make sure that WAIT FOREVER is defined
#ifndef WAIT_FOREVER
#define WAIT_FOREVER VX_WAIT_FOREVER
#endif /* WAIT_FOREVER */

#endif /* _VXWORKS ] */


//  Static Declarations
// static unsigned long sulTotalReferenceCount = 0;
extern bool     bPingtelDebug;


#ifndef WIN32 /* [ */

#ifdef __pingtel_on_posix__ /* [ */
#include <os/OsBSem.h>
typedef OsBSem * CRITICAL_SECTION;
#define interface struct
#endif /* __pingtel_on_posix__ ] */

#ifdef _VXWORKS /* [ */
#include <semLib.h>
typedef SEM_ID  CRITICAL_SECTION;
#endif /* _VXWORKS ] */


/*|><|************************************************************************
*
* Functions: CriticalSection support
*
*
* Inputs:       CRITICAL_SECTION *csSynchronized
*
* Outputs:      None
*
* Returns:      None
*
* Description:  Compatability functions that map to
*               complementary calls under Win32.
*
*
*************************************************************************|<>|*/
void InitializeCriticalSection(CRITICAL_SECTION *csSynchronized);
void EnterCriticalSection(CRITICAL_SECTION *csSynchronized);
void LeaveCriticalSection(CRITICAL_SECTION *csSynchronized);
void DeleteCriticalSection(CRITICAL_SECTION *csSynchronized);
#endif /* NOT _WIN32 ] */


/**
 *
 * Class Name:  CBaseClass
 *
 * Inheritance: CBaseClass           - Base Class Implementation
 *
 *
 * Interfaces:  None
 *
 * Description: The CBaseClass Class contains functions common to all class
 *              within the Pingtel programming environment.
 *
 * Notes:       The CBaseClass is inherited by all RTCP Report objects.
 *
 */
class CBaseClass
{

//  Public Methods
public:

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
    CBaseClass CBASECLASS_PROTO_ARGS((const char* pDerivedType = "Unknown", int callLineNum = 0));



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
    virtual ~CBaseClass(void);

/**
 *
 * Method Name:  Initialize
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     bool
 *
 * Description: Performs runtime initialization that may be failure prone
 *              and therefore inappropriate for execution within a constructor.
 *
 * Usage Notes:
 *
 *
 */
    virtual bool Initialize(void);

/**
 *
 * Method Name: IsInitialized
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     Boolean True/False
 *
 * Description: Returns a boolean specifying whether an object has already
 *              been created and initialized.
 *
 * Usage Notes:
 *
 *
 */
    bool IsInitialized(void);

    void setAutomatic(bool);

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
    virtual unsigned long AddRef ADD_RELEASE_PROTO_ARGS((int callLineNum));

/**
 *
 * Method Name: Release
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
    virtual unsigned long Release ADD_RELEASE_PROTO_ARGS((int callLineNum));

static bool AllowDeletes(void);
static void s_SetAllowDeletes(int v);

// protected:
/**
 *
 * Attribute Name:  m_bInitialized
 *
 * Type:            bool
 *
 * Description:     This member shall store an indicator of whether
 *                  construction time initialization completed successfully.
 *
 */
      bool m_bInitialized;

      bool m_bAutomatic;

/**
 *
 * Attribute Name:  m_ulReferences
 *
 * Type:            int
 *
 * Description:     This member shall keep track of the number of references
 *                  to an object.
 *
 */
      int m_ulReferences;

#ifdef RTCP_DEBUG_REFCOUNTS /* [ */
      const char* m_DerivedType;
      int   m_Line;
#endif /* RTCP_DEBUG_REFCOUNTS ] */

      static bool s_bAllowDeletes;

};

/**
 *
 * Method Name:  Initialize
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     bool
 *
 * Description: Performs runtime initialization that may be failure prone and
 *              therefore inappropriate for execution within a constructor.
 *
 * Usage Notes:
 *
 *
 */
inline bool CBaseClass::Initialize(void)
{

//  Set initialization flag to TRUE
    m_bInitialized = TRUE;

    return(TRUE);

}


/**
 *
 * Method Name: IsInitialized
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     Boolean True/False
 *
 * Description: Returns a boolean specifying whether an object instance has
 *              already been created and initialized.
 *
 * Usage Notes:
 *
 *
 */
inline bool CBaseClass::IsInitialized(void)
{

    return(m_bInitialized);

}

inline void CBaseClass::setAutomatic(bool yesNo)
{
    m_bAutomatic = yesNo;
}

inline bool CBaseClass::AllowDeletes(void)
{
    return(s_bAllowDeletes);
}


/**
 *
 * Macro Name:  DECLARE_IBASE_M
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: This implements the IBaseClass functions used and exposed by
 *              derived classes.
 *
 * Usage Notes:
 *
 *
 */
#define DECLARE_IBASE_M \
   unsigned long AddRef ADD_RELEASE_PROTO_ARGS((int callLineNum)) \
   { \
      if (callLineNum > 0) OsSysLog::add(FAC_MP, PRI_DEBUG,  __FILE__ " %p->AddRef(%d),  count: %d", this, callLineNum, m_ulReferences); \
      return CBaseClass::AddRef(ADD_RELEASE_CALL_ARGS(callLineNum)); \
   } \
   unsigned long Release ADD_RELEASE_PROTO_ARGS((int callLineNum)) \
   { \
      int ulRefCount; \
      if((ulRefCount = CBaseClass::Release(ADD_RELEASE_CALL_ARGS(callLineNum))) == 0) \
      { \
        if (CBaseClass::AllowDeletes()) { \
          delete this; \
        } else { \
          if (callLineNum > 0) OsSysLog::add(FAC_MP, PRI_DEBUG,  __FILE__ " %p->Release(%d) [would delete this]", this, callLineNum); \
        } \
      } \
      if (callLineNum > 0) OsSysLog::add(FAC_MP, PRI_DEBUG,  __FILE__ " %p->Release(%d), count: %d", this, callLineNum, ulRefCount); \
      return(ulRefCount); \
   }
#endif /* ] */
