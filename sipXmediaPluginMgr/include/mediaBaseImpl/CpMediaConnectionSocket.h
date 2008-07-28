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
// Copyright (C) 2005-2008 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _CpMediaConnectionSocket_h_
#define _CpMediaConnectionSocket_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsNatConnectionSocket.h"
#include "os/OsMsgQ.h"
#include "os/OsTimer.h"
#include "os/IOsNatSocket.h"
#include "os/OsServerSocket.h"
#include "mediaInterface/IMediaSocket.h"
#include "mediaBaseImpl/CpMediaServer.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

//: CpMediaConnectionSocket creates a OsConnectionSocket and automatically initiates
//: stun lookups and keep-alives.  
class CpMediaConnectionSocket : public IMediaSocket, public OsNatConnectionSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
    CpMediaConnectionSocket(int type,
                              int remoteHostPort, 
                              const char* remoteHostName, 
                              int localHostPort = 0, 
                              const char* localHostName = NULL,
                              const char* szTurnServer = NULL,
                              const RtpTcpRoles role = RTP_TCP_ROLE_ACTPASS) ;

      //: Constructor accepting remote host port, name and optional local
      //: host name and port, and stun attributes.

    virtual ~CpMediaConnectionSocket();
       //:Destructor
         
/* ============================ MANIPULATORS ============================== */
    
/* ============================ ACCESSORS ================================= */

    void setRole(const RtpTcpRoles role) ;

    virtual int getSocketDescriptor() const ;
    virtual bool pushPacket();
    virtual void pushPacket(char* rtpPacket, size_t packetSize, int port) = 0;
    virtual int readPacket(char* pData, const int maxSize, UtlString& address, int& port);


    void setEnabled(bool bEnabled);
    
       
   virtual int write(const char* buffer,
                                       int bufferLength,
                                       const char* ipAddress,
                                       int port);
                                        
   virtual int write(const char* buffer, int bufferLength);
   //:Blocking write to the socket
   // Write the characters in the given buffer to the socket.
   // This method will block until all of the bytes are written.
   //!param: buffer - The bytes to be written to the socket.
   //!param: bufferLength - The number of bytes contained in buffer.
   //!returns: The number of bytes actually written to the socket.
   //!returns: <br>Note: This does not necessarily mean that the bytes were
   //!returns: actually received on the other end.
   
   bool handleUnframedBuffer(const TURN_FRAMING_TYPE type,
                             const char* buff,
                             const int buffSize,
                             const char* receivedIp,
                             const int port);   

    virtual OsSocket* getSocket()
        { return this ;} ;
    
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    bool validRtpPacket(int type,
                        const char* pBuf, 
                        int         nBuf,
                        const char* szFromAddress, 
                        int         iFromPort) ;

    void syncToSource(int         type,
                      RTP_HEADER* pHeader,
                      const char* szFromAddress, 
                      int         iFromPort) ;
                      
   virtual int connect();
   //: Sets up the initial connection with the server                      

#ifdef TRANSPORT_DUMP
    void transportDump(const char* szLabel,     
                       const char* pBuf,
                       int         nBuf) ;
#endif

    int miType ;
    OsMutex mEnabledMutex;
    mutable OsRWMutex mMediaServerMutex;
    RTP_MONITOR_CONEXT  mRtpContexts[4] ;
    bool mbEnabled ;
    CpMediaServer* mpMediaServer;

    static const int CHUNK_SIZE = 4096;
    char         mszStreamBuffer[(CHUNK_SIZE + 2) * 2];
    char         mszFragment[(CHUNK_SIZE + 2)];
    int          mFragmentSize;
    char         szFramedBuffer[CHUNK_SIZE + 2];    

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

/* ============================ INLINE METHODS ============================ */


#endif  // _CpMediaConnectionSocket_h_

