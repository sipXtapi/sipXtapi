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
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _OsNatAgentTask_h_	/* [ */
#define _OsNatAgentTask_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/IOsNatSocket.h"
#include "os/OsNatKeepaliveListener.h"
#include "os/OsServerTask.h"
#include "os/OsRpcMsg.h"
#include "os/OsEventMsg.h"
#include "utl/UtlHashMap.h"
#include "os/TurnMessage.h"
#include "os/StunMessage.h"
#include "os/NatMsg.h"
#include "os/OsNatAgentContext.h"
#include "tapi/sipXtapi.h"

// DEFINES
#define SYNC_MSG_TYPE    (OsMsg::USER_START + 2)        /**< Synchronized Msg type/id */

#if defined(_DEBUG) || !defined(_WIN32)
#  define NAT_INITIAL_ABORT_COUNT               6       /** Abort after N times (first attempt) */
#  define NAT_PROBE_SOFTABORT_COUNT             4       /** Select route if successful sooner */
#  define NAT_PROBE_HARDABORT_COUNT             8       /** Abort STUN probes after N attempts */
#else
#  define NAT_INITIAL_ABORT_COUNT               6       /** Abort after N times (first attempt) */
#  define NAT_PROBE_SOFTABORT_COUNT             4       /** Select route if successful sooner */
#  define NAT_PROBE_HARDABORT_COUNT             8       /** Abort STUN probes after N attempts */
#endif
#define NAT_RESEND_ABORT_COUNT                  25      /** Fail after N times (refreshes) */

#define NAT_RESPONSE_TIMEOUT_MS_MIN             100     /** First wait time for no-response*/
#define NAT_RESPONSE_TIMEOUT_MS_START           100     /** Initial timeout value */
#define NAT_RESPONSE_TIMEOUT_MS_MAX             200     /** Max timeout for no-response */

#define NAT_DEFAULT_KEEPALIVE_SEC               27      /** Default keepalive for NAT probes */

#define NAT_FIND_BINDING_POOL_MS                50      /** poll delay for contact searchs */
#define NAT_BINDING_EXPIRATION_SECS             60      /** expiration for bindings if not renewed */
#define NAT_MAX_STURN_MSG_LENGTH                4096    /** Max length of a stun/turn msg */


// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


typedef enum
{
    NET_NOE_STUN_RESULTS,
    NET_NOE_STUN_FAILURE,
    NET_NOE_NAT_CLASSIFICATION
} OS_NOE_TYPE ;

class OsNatOutcomeEvent
{
public:
    OsNatOutcomeEvent()
    {
        mType = NET_NOE_STUN_FAILURE ;
        mClassification = NAT_CLASSIFICATION_SERVER_ERROR ;
    }


    OsNatOutcomeEvent(const SIPX_CONTACT_ADDRESS& contact)
    {
        mType = NET_NOE_STUN_RESULTS ;
        mContact = contact ;
        mClassification = NAT_CLASSIFICATION_SERVER_ERROR ;
    }

    OsNatOutcomeEvent(NAT_CLASSIFICATION_TYPE type)
    {
        mType = NET_NOE_NAT_CLASSIFICATION ;
        mClassification = type ;
    }

    OS_NOE_TYPE getType() const
    {
        return mType ;
    }

    bool getContact(SIPX_CONTACT_ADDRESS* pContact) const
    {
        bool bRC = false;

        if (pContact && mType == NET_NOE_STUN_RESULTS)
        {
            *pContact = mContact ;
            bRC = true ;            
        }

        return bRC ;
    }

    bool getClassification(NAT_CLASSIFICATION_TYPE* pClassification) const
    {
        bool bRC = false;

        if (pClassification && mType == NET_NOE_NAT_CLASSIFICATION)
        {
            *pClassification = mClassification ;
            bRC = true ;            
        }

        return bRC ;
    }

protected:
    OS_NOE_TYPE             mType ;
    SIPX_CONTACT_ADDRESS    mContact ;
    NAT_CLASSIFICATION_TYPE mClassification ;
} ;


/**
 * The OsNatAgentTask is responsible for servicing all stun requests and
 * and responses on behalf of the IOsNatSocket.  This handles the 
 * stun requests/responses however relies on someone else to pump sockets.
 *
 * Use cases:
 *
 *   1) Send a STUN request via a supplied IOsNatSocket
 *   2) Process responses from a IOsNatSocket
 *   3) Process server requests from a IOsNatSocket
 */
class OsNatAgentTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */

    friend class OsNatAgentContext;
    friend class OsNatAgentTurnContext;
    friend class OsNatAgentNCContext;

/* ============================ CREATORS ================================== */
private:
    /**
     * Private constructor, use getInstance() 
     */
    OsNatAgentTask();

    /**
     * Private destuctor, use freeInstance() ;
     */
    virtual ~OsNatAgentTask();

