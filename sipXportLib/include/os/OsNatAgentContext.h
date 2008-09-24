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

#ifndef _OsNatAgentContext_h_
#define _OsNatAgentContext_h_

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

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef enum
{
    INVALID_BINDING_TYPE,

    STUN_DISCOVERY,
    STUN_PROBE,

    CRLF_KEEPALIVE,
    STUN_KEEPALIVE,

    TURN_ALLOCATION,
    TURN_SEND_REQUEST,
    TURN_SET_ACTIVE_DEST,

    NAT_CLASSIFICATION

} NAT_AGENT_BINDING_TYPE ;

typedef enum
{
    SUCCESS,
    SENDING,
    SENDING_ERROR,
    RESENDING,
    RESENDING_ERROR,
    FAILED,
} NAT_AGENT_STATUS ;


typedef enum
{
    NAT_FAILURE_UNKNOWN,
    NAT_FAILURE_TIMEOUT,
    NAT_FAILURE_PROTOCOL,
    NAT_FAILURE_NETWORK
} NAT_FAILURE_CAUSE ;


typedef struct 
{
    OsSocket*    pSocket ;
    UtlString    remoteAddress ;
    int          remotePort ;
    UtlString    contactAddress ;
    int          contactPort ;
    OsTime       expiration ;
} NAT_AGENT_EXTERNAL_CONTEXT ;


// FORWARD DECLARATIONS
class OsNatAgentTask;


class OsNatAgentContext
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    NAT_AGENT_BINDING_TYPE  type ;
    NAT_AGENT_STATUS        status ;
    UtlString               serverAddress ;
    int                     serverPort ;
    UtlString               relayAddress;
    int                     relayPort;
    int                     options ;
    STUN_TRANSACTION_ID     transactionId ;
    IOsNatSocket*           pSocket ;
    OsTimer*                pErrorTimer ;
    OsTimer*                pRefreshTimer ;
    int                     keepAliveSecs ;
    int                     abortCount ;
    int                     refreshErrors ;
    int                     errorTimeoutMSec ;
    UtlString               address ;
    int                     port ;
    int                     priority ;
    OsNatKeepaliveListener* pKeepaliveListener ;
    unsigned long           lStartTimestamp ;
    unsigned long           lEndTimestamp ;

/* ============================ CREATORS ================================== */

    OsNatAgentContext(OsNatAgentTask* pAgentTask) ;
    virtual ~OsNatAgentContext() ;

/* ============================ MANIPULATORS ============================== */

    virtual void markFailure(NAT_FAILURE_CAUSE cause) ;
    virtual void markSuccess(const UtlString& mappedAddress, 
                     int mappedPort, 
                     const UtlString& receivedIp, 
                     int receivedPort,
                     const UtlString& changeIp, 
                     int changePort) ;
    virtual void markTimeout() ;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    OsNatAgentTask* mpAgentTask ;

    void handleStunTimeout() ;
    bool handleCrLfKeepaliveTimeout() ;
    bool handleStunKeepaliveTimeout() ;


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

} ;


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


class OsNatAgentTurnContext
    : public OsNatAgentContext
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    UtlString username ;  
    UtlString password ;  
    char*     pPayload ;
    int       nPayload ;

/* ============================ CREATORS ================================== */

    OsNatAgentTurnContext(OsNatAgentTask* pAgentTask) ;
    virtual ~OsNatAgentTurnContext() ;

/* ============================ MANIPULATORS ============================== */

    virtual void markFailure(NAT_FAILURE_CAUSE cause) ;
    virtual void markSuccess(const UtlString& mappedAddress, 
                             int mappedPort, 
                             const UtlString& receivedIp, 
                             int receivedPort,
                             const UtlString& relayIp, 
                             int relayPort) ;
    virtual void markTimeout() ;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    void handleTurnAllocTimeout() ;
    void handleTurnSendRequestTimeout() ;
    void handleTurnSetDestTimeout() ;    

//////////////////////////////////////////////////////////////////////////////

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


} ;


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

typedef enum
{
    NAT_STAGE_TEST1,
    NAT_STAGE_FW_TEST2,
    NAT_STAGE_NAT_TEST2,
    NAT_STAGE_NAT_TEST1,
    NAT_STAGE_NAT_TEST3,
    NAT_STAGE_DONE
} NAT_CLASSIFICATION_STAGE ;

typedef enum 
{
    NAT_CLASSIFICATION_SERVER_ERROR,

    NAT_CLASSIFICATION_BLOCKED,

    NAT_CLASSIFICATION_OPEN,
    NAT_CLASSIFICATION_FIREWALL_SYMMETRIC,

    NAT_CLASSIFICATION_NAT_FULL_CONE,

    NAT_CLASSIFICATION_NAT_SYMMETRIC,
    NAT_CLASSIFICATION_NAT_RESTRICTED,
    NAT_CLASSIFICATION_NAT_PORT_RESTRICTED,

} NAT_CLASSIFICATION_TYPE ;



class OsNatAgentContextListener
{
public:
    virtual void OnClassificationComplete(NAT_CLASSIFICATION_TYPE type) = 0 ;
} ;


class OsNatAgentNCContext
    : public OsNatAgentContext
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    OsNatAgentNCContext(OsNatAgentTask* pAgentTask,
                        IOsNatSocket* pNatSocket,
                        OsNatAgentContextListener* pListener) ;
    virtual ~OsNatAgentNCContext() ;

/* ============================ MANIPULATORS ============================== */

    virtual void markFailure(NAT_FAILURE_CAUSE cause) ;
    virtual void markSuccess(const UtlString& mappedAddress, 
                             int mappedPort, 
                             const UtlString& receivedIp, 
                             int receivedPort,
                             const UtlString& changeIp, 
                             int changePort) ;
    virtual void markTimeout() ;

    virtual void start() ;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    NAT_CLASSIFICATION_STAGE mStage;
    OsNatAgentContextListener* mpListener;
    UtlString mChangeIp ;
    int       mChangePort ;
    OsSocket* mpOwnedSocket ;

    UtlString mOrigServer ;
    int       mOrigPort ;

    void setStage(NAT_CLASSIFICATION_STAGE stage) ;
    void setClassification(NAT_CLASSIFICATION_TYPE type) ;

    void doReset() ;
    void doSend() ;

//////////////////////////////////////////////////////////////////////////////

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


} ;




#endif  /* _OsNatAgentContext_h_ */
