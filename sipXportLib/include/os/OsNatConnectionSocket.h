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

#ifndef _OsNatConnectionSocket_h_
#define _OsNatConnectionSocket_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsConnectionSocket.h"
#include "os/OsMsgQ.h"
#include "os/OsTimer.h"
#include "os/OsQueuedEvent.h"
#include "os/OsRWMutex.h"
#include "os/OsNatSocketBaseImpl.h"
#include "utl/UtlHashMap.h"
#include "os/OsNatDatagramSocket.h"


// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS

// TYPEDEFS
// FORWARD DECLARATIONS
class NatMsg ;
class OsEvent ;
class OsNatAgentTask;
class OsNatKeepaliveListener;


/**
 * OsNatConnectionSocket extends an OsConnectionSocket by adding an 
 * integrated TURN client.
 *
 * For this mechanism to work, someone must pump inbound socket data by 
 * calling one of the ::read() methods.  Otherwise, the packets will not be 
 * received/processed.  Internally, the implemenation peeks at the read 
 * data and passes the message to the OsNatAgentTask for processing.
 */
class OsNatConnectionSocket : public OsConnectionSocket, public OsNatSocketBaseImpl
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    friend class OsNatAgentTask ;

/* ============================ CREATORS ================================== */
    OsNatConnectionSocket(int connectedSocketDescriptor,
                          const RtpTcpRoles role);

    OsNatConnectionSocket(const char* szLocalIp,
                          int connectedSocketDescriptor,
                          const RtpTcpRoles role);

    // Constructor
    OsNatConnectionSocket(int serverPort,
                          const char* serverName,
                          UtlBoolean blockingConnect,
                          const char* localIp,
                          const bool bConnect,
                          const RtpTcpRoles role);

    /**
     * Standard Destructor
     */
    virtual ~OsNatConnectionSocket();

    virtual void destroy();

/* ============================ MANIPULATORS ============================== */

    virtual void setRole(const RtpTcpRoles role);    
    
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
     * Standard write, see OsDatagramSocket for details.
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
     * Not applicable for a connection-oriented socket
     */
    virtual void enableStun(const char* szStunServer, 
                            int stunPort,
                            int iKeepAlive, 
                            int stunOptions, 
                            bool bReadFromSocket) 
        { assert(false) ;} ;

    /**
     * Not applicable for a connection-oriented socket
     */
    virtual void disableStun()
        { assert(false) ;} ;


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
                            bool bReadFromSocket) ;


/* ============================ ACCESSORS ================================= */

    virtual const RtpTcpRoles getRole() const;

    /**
     * Applies framing to buffers sent over streaming connections
     * to a TURN server.
     * Framing prepends with a header like so:
     *
     *   0                   1                   2                   3
     *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *  |     Type      |  Reserved = 0 |            Length             |
     *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *
     * 
     * @param type Type of framed buffer - STUN or DATA
     * @param buffer Buffer to be framed.
     * @param bufferLenght Length of the buffer to be framed.
     * @param framedBufferLength Output parameter for the length of the 
     *        framed buffer.
     *
     * @returns Pointer to a newly allocated buffer.  Must be freed by caller.
     */
    const char* frameBuffer(TURN_FRAMING_TYPE type,
                            const char* buffer,
                            const int bufferLength,
                            int& framedBufferLen);
   
    virtual int clientConnect(const char* szServer, const int port);
    virtual bool isClientConnected(const char* szServer, const int port);
    virtual OsNatConnectionSocket* getClientConnection(const char* szServer, const int port);

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    /**
     * active, passive, or actpass
     */
    RtpTcpRoles mRole;
    
    mutable OsRWMutex mRoleMutex;
    mutable OsMutex mStreamHandlerMutex;

    void setStunAddress(const UtlString& address, const int iPort) 
        { assert(false) ;} ;

    void markStunFailure() 
        { assert(false) ;} ;

    void markStunSuccess(bool bAddressChanged) 
        { assert(false) ;} ;


    virtual void handleFramedStream(char*       pData,
                                    const int   size,
                                    const char* receivedIp,
                                    const int   port);
                                                      
    virtual bool handleUnframedBuffer(const TURN_FRAMING_TYPE type,
                                      const char* buff,
                                      const int buffSize,
                                      const char* receivedIp,
                                      const int port);                                                      
            
    void addClientConnection(const char* ipAddress, const int port, OsNatConnectionSocket* pClient);

    UtlHashMap mClientConnectionSockets;    // map of client connection sockets
                                            // key = destination IP address
                                            // value = OsNatConnectionSocket pointer

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    /* Global Attributes */
    char      mszFragment[(MAX_RTP_BYTES + 4)];
    int       mFragmentSize;
};


#endif  // _OsNatConnectionSocket_h_

