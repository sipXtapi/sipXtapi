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

#ifndef _IOsNatSocket_h_
#define _IOsNatSocket_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsDateTime.h"
#include "os/OsMsg.h"
#include "utl/UtlString.h"

// DEFINES
#define NAT_MSG_TYPE            (OsMsg::USER_START + 1) /**< Stun Msg type/Id */
#define MAX_RTP_BYTES           16384  /**< Max RTP packet/read Size */

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef enum OS_NAT_PACKET_TYPE 
{
    UNKNOWN_PACKET,
    MEDIA_PACKET,
    STUN_PROBE_PACKET,
    STUN_DISCOVERY_PACKET,
    TURN_PACKET,
    CRLF_KEEPALIVE_PACKET,
    STUN_KEEPALIVE_PACKET,
    OTHER_PACKET
};

/**
 * Possible roles that a Media connection can have.
 */
 typedef enum RTP_TRANSPORT
 {
   RTP_TRANSPORT_UNKNOWN    = 0x00000000,
   RTP_TRANSPORT_UDP        = 0x00000001,
   RTP_TRANSPORT_TCP        = 0x00000002,

   RTP_TCP_ROLE_ACTIVE      = 0x00000004,
   RTP_TCP_ROLE_PASSIVE     = 0x00000008,
   RTP_TCP_ROLE_ACTPASS     = 0x00000010,
   RTP_TCP_ROLE_CONNECTION  = 0x00000020,

   RTP_TRANSPORT_ARS        = 0x00000040
} RTP_TRANSPORT;

typedef int RtpTransportOptions;
typedef int RtpTcpRoles;

class OsSocket;
class OsNotification; 

typedef enum TURN_FRAMING_TYPE
{
    STUN = 0x02,
    DATA = 0x03
} TURN_FRAMING_TYPE ;

/**
 * Generic interface defining the Nat APIs for a socket.  The 
 * OsNatSocketBaseImpl and OsNatDatagramSocket/OsNatConnetionSocket 
 * implement this interface.
 */
class IOsNatSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    friend class OsNatAgentTask ;
    friend class OsNatAgentContext ;
    friend class OsNatAgentTurnContext ;

/* ============================ CREATORS ================================== */

    /**
     * virtual destructor.
     */
    virtual ~IOsNatSocket() {};

    /** 
     * Cleanup routine.
     */
    virtual void destroy() = 0;

/* ============================ MANIPULATORS ============================== */
            
    /**
     * Standard write - should be used to invoke the base class's write method
     */
    virtual int socketWrite(const char* buffer, int bufferLength,
                      const char* ipAddress, int port, OS_NAT_PACKET_TYPE packetType=UNKNOWN_PACKET) = 0;    



    /**
     * When a stun packet is received this socket can either call read again
     * to obtain the next packet or return with zero bytes read.  By default
     * the socket will transparently call Read again and will block until a
     * non-stun packet is read. Calling this method will effect the next 
     * read -- in other words it will not unblock an active read.
     *
     * @param bEnable True to enable transparent stun reads and block until
     *        a non-stun packet is received (default) or False to return 
     *        with zero bytes read if a stun packet is received.
     */ 
    virtual void setTransparentStunRead(bool bEnable) = 0 ;

    /** 
     * Enable STUN.  Enabling STUN will reset the the keep alive timer and 
     * will force a binding refresh.
     *
     * @param szStunServer
     * @param stunPort
     * @param iKeepAlive
     * @param stunOptions
     * @param bReadFromSocket
     */
    virtual void enableStun(const char* szStunServer, 
                            int stunPort,
                            int iKeepAlive, 
                            int stunOptions, 
                            bool bReadFromSocket,
                            int iNCPeriod = -1) = 0;

    virtual UtlBoolean getMappedIp(UtlString* ip, int* port) = 0;
        
    /** 
     * Enable STUN.  Enabling STUN will reset the the keep alive timer and 
     * will force a binding refresh.
     *
     * @param szTurnSever
     * @param turnPort
     * @param iKeepAlive
     * @param username
     * @param password
     * @param bReadFromSocket
     */
    virtual void enableTurn(const char* szTurnSever,
                            int turnPort,
                            int iKeepAlive,
                            const char* username,
                            const char* password,  
                            bool bReadFromSocket) = 0;

    virtual void disableTurn(bool bForce = true) = 0 ;

    virtual UtlBoolean getTurnIp(UtlString* ip, int* port) = 0;

    /**
      * Prepares a destination under TURN usage.
      */
    virtual void primeTurnDestination(const char* szAddress, int iPort) = 0;        

    /**
     * TODO: DOCS
     */
    virtual UtlBoolean setTurnDestination(const char* szAddress, int iPort) = 0;


    /**
     * Add an alternate destination to this OsNatDatagramSocket.  Alternate 
     * destinations are tested by sending stun packets.  If a stun response is
     * received and the priority is greater than what has already been selected
     * then that address is used.
     * 
     * @param szAddress IP address of the alternate destination
     * @param iPort port number of the alternate destination
     * @param priority priority of the alternate where a higher number 
     *        indicates a higher priority.
     */
    virtual void addAlternateDestination(const char* szAddress, int iPort, int priority) = 0;

/* ============================ ACCESSORS ================================= */                           

    /**
     * Get the OsSocket derived socket
     */
    virtual OsSocket* getSocket() = 0;


    /**
     * Wait for the best destination to become available
     */
    virtual bool waitForBestDestination(bool       bLongWait,
                                        UtlString& address,
                                        int&       iPort,
                                        bool&      bViaOurRelay,
                                        int&       priority) = 0 ;
    /**
     * TODO: DOCS
     */
    virtual UtlBoolean getBestDestinationAddress(UtlString& address, 
                                                 int&       iPort, 
                                                 bool&      bViaOurRelay, 
                                                 int&       priority) = 0;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    /**
     * Report that a stun attempt failed.
     */
    virtual void markStunFailure() = 0;


    /**
     * Report that a stun attempt succeeded.
     */
    virtual void markStunSuccess(bool bAddressChanged) = 0;


    /**
     * Report that a stun attempt failed.
     */
    virtual void markTurnFailure() = 0;


    /**
     * Report that a stun attempt succeeded.
     */
    virtual void markTurnSuccess() = 0;

    /** 
     * Sets the STUN-resolved address and port for this socket.
     *
     * @param address String containing the stunned address.
     * @param iPort integer value of the stunned port.     
     */
    virtual void setStunAddress(const UtlString& address, const int iPort) = 0;
    
    /**
     * Set the TURN-dervied relay address for this socket. 
     *
     * @param address TURN-derived hostname/IP address
     * @param iPort TURN-derived port address
     */
    virtual void setTurnAddress(const UtlString& address, const int iPort) = 0;

    /**
     * Reset the destination address for this OsNatDatagramSocket.  This
     * method is called by the OsStunAgentTask when a better address is 
     * found via STUN/ICE.
     *
     * @param address The new destination address
     * @param port The new destination port
     * @param priority Priority of the destination address
     */
    virtual void evaluateDestinationAddress(const UtlString& address, int iPort, int priority, bool bViaRelay) = 0;    
};


#endif // #ifndef _IOsNatSocket_h_
