//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsStunAgentTask_h_	/* [ */
#define _OsStunAgentTask_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "os/OsStunDatagramSocket.h"
#include "os/OsRpcMsg.h"
#include "os/OsEventMsg.h"
#include "utl/UtlHashMap.h"

// DEFINES
#define SYNC_MSG_TYPE (OsMsg::USER_START + 2)      /**< Synchronized Msg type/id */

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
 * The OsStunAgentTask is responsible for servicing all stun requests and
 * and responses on behalf of the OsStunDatagramSocket.  This handles the 
 * stun requests/responses however relies on someone else to pump sockets.
 *
 * Use cases:
 *
 *   1) Send a STUN request via a supplied OsStunDatagramSocket
 *   2) Process responses from a OsStunDatagramSocket
 *   3) Process server requests from a OsStunDatagramSocket
 */
class OsStunAgentTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

private:

    /**
     * Private constructor, use getInstance() 
     */
    OsStunAgentTask();

    /**
     * Private destuctor, use freeInstance() ;
     */
    virtual ~OsStunAgentTask();

public:
    /**
     * Obtain a singleton instance
     */
    static OsStunAgentTask* getInstance() ;

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



    /**
     * Send a stun request to the designated stun server and port and signal
     * the specified notification object when complete.
     */
    UtlBoolean sendStunDiscoveryRequest(OsStunDatagramSocket* pSocket,
                                        const UtlString& stunServer,
                                        const int stunPort,
                                        const int stunOption) ;


    UtlBoolean sendStunConnectivityRequest(OsStunDatagramSocket* pSocket,
                                           const UtlString& stunServer,
                                           int iStunPort,
                                           unsigned char cPriority) ;

    /**
     * Synchronize with the OsStunAgentTask by posting a message to this event
     * queue and waiting for that message to be processed.  Do not call this
     * method from the OsStunAgentTask's thread context (will block forever).
     */
    void synchronize() ;

    
    /**
     * Remove any references for the given socket.  This is a cleanup method 
     * needed to referencing stale pointers.
     */
    void removeSocket(OsStunDatagramSocket* pSocket) ;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    /**
     * Signal either a positive or negative response (true or false) to the
     * for the specified socket.  This method is effectively a NOP if a NULL
     * event object was passed to sendStunRequest or someone invoked 
     * removeNotify().
     */
    void signalStunOutcome(OsStunDatagramSocket* pSocket, UtlBoolean bSuccess) ;

    /**
     * Handle a stun refresh event (timer).  The timer is actually created and
     * maintained by the OsStunDatagramSocket.
     */
    virtual UtlBoolean handleStunTimerEvent(OsEventMsg& rMsg) ;

    /**
     * Handle an inbound stun message.  The messages are handled to this thread
     * by the OsStunDatagramSocket whenever someone calls one of the read 
     * methods.
     */
    virtual UtlBoolean handleStunMessage(StunMsg& rMsg) ;

    /**
     * Handle a synchronization request.  Synchronization consists of sending
     * a message and waiting for that messsage to be processed.
     */
    virtual UtlBoolean handleSynchronize(OsRpcMsg& rMsg) ;


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    static OsStunAgentTask* spInstance ;    /**< Singleton instance */
    static OsMutex sLock ;                  /**< Lock for singleton accessors */
    UtlHashMap mResponseMap;                /**< Map of outstanding refresh requests */  
    UtlHashMap mConnectivityMap ;           /**< Map of outstanding connectivity probes */
    OsMutex mMapsLock ;                     /**< Lock for Notify and Connectiviy maps */
    UtlSList mTimerPool;                    /**< List of free timers available for use */
    
    /** Disabled copy constructor (not supported) */
    OsStunAgentTask(const OsStunAgentTask& rOsStunAgentTask);     

    /** Disabled equal operators (not supported) */
    OsStunAgentTask& operator=(const OsStunAgentTask& rhs);  
   
};

/* ============================ INLINE METHODS ============================ */

#endif  /* _OsStunAgentTask_h_ ] */
