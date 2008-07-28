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

#ifndef _OsNatDatagramSocket_h_
#define _OsNatDatagramSocket_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsNatSocketBaseImpl.h"
#include "os/OsDatagramSocket.h"
#include "os/OsMsgQ.h"
#include "os/OsTimer.h"
#include "os/OsQueuedEvent.h"
#include "utl/UtlHashMap.h"

// DEFINES
#define NAT_MSG_TYPE         (OsMsg::USER_START + 1) /**< Stun Msg type/Id */

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef enum
{
   NO_BINDING,
   STUN_BINDING,
   TURN_BINDING,
   STUN_TURN_BINDING
} NAT_BINDING;

// FORWARD DECLARATIONS
class NatMsg ;
class OsEvent ;
class OsNatAgentTask;
class OsNatKeepaliveListener;
class OsNotification;

/**
 * OsNatDatagramSocket extends an OsDatagramSocket by adding an integrated
 * STUN and TURN client.  If STUN or TURN is enabled, request will be send 
 * to the designated server every refresh period.  The external addresses 
 * obtained by these mechanisms are retrieved by invoking getMappedIp and
 * getTurnIp.
 *
 * For this mechanism to work, someone must pump inbound socket data by 
 * calling one of the ::read() methods.  Otherwise, the packets will not be 
 * received/processed.  Internally, the implemenation peeks at the read 
 * data and passes the message to the OsNatAgentTask for processing.
 */
class OsNatDatagramSocket : public OsDatagramSocket, public OsNatSocketBaseImpl
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    friend class OsNatAgentTask ;

/* ============================ CREATORS ================================== */
    /**
     * Constructor accepting the remote host port, name, localhost port, 
     * name, and stun attributes.
     *
     * @param remoteHostPort Port of the remote host for connection-like use of
     *        OsDatagramSocket.  Alternatively, you can also use the ::WriteTo
     *        method and include the host/port at sent time.
     * @param remoteHostName Hostname of remote host for a connection-like use
     *        of OsDatagramSocket.
     * @param localHostPort Local port number for the socket,
     *        PORT_DEFAULT to autoselect
     * @param localHostName Local host name for the socket (e.g. which interface
     *        to bind on.
     * @param pNotification Optional notification event that is signaled upon
     *        the initial successful stun response or on failure.
     */
    OsNatDatagramSocket(int remoteHostPort, 
                        const char* remoteHostName, 
                        int localHostPort = PORT_DEFAULT,
                        const char* localHostName = NULL,
                        OsNotification* pStunNotification = NULL) ;

    /**
     * Standard Destructor
     */
    virtual ~OsNatDatagramSocket();

    virtual void destroy();

/* ============================ MANIPULATORS ============================== */
    
    /**
     * Standard read, see OsDatagramSocket for details.
     */
    virtual int read(char* buffer, int bufferLength) ;

    /**
     * Standard read, see OsDatagramSocket for details.
     */
    virtual int read(char* buffer, int bufferLength,
            UtlString* ipAddress, int* port);

    /**
     * Standard read, see OsDatagramSocket for details.
     */
    virtual int read(char* buffer, int bufferLength,
            struct in_addr* ipAddress, int* port);

    /**
     * Standard read, see OsDatagramSocket for details.
     */
    virtual int read(char* buffer, int bufferLength, long waitMilliseconds);


    /**
     * Standard write, see OsDatagramSocket for details.
     */
    virtual int write(const char* buffer, int bufferLength);

    /**
     * Standard write - used to invoke the base class write,
     * without timestamping.
     */
    virtual int socketWrite(const char* buffer, int bufferLength,
                      const char* ipAddress, int port, OS_NAT_PACKET_TYPE packetType=UNKNOWN_PACKET);
                      
    /**
     * Standard write, see OsDatagramSocket for details.
     */
    virtual int write(const char* buffer, int bufferLength,
                      const char* ipAddress, int port);

    /**
     * Standard write, see OsDatagramSocket for details.
     */
    virtual int write(const char* buffer, int bufferLength, 
                      long waitMilliseconds);

    /**
     * Sets read-based rate limiting at the socket layer to guard against
     * DOS attacks.  The remote client which places the socket over the max
     * rate limit is isolated for ignorePeriodMS.  If any packets are 
     * received by client during its isolation period -- its timer is 
     * restarted.
     * 
     *
     * @param readsPerSec number of reads/second allowed (0 to disable)
     * @param ignorePeriodSecs length of timeout/isolation period
     */
    virtual void setReadRateLimiting(int readsPerSec, int ignorePeriodSecs) ;

    /**
     * Waits for result of STUN/TURN binding on this socket. Must be
     * called after STUN/TURN is enabled and reading is not done from
     * socket.
     *
     * @param binding Binding type to wait for.
     * @param bWaitUntilReady Whether we should block until bindings are
     *        ready
     * @return True if this function needs to be called again, because
     *         some of bindings are not yet available. False if all requested
     *         bindings are ready.
     */
    virtual bool waitForBinding(NAT_BINDING binding, bool bWaitUntilReady);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    int  mRateLimit ;            // reads/seconds
    int  mIgnorePeriodSecs ;     // ignore period

    double mCurrentRate ;
    UtlHashMap mRateLimitedMap ; 

    double calculateRate(unsigned long delta, double oldRate) ;

    bool checkTotalRateLimit() ;
    bool checkHostRateLimit(UtlString* pIP) ;

    bool shouldRateLimit(UtlString* pIP, int* pPort) ;


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

/* ///////////////////////// HELPER CLASSES /////////////////////////////// */

#endif  // _OsNatDatagramSocket_h_

