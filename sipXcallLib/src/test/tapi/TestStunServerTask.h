//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _TestStunServerTask_h_
#define _TestStunServerTask_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsTask.h"
#include "EventValidator.h"

// DEFINES

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef enum
{
    TEST_NORMAL,
    TEST_NO_RESPONSE,
    TEST_RETURN_ERROR,
    TEST_DROP_ODD_REQUEST,
    TEST_DROP_EVEN_REQUEST,
    TEST_DELAY_RESPONSE,
} STUN_TEST_MODE ;

// FORWARD DECLARATIONS
class OsDatagramSocket ;
class StunMessage ;
class UtlHashMap ;

/**
 * 
 */
class TestStunServerTask : public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

    /**
     * Default constructor
     */
    TestStunServerTask(OsDatagramSocket* pPrimarySocket,
                       OsDatagramSocket* pPrimarySocket2,
                       OsDatagramSocket* pSecondarySocket,
                       OsDatagramSocket* pSecondarySocket2);


    /**
     * Destructor
     */
    virtual ~TestStunServerTask();

/* ============================ MANIPULATORS ============================== */

    /** 
     * Entry point for the task 
     */
    virtual int run(void* pArg) ;

    void setEventValidator(EventValidator* pEventValidator) ;

    void setTestMode(STUN_TEST_MODE testMode) ;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:      
    OsDatagramSocket* mpPrimarySocket ;
    OsDatagramSocket* mpPrimarySocket2 ;
    OsDatagramSocket* mpSecondarySocket ;
    OsDatagramSocket* mpSecondarySocket2 ;

    EventValidator* mpEventValidator ;
    STUN_TEST_MODE mTestMode ;
    int miRequestNumber ;

    virtual void handleStunMessage(const char* pBuf, int nBuf, const UtlString& fromAddress, unsigned short fromPort) ;

    virtual void handleStunBindRequest(StunMessage* pMsg, const UtlString& fromAddress, unsigned short fromPort) ;

    virtual void handleStunSharedSecretRequest(StunMessage* pMsg, const UtlString& fromAddress, unsigned short fromPort) ;

    virtual void sendResponse(StunMessage* pMsg, OsDatagramSocket* pSocket, const UtlString& toAddress, unsigned short toPort) ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

    /** Copy constructor */
    TestStunServerTask(const TestStunServerTask& rTestStunServerTask);

    /** Assignment operator */
    TestStunServerTask& operator=(const TestStunServerTask& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _TestStunServerTask_h_
