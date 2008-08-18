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
// $$
//////////////////////////////////////////////////////////////////////////////
// Author: Bob Andreasen
// 

#ifndef _ARSCONNECTIONSOCKET_H
#define _ARSCONNECTIONSOCKET_H

#include "os/OsConnectionSocket.h"
// #include "os/IOsNatSocket.h"
#include "mediaInterface/IMediaSocket.h"

#define ARS_MAX_PACKET_SIZE   4096
#define ARS_FRAME_LEN_SIZE    2     // packet len in network byte order
#define ARS_FRAME_TYPE_SIZE   2     // packet type in network byte order
#define ARS_FRAMING_SIZE      (ARS_FRAME_LEN_SIZE+ARS_FRAME_TYPE_SIZE)

class IArsPacketHandler
{
public:
    virtual ~IArsPacketHandler() {} ;


    // processes unframed data (assumes first short is len in network byte
    // order).
    virtual bool handleData(char* pData, int len, int channel, bool& bMarkReadData) = 0 ;
} ;

//////////////////////////////////////////////////////////////////////////////

#define ARS__                                    1098
#define ARS__TLV_TAGS_SERVICE_UUID               1
#define ARS__TLV_TAGS__NUM_TAGS                  1
#define ARS__TLV_TAGS__MIN_TAG                   1
#define ARS__TLV_TAGS__MAX_TAG                   1
#define ARS__REQUESTDENIED_ERR_REDIRECT          0xFF00

#define ARS_TYPE_ERR            1
#define ARS_TYPE_LISTEN         2
#define ARS_TYPE_LISTEN_REPLY   3
#define ARS_TYPE_CONNECT        4
#define ARS_TYPE_CONNECT_ACK    5

//////////////////////////////////////////////////////////////////////////////

typedef enum 
{
    ARS_CONNECTION_IDLE,

    ARS_CONNECTION_HTTPS_PROXY_PREALLOCATE,
    ARS_CONNECTION_ALLOCATING,
    ARS_CONNECTION_LISTENING,

    ARS_CONNECTION_HTTPS_PROXY_PRECONNECT,       
    ARS_CONNECTION_CONNECTING,

    ARS_CONNECTION_CONNECTED,   // Everything above connected is considered a failure

    ARS_CONNECTION_PROXY_FAILURE,
    ARS_CONNECTION_PROTOCOL_ERROR,
    ARS_CONNECTION_NETWORK_ERROR,
} ARS_CONNECTION_STATE ;

typedef enum
{
    ARS_MIMETYPE_AUDIO,
    ARS_MIMETYPE_VIDEO
} ARS_MIMETYPE ;


typedef struct 
{
    unsigned char data[8] ;
} ARS_COOKIE ;

//////////////////////////////////////////////////////////////////////////////

class ArsConnectionSocket 
    : public OsConnectionSocket 
    , public IMediaSocket
{
protected:
    ARS_CONNECTION_STATE mState ;
    UtlString            mArsServer ;
    int                  mArsPort;
    UtlString            mUsername ;
    ARS_MIMETYPE         mArsMimeType ;
    ARS_COOKIE           mCookie ;

    UtlString            mFusionAddress ;
    int                  mFusionSlot ;
    bool                 mbEnabled ;
    unsigned short       mLOLength ;
    unsigned short       mLOBytes ;
    char                 mLOBuffer[ARS_MAX_PACKET_SIZE + ARS_FRAME_TYPE_SIZE] ;
    bool                 mbPartialLength ;
    unsigned short       mPartialLength ;

    UtlString            mHttpsProxyAddress ;
    int                  mHttpsProxyPort ;
    UtlString            mHttpsProxyUsername ;
    UtlString            mHttpsProxyPassword ;

    IArsPacketHandler*   mpPacketHandler ;

    int  mPacketsSent ;          // Send/Recv Stats
    int64_t mBytesSent ;         // Send/Recv Stats
    int  mPacketsRecv ;          // Send/Recv Stats
    int64_t mBytesRecv ;         // Send/Recv Stats
       
public:
    ArsConnectionSocket(const char*  szArsServer, 
                        int          iArsPort, 
                        const char*  szUsername,
                        ARS_MIMETYPE mimetype,
                        const char*  szLocalIp) ;
    virtual ~ArsConnectionSocket() ;

    void setHttpsProxy(const char* szProxy, int proxyPort, const char* szUsername, const char *szPassword) ;

    bool listenArs(ARS_COOKIE cookie) ;
    bool isArsFusionAvailable() ;
    bool getArsFusion(UtlString& fusionAddress, int& fusionSlot) ;
    bool connectArs(const char* szFusionAddress, int iFusionSlot, ARS_COOKIE cookie) ;
    bool isArsConnected() ;
    bool isArsFailed() ;
    void setPacketHandler(IArsPacketHandler* pPacketHandler) ;
    bool isUsingHttpsProxy() const ;

    // IMediaSocket impl
    virtual bool pushPacket() ;
    virtual void setEnabled(bool bEnabled) ;
    virtual OsSocket* getSocket()
        { return this; } ;

    // OsConnectionSocket Impl
    virtual int write(const char* buffer,
                     int bufferLength,
                     const char* ipAddress,
                     int port) ;
    virtual int write(const char* buffer, int bufferLength) ;

    virtual int write(const char* buffer, int bufferLength, long waitMilliseconds) ;

    virtual bool getSendRecvStats(int&     rPacketsSent,
                                  int64_t& rBytesSent,
                                  int&     rPacketsRecv,
                                  int64_t  rBytesRecv) ;

protected:
    bool doSetNoDelay() ;

    bool doListen() ;
    bool doProcessListenResponse(const char* pBuf, int nBuf) ;

    bool doConnect() ;
    bool doProcessConnectResponse(const char* pBuf, int nBuf) ;

    bool doHttpsProxyConnect() ;
    bool doProcessHttpsProxyResponse(const char* pBuf, int nBuf) ;

    void setState(ARS_CONNECTION_STATE state);
} ;

//////////////////////////////////////////////////////////////////////////////

#endif