public:
    /**
     * Obtain a singleton instance
     */
    static OsNatAgentTask* getInstance() ;

    /**
     * Release/Free the singleton instance obtained by calling getInstance.
     * This method is included for clean shutdown of the system.
     */
    static void releaseInstance() ;

/* ============================ MANIPULATORS ============================== */

    /**
     * Standard OsServerTask message handler -- used to process timer 
     * messages for stun refreshes, reads, etc.
     */
    virtual UtlBoolean handleMessage(OsMsg& rMsg) ;

    UtlBoolean sendStunProbe(IOsNatSocket* pSocket,
                             const char*   emoteAddress,
                             int           remotePort,
                             const char*   relayAddress,
                             int           relayPort,
                             int           priority) ;

    UtlBoolean enableStun(IOsNatSocket* pSocket,
                          const UtlString&     stunServer,
                          int                  stunPort,                                      
                          const int            stunOptions,
                          int                  keepAlive) ;

    UtlBoolean disableStun(IOsNatSocket* pSocket) ;

    UtlBoolean enableTurn(IOsNatSocket* pSocket,
                          const UtlString& turnServer,
                          int iTurnPort,
                          int keepAliveSecs,
                          const UtlString& username,
                          const UtlString& password) ;

    UtlBoolean primeTurnReception(IOsNatSocket* pSocket,
                                  const char* szAddress,
                                  int iPort);

    UtlBoolean setTurnDestination(IOsNatSocket* pSocket,
                                  const char* szAddress,
                                  int iPort ) ;

    void disableTurn(IOsNatSocket* pSocket) ;

    UtlBoolean addCrLfKeepAlive(IOsNatSocket*           pSocket, 
                                const UtlString&        remoteIp,
                                int                     remotePort,
                                int                     keepAliveSecs,
                                OsNatKeepaliveListener* pListener) ;

    UtlBoolean removeCrLfKeepAlive(IOsNatSocket* pSocket,
                                   const UtlString&     serverIp,
                                   int                  serverPort) ;

    UtlBoolean addStunKeepAlive(IOsNatSocket*    pSocket, 
                                const UtlString&        remoteIp,
                                int                     remotePort,
                                int                     keepAliveSecs,
                                OsNatKeepaliveListener* pListener) ;

    UtlBoolean removeStunKeepAlive(IOsNatSocket* pSocket,
                                   const UtlString&     serverIp,
                                   int                  serverPort) ;

    UtlBoolean removeKeepAlives(IOsNatSocket* pSocket) ;

    UtlBoolean removeStunProbes(IOsNatSocket* pSocket) ;

    /**
     * Synchronize with the OsNatAgentTask by posting a message to this event
     * queue and waiting for that message to be processed.  Do not call this
     * method from the OsNatAgentTask's thread context (will block forever).
     */
    void synchronize() ;

    /**
     * Determines if probes of a higher priority are still outstanding
     */
    UtlBoolean areProbesOutstanding(IOsNatSocket* pSocket, int priority, bool bLongWait) ;

    /**
     * Dumps the probe results to the log file
     */
    void logProbeResults(IOsNatSocket* pSocket) ;

    /**
     * Does a binding of the designated type/server exist 
     */
    UtlBoolean doesBindingExist(IOsNatSocket*   pSocket,
                                NAT_AGENT_BINDING_TYPE type, 
                                const UtlString&       serverIp,
                                int                    serverPort) ;

    /**
     * Accessor for the timer object. 
     */
    OsTimer* getTimer(OsNatAgentContext* pBinding) ;

    void performNatClassification(IOsNatSocket*              pSocket, 
                                  const UtlString&           stunServer,
                                  int                        stunPort,
                                  int                        refreshPeriod,
                                  OsNatAgentContextListener* pListener) ;

    static void trace(OsSysLogPriority priority, const char* format, ...) ;


    /* ============================ ACCESSORS ================================= */

    /**
     * Look at all of the stun data structures and see if you can find a 
     * known back-route to the specified destination.
     */
    UtlBoolean findContactAddress(  const UtlString& destHost, 
                                    int              destPort, 
                                    UtlString*       pContactHost, 
                                    int*             pContactPort,
                                    int              iTimeoutMs = 0) ;

    /**
     * Add an external binding (used for findContactAddress)
     */
    void addExternalBinding(OsSocket*  pSocket,
                            UtlString  remoteAddress,
                            int        remotePort,
                            UtlString  contactAddress,
                            int        contactPort) ;

    void clearExternalBinding(OsSocket*  pSocket,
                              UtlString  remoteAddress,
                              int        remotePort,
                              bool       bOnlyIfEmpty = false) ;


    /**
     * Locate an external binding for the specified destination host/port.  
     * This API while block while wait for a result.
     */
    UtlBoolean findExternalBinding(const UtlString& destHost, 
                                   int              destPort, 
                                   UtlString*       pContactHost, 
                                   int*             pContactPort,
                                   int              iTimeoutMs = 0,
                                   UtlBoolean*      pTimedOut = NULL) ;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    virtual UtlBoolean handleTimerEvent(OsNatAgentContext* pContext, OsTimer* pTimer) ;

    virtual UtlBoolean doSendTurnSendRequest(OsNatAgentTurnContext* pContext, bool bNewTransaction) ;

    virtual UtlBoolean doSendTurnSetDestRequest(OsNatAgentTurnContext* pContext, bool bNewTransaction) ;

    /**
     * Handle an inbound Stun message.  The messages are handled to this 
     * thread by the IOsNatSocket whenever someone calls one of the 
     * read methods.
     */
    virtual UtlBoolean handleStunMessage(NatMsg& rMsg) ;


    /**
     * Handle an inbound Turn message.  The messages are handled to this 
     * thread by the IOsNatSocket whenever someone calls one of the 
     * read methods.
     */
    virtual UtlBoolean handleTurnMessage(NatMsg& rMsg) ;


    /**
     * Handle a synchronization request.  Synchronization consists of sending
     * a message and waiting for that messsage to be processed.
     */
    virtual UtlBoolean handleSynchronize(OsRpcMsg& rMsg) ;

    virtual UtlBoolean sendTurnSendRequest(IOsNatSocket* pSocket,
                                           const char*  szFinalAddress,
                                           int          iFinalPort,
                                           const char*  pPayload,
                                           int          nPayload) ;

    virtual UtlBoolean sendMessage(StunMessage* pMsg, 
                                   IOsNatSocket* pSocket, 
                                   const UtlString& toAddress, 
                                   unsigned short toPort,
                                   OS_NAT_PACKET_TYPE packetType = UNKNOWN_PACKET) ;

    OsNatAgentContext* getBinding(IOsNatSocket* pSocket, NAT_AGENT_BINDING_TYPE type) ;
    OsNatAgentContext* getBinding(OsNatAgentContext* pContext) ;
    OsNatAgentContext* getBinding(STUN_TRANSACTION_ID* pId) ;

    OsNatAgentTurnContext* getTurnBinding(STUN_TRANSACTION_ID* pId) ;
    OsNatAgentTurnContext* getTurnBinding(IOsNatSocket* pSocket, NAT_AGENT_BINDING_TYPE type) ;
    OsNatAgentTurnContext* getTurnBinding(OsNatAgentContext* pContext) ;

    OsNatAgentNCContext* getNCBinding(STUN_TRANSACTION_ID* pId) ;
    OsNatAgentNCContext* getNCBinding(IOsNatSocket* pSocket, NAT_AGENT_BINDING_TYPE type) ;
    OsNatAgentNCContext* getNCBinding(OsNatAgentContext* pContext) ;

    UtlBoolean doesProbeBindingExist(IOsNatSocket* pSocket, const char* szIp, int port, const char* szRelayIp, int relayPort) ;

    void destroyBinding(OsNatAgentContext* pBinding) ;

    void releaseTimer(OsTimer* pTimer) ;

    UtlBoolean sendStunRequest(OsNatAgentContext* pBinding, UtlBoolean bNewTransaction) ;
    
    UtlBoolean sendTurnRequest(OsNatAgentTurnContext* pBinding, UtlBoolean bNewTransaction) ;

    UtlBoolean armErrorTimer(OsNatAgentContext* pBinding) ;

    OsNatKeepaliveEvent populateKeepaliveEvent(OsNatAgentContext* pContext) ;

    void dumpContext(UtlString* pResults, OsNatAgentContext* pBinding) ;    

    void purgeTimers(OsTimer* pTimer, OsNatAgentContext* pContext) ;

    static UtlBoolean purgeMsgQCallback(const OsMsg& rMsg, void* pUserData1, void* pUserData2);

    void onStunRTT(long ms);
    void onTurnRTT(long ms);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    static OsNatAgentTask* spInstance ;  /**< Singleton instance */
    static OsMutex sLock ;               /**< Lock for singleton accessors */    

    UtlSList mTimerPool;                    /**< List of free timers available for use */
    UtlHashMap mContextMap ;
    OsMutex mMapsLock ;                     /**< Lock for Notify and Connectiviy maps */

    UtlSList  mExternalBindingsList ;
    OsRWMutex mExternalBindingMutex ;
    
    
    /** Disabled copy constructor (not supported) */
    OsNatAgentTask(const OsNatAgentTask& rOsNatAgentTask);     

    /** Disabled equal operators (not supported) */
    OsNatAgentTask& operator=(const OsNatAgentTask& rhs);  

protected:
    static int sStunTimeoutMS ;          /**< Starting stun timeout in MS */
    static int sTurnTimeoutMS ;          /**< Starting turn timeout in MS */

   
};

/* ============================ INLINE METHODS ============================ */

#endif  /* _OsNatAgentTask_h_ ] */
