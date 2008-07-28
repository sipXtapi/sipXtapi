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

#include <assert.h>

#include "os/OsSysLog.h"
#include "ARS/ArsConnectionSocket.h"
#include "ARS/SnacCodec.h"
#include "net/HttpMessage.h"

#if defined(_VXWORKS)
#   include <socket.h>
#   include <resolvLib.h>
#   include <netinet/ip.h>
#elif defined(__pingtel_on_posix__) || defined(__MACH__)
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <arpa/inet.h>
#endif


//////////////////////////////////////////////////////////////////////////////

#define ARS_GUID_LENGTH 16

//////////////////////////////////////////////////////////////////////////////

static const unsigned char cAudioGUID[ARS_GUID_LENGTH] =
{
    0x09, 0x46, 0x01, 0x04,0x4C, 0x7F,0x11, 0xD1, 0x82,0x22,0x44,0x45,0x53,0x54,0x00,0x00
} ;

static const unsigned char cVideoGUID[ARS_GUID_LENGTH] =
{
    0x09, 0x46, 0x01, 0x01,0x4C, 0x7F,0x11, 0xD1, 0x82,0x22,0x44,0x45,0x53,0x54,0x00,0x00
} ;


//////////////////////////////////////////////////////////////////////////////


ArsConnectionSocket::ArsConnectionSocket(const char*  szArsServer, 
                                         int          iArsPort, 
                                         const char*  szUsername,
                                         ARS_MIMETYPE mimetype,
                                         const char*  szLocalIp)
    : OsConnectionSocket(szLocalIp, -1)
{
    setState(ARS_CONNECTION_IDLE) ;
    mArsServer = szArsServer ;
    mArsPort = iArsPort ;
    mUsername = szUsername ;
    mArsMimeType = mimetype ;
    memset(&mCookie, 0, sizeof(ARS_COOKIE)) ;
    mpPacketHandler = NULL ;

    mFusionAddress.remove(0) ;
    mFusionSlot = 0 ;
    mbEnabled = true ;
    mLOLength = 0 ; 
    mLOBytes = 0 ;
    mbPartialLength = false ;
    mPartialLength = 0;
    mHttpsProxyPort = 0 ;
}

//////////////////////////////////////////////////////////////////////////////

ArsConnectionSocket::~ArsConnectionSocket()
{    
}

//////////////////////////////////////////////////////////////////////////////

void ArsConnectionSocket::setHttpsProxy(const char* szProxy, int proxyPort, const char* szUsername, const char *szPassword) 
{
    mHttpsProxyAddress = szProxy ;
    mHttpsProxyPort = proxyPort ;
    mHttpsProxyUsername = szUsername ;
    mHttpsProxyPassword = szPassword ;
}

//////////////////////////////////////////////////////////////////////////////

bool ArsConnectionSocket::listenArs(ARS_COOKIE cookie)
{
    bool bRC = false ;
    assert(mState == ARS_CONNECTION_IDLE) ;
    if (mState == ARS_CONNECTION_IDLE)
    {        
        memcpy(&mCookie, &cookie, sizeof(ARS_COOKIE)) ;

        if (mHttpsProxyAddress.length())
        {
            bRC = initialize(mHttpsProxyAddress.data(), mHttpsProxyPort, true) ;
            assert(bRC == true) ;      
            bRC = (OsConnectionSocket::connect() == 0)  ;
            if (bRC && isConnected() && isOk())
            {
                setState(ARS_CONNECTION_HTTPS_PROXY_PREALLOCATE) ;
                doSetNoDelay() ;    // failure isn't fatal
                bRC = doHttpsProxyConnect() ;
            }
            else
            {
                setState(ARS_CONNECTION_NETWORK_ERROR) ;
                OsSysLog::add(FAC_ARS, PRI_ERR, "%08X Failed to initiate proxy socket connection to %s:%d, errno=%d",
                        this, mHttpsProxyAddress.data(), mHttpsProxyPort, OsSocketGetERRNO()) ;                
            }            
        }
        else
        {
            bRC = initialize(mArsServer.data(), mArsPort, true) ;
            assert(bRC == true) ;      
            bRC = (OsConnectionSocket::connect() == 0)  ;
            if (bRC && isConnected() && isOk())
            {
                doSetNoDelay() ;    // failure isn't fatal
                bRC = doListen() ;
            }
            else
            {
                setState(ARS_CONNECTION_NETWORK_ERROR) ;
                OsSysLog::add(FAC_ARS, PRI_ERR, "%08X Failed to initiate socket connection to %s:%d, errno=%d",
                        this, mArsServer.data(), mArsPort, OsSocketGetERRNO()) ;                
            }
        }
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

bool ArsConnectionSocket::isArsFusionAvailable() 
{
    return (mState == ARS_CONNECTION_LISTENING || mState == ARS_CONNECTION_CONNECTED) ;
}


//////////////////////////////////////////////////////////////////////////////

bool ArsConnectionSocket::getArsFusion(UtlString& fusionAddress, int& fusionSlot) 
{
    bool bRC = false ;

    if (mState == ARS_CONNECTION_LISTENING || mState == ARS_CONNECTION_CONNECTED) 
    {
        fusionAddress = mFusionAddress ;
        fusionSlot = mFusionSlot ;
        bRC = true ;
    }
    else
    {
        fusionAddress.remove(0) ;
        fusionSlot = 0 ;
    }

    return bRC ;        
}

//////////////////////////////////////////////////////////////////////////////

bool ArsConnectionSocket::connectArs(const char* szFusionAddress, int iFusionSlot, ARS_COOKIE cookie)
{
    bool bRC = false ;
    assert(mState == ARS_CONNECTION_IDLE) ;
    if (mState == ARS_CONNECTION_IDLE)
    {
        memcpy(&mCookie, &cookie, sizeof(ARS_COOKIE)) ;

        // Reset server to proxy, but keep proxy port
        mArsServer = szFusionAddress ;

        mFusionAddress = szFusionAddress ; 
        mFusionSlot = iFusionSlot ;

        if (mHttpsProxyAddress.length())
        {
            bRC = initialize(mHttpsProxyAddress.data(), mHttpsProxyPort, true) ;
            assert(bRC == true) ;
            bRC = (OsConnectionSocket::connect() == 0) ;
            if (bRC && isConnected() && isOk())
            {                
                setState(ARS_CONNECTION_HTTPS_PROXY_PRECONNECT) ;
                doSetNoDelay() ;    // failure isn't fatal
                bRC = doHttpsProxyConnect() ;
            }
            else
            {
                setState(ARS_CONNECTION_NETWORK_ERROR) ;
                OsSysLog::add(FAC_ARS, PRI_ERR, "%08X Failed to initiate proxy socket connection to %s:%d, errno=%d",
                        this, mHttpsProxyAddress.data(), mHttpsProxyPort, OsSocketGetERRNO()) ;                
            }

        }
        else
        {
            bRC = initialize(mArsServer.data(), mArsPort, true) ;
            assert(bRC == true) ;
            bRC = (OsConnectionSocket::connect() == 0) ;
            if (bRC && isConnected() && isOk())
            {
                doSetNoDelay() ;    // failure isn't fatal
                bRC = doConnect() ;
            }
            else
            {
                setState(ARS_CONNECTION_NETWORK_ERROR) ;
                OsSysLog::add(FAC_ARS, PRI_ERR, "%08X Failed to initiate socket connection to %s:%d/%d, errno=%d",
                        this, mArsServer.data(), mArsPort, mFusionSlot, OsSocketGetERRNO()) ;                
            }
        }
    }
    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

bool ArsConnectionSocket::isArsConnected()
{
    return (mState == ARS_CONNECTION_CONNECTED) ;
}

//////////////////////////////////////////////////////////////////////////////

bool ArsConnectionSocket::isArsFailed()
{
    return (mState > ARS_CONNECTION_CONNECTED) ;
}

//////////////////////////////////////////////////////////////////////////////

void ArsConnectionSocket::setPacketHandler(IArsPacketHandler* pPacketHandler) 
{
    mpPacketHandler = pPacketHandler ;
}

//////////////////////////////////////////////////////////////////////////////

bool ArsConnectionSocket::pushPacket()
{
    bool bSuccess = true ;

    switch (mState)
    {
        case ARS_CONNECTION_IDLE:
        case ARS_CONNECTION_ALLOCATING:
            // Listening Response
            {
                char cTemp[1024] ;
                int size = read(cTemp, sizeof(cTemp)) ;
                if (size > 0)
                    doProcessListenResponse(cTemp, size) ;
                else if (size < 0)
                {
                    setState(ARS_CONNECTION_NETWORK_ERROR);
                    bSuccess = false ;
                    close() ;
                }
            }
            break ;
        case ARS_CONNECTION_LISTENING:
        case ARS_CONNECTION_CONNECTING:
            // Connected Response
            {
                char cTemp[1024] ;
                int size = read(cTemp, sizeof(cTemp)) ;
                if (size > 0)
                    doProcessConnectResponse(cTemp, size) ;
                else if (size < 0)
                {
                    setState(ARS_CONNECTION_NETWORK_ERROR);
                    bSuccess = false ;
                    close() ;
                }
            }
            break ;
        case ARS_CONNECTION_CONNECTED:    

            // Check for data hanging out from a previous read
            if (mLOLength)
            {
                assert((mLOLength-mLOBytes) < sizeof(mLOBuffer)) ;
                int size = read(&mLOBuffer[mLOBytes], mLOLength-mLOBytes) ;
                if (size > 0)
                {
                    mLOBytes += size ;
                    if (mLOBytes == mLOLength)
                    {
                        bool bMarkLastRead = false ;
                        if (mpPacketHandler && mbEnabled)
                            mpPacketHandler->handleData(mLOBuffer, mLOLength, getChannel(), bMarkLastRead) ;
                        if (bMarkLastRead)
                            markReadTime() ;
                        mLOBytes = 0 ;
                        mLOLength = 0 ;
                    }
                }
                else if (size < 0)
                {
                    setState(ARS_CONNECTION_NETWORK_ERROR);
                    bSuccess = false ;
                    close() ;
                }
            }
            else
            {
                unsigned short len = 0 ;
                int size = 0 ;

                // Check for partial length read
                if (mbPartialLength)
                {
                    size = read(&((char*) &mPartialLength)[1], 1) ;
                    if (size == 1)
                    {
                        size = sizeof(mPartialLength) ;
                        len = mPartialLength ;
                        mPartialLength = 0 ;
                        mbPartialLength = false ;
                    }
                    else if (size < 0)
                    {
                        setState(ARS_CONNECTION_NETWORK_ERROR);
                        bSuccess = false ;
                        close() ;
                    }
                }
                else
                {                    
                    size = read((char*) &len, sizeof(len)) ;
                }

                if (size == 1)
                {
                    mPartialLength = len ;
                    mbPartialLength = true ;
                }
                else if (size == sizeof(len))
                {
                    len = ntohs(len) ;
                    size = read(mLOBuffer, len) ;
                    if (size > 0)
                    {
                        if (size != len)
                        {
                            mLOLength = len ;
                            mLOBytes = size ;
                        }
                        else
                        {
                            bool bMarkLastRead = false ;
                            if (mpPacketHandler && mbEnabled)
                                mpPacketHandler->handleData(mLOBuffer, len, getChannel(), bMarkLastRead) ;
                            if (bMarkLastRead)
                                markReadTime() ;
                        }
                    }
                }
                else if (size < 0)
                {
                    setState(ARS_CONNECTION_NETWORK_ERROR);
                    bSuccess = false ;
                    close() ;
                }
            }
            break ;
        case ARS_CONNECTION_HTTPS_PROXY_PREALLOCATE:
        case ARS_CONNECTION_HTTPS_PROXY_PRECONNECT:
            {
                char cTemp[2048] ;
                int size = read(cTemp, sizeof(cTemp)) ;
                if (size > 0)
                    doProcessHttpsProxyResponse(cTemp, size) ;
                else if (size < 0)
                {
                    setState(ARS_CONNECTION_NETWORK_ERROR);
                    bSuccess = false ;
                    close() ;
                }
            }
            break ;
        case ARS_CONNECTION_PROTOCOL_ERROR:
        case ARS_CONNECTION_NETWORK_ERROR:
        case ARS_CONNECTION_PROXY_FAILURE:
            bSuccess = false ;
            close();
            break ;
        default:
            assert(FALSE) ;
            break ;
    }

    return bSuccess ;
}

//////////////////////////////////////////////////////////////////////////////

void ArsConnectionSocket::setEnabled(bool bEnabled)
{
    mbEnabled = bEnabled ;
}

//////////////////////////////////////////////////////////////////////////////

bool ArsConnectionSocket::doSetNoDelay()
{
    int boolVal = 1 ;
    int rc = setsockopt(getSocketDescriptor(), IPPROTO_TCP, TCP_NODELAY, (char*) &boolVal, sizeof(boolVal)) ;
    assert(rc == 0) ;
    if (rc != 0)
        OsSysLog::add(FAC_ARS, PRI_ERR, "%08X Failed to set TCP_NODELAY on socket: errno=%d", 
                this, OsSocketGetERRNO()) ;

    return (rc == 0) ;
}

//////////////////////////////////////////////////////////////////////////////

bool ArsConnectionSocket::doListen()
{
    bool bRC = false ;

    SnacCodec encoder ;
    encoder.appendShort(0xFFFF) ;           // Length
    encoder.appendShort(ARS__) ;            // Group
    encoder.appendShort(ARS_TYPE_LISTEN) ;  // Type
    encoder.appendShort(0) ;                // Flags
    encoder.appendLong(0) ;                 // reqId
    encoder.appendPString(mUsername.data()) ;   // username
    encoder.appendBlob((const unsigned char*) mCookie.data, sizeof(mCookie.data)) ;  // cookie
    encoder.appendShort(ARS__TLV_TAGS_SERVICE_UUID) ; // UUID type
    encoder.appendShort(ARS_GUID_LENGTH) ;  // len of mime type
    encoder.appendBlob(mArsMimeType == ARS_MIMETYPE_AUDIO ? cAudioGUID : cVideoGUID, ARS_GUID_LENGTH) ;
    encoder.updateShort(0, encoder.getLength()-2) ;

    if (isConnected() && isOk())
    {
        setState(ARS_CONNECTION_ALLOCATING);

        OsSysLog::add(FAC_ARS, PRI_INFO, "%08X Initiating Listen request to %s:%d (user=%s, type=%s)",
                this, mArsServer.data(), mArsPort, mUsername.data(), 
                mArsMimeType == ARS_MIMETYPE_AUDIO ? "audio" : "video") ;

        int len = write(encoder.getData(), encoder.getLength()) ;
        bRC = (len  == encoder.getLength()) ;
        if (!bRC)
        {
            setState(ARS_CONNECTION_NETWORK_ERROR) ;
            OsSysLog::add(FAC_ARS, PRI_ERR, "%08X Failed to initiate listen request (send failed) to %s:%d (user=%s, type=%s), errno=%d",
                    this,
                    mArsServer.data(), mArsPort, mUsername.data(), 
                    mArsMimeType == ARS_MIMETYPE_AUDIO ? "audio" : "video",
                    OsSocketGetERRNO()) ;
        }
    }
    else
    {
        setState(ARS_CONNECTION_NETWORK_ERROR) ;
        OsSysLog::add(FAC_ARS, PRI_ERR, "%08X Failed to initiate listen request (socket problem) to %s:%d (user=%s, type=%s)",
                this,
                mArsServer.data(), mArsPort, mUsername.data(), 
                mArsMimeType == ARS_MIMETYPE_AUDIO ? "audio" : "video") ;        
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

bool ArsConnectionSocket::doConnect()
{
    bool bRC = false ;

    SnacCodec encoder ;
    encoder.appendShort(0xFFFF) ;            // Length
    encoder.appendShort(ARS__) ;             // Group
    encoder.appendShort(ARS_TYPE_CONNECT) ;  // Type
    encoder.appendShort(0) ;                 // Flags
    encoder.appendLong(0) ;                  // reqId
    encoder.appendPString(mUsername.data()) ;   // username
    encoder.appendShort(mFusionSlot) ;
    encoder.appendBlob(mCookie.data, sizeof(mCookie.data)) ;  // cookie
    encoder.appendShort(ARS__TLV_TAGS_SERVICE_UUID) ; // UUID type
    encoder.appendShort(ARS_GUID_LENGTH) ;  // len of mime type
    encoder.appendBlob(mArsMimeType == ARS_MIMETYPE_AUDIO ? cAudioGUID : cVideoGUID, 16) ;
    encoder.updateShort(0, encoder.getLength()-2) ;

    if (isConnected() && isOk())
    {
        setState(ARS_CONNECTION_CONNECTING) ;

        OsSysLog::add(FAC_ARS, PRI_INFO, "%08X Initiating Connect request to %s:%d/%d (user=%s, type=%s)",
                this,
                mFusionAddress.data(), mArsPort, mFusionSlot, mUsername.data(), 
                mArsMimeType == ARS_MIMETYPE_AUDIO ? "audio" : "video") ;

        int len = write(encoder.getData(), encoder.getLength()) ;
        bRC = (len  == encoder.getLength()) ;
        if (!bRC)
        {
            setState(ARS_CONNECTION_NETWORK_ERROR) ;
            OsSysLog::add(FAC_ARS, PRI_ERR, "%08X Failed to initiate connect request (send failed) to %s:%d/%d (user=%s, type=%s), errno=%d",
                    this,
                    mArsServer.data(), mArsPort, mFusionSlot, mUsername.data(), 
                    mArsMimeType == ARS_MIMETYPE_AUDIO ? "audio" : "video",
                    OsSocketGetERRNO()) ;
        }
    }
    else
    {
        setState(ARS_CONNECTION_NETWORK_ERROR) ;
        OsSysLog::add(FAC_ARS, PRI_ERR, "%08X Failed to initiate listen request (socket problem) to %s:%d/%d (user=%s, type=%s)",
                this,
                mArsServer.data(), mArsPort, mFusionSlot, mUsername.data(), 
                mArsMimeType == ARS_MIMETYPE_AUDIO ? "audio" : "video") ;
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

bool ArsConnectionSocket::doProcessListenResponse(const char* pBuf, int nBuf)
{
    bool bRC = false ;

    SnacCodec decoder(pBuf, nBuf) ;

    unsigned short len ;
    unsigned short group ;
    unsigned short type ;
    unsigned short flags ;
    unsigned long  reqId ;

    if (decoder.getShort(len) &&
        decoder.getShort(group) &&
        decoder.getShort(type) &&
        decoder.getShort(flags) &&
        decoder.getLong(reqId))
    {
        if ((group == ARS__) && (type == ARS_TYPE_LISTEN_REPLY))
        {
            unsigned short slot ;
            unsigned long  ip ;

            if (decoder.getShort(slot) && decoder.getLong(ip))
            {
                char cIp[100] ;
                ip = htonl(ip) ;
                strcpy(cIp, inet_ntoa(*((in_addr*) &ip))) ;
                mFusionAddress = cIp ;
                mFusionSlot = slot ;
                setState(ARS_CONNECTION_LISTENING) ;

                OsSysLog::add(FAC_ARS, PRI_INFO, "%08X fusion address: %s/%d",
                        this,
                        mFusionAddress.data(),
                        mFusionSlot) ;

                bRC = true ;
            }
            else
            {
                UtlString data ;
                decoder.hexdump(data) ;
                OsSysLog::add(FAC_ARS, PRI_ERR, "%08X Unable to parse listen response snac:\n%s", 
                        this, data.data()) ;
                setState(ARS_CONNECTION_PROTOCOL_ERROR) ;
            }
        }
        else
        {
            UtlString data ;
            decoder.hexdump(data) ;
            OsSysLog::add(FAC_ARS, PRI_ERR, "%08X Unexpected/Error listen response: group=0x%02X, type=0x%02X\n%s",
                    this, group, type, data.data()) ;
            
            setState(ARS_CONNECTION_PROTOCOL_ERROR) ;
        }
    }
    else
    {
        UtlString data ;
        decoder.hexdump(data) ;
        OsSysLog::add(FAC_ARS, PRI_ERR, "%08X Unable to parse listen response snac:\n%s", 
                this, data.data()) ;
        setState(ARS_CONNECTION_PROTOCOL_ERROR) ;
    }
    
    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

bool ArsConnectionSocket::doHttpsProxyConnect()
{
    HttpMessage msg ;

    UtlString uri ;

    uri.format("%s:%d", mArsServer.data(), mArsPort) ;

    msg.setFirstHeaderLine(HTTP_CONNECT_METHOD, uri, HTTP_PROTOCOL_VERSION) ;
    msg.setUserAgentField("sipXtapi") ;

    if (mHttpsProxyUsername.length())
    {
        msg.setBasicAuthorization(mHttpsProxyUsername, mHttpsProxyPassword, HttpMessage::SERVER) ;
        msg.setBasicAuthorization(mHttpsProxyUsername, mHttpsProxyPassword, HttpMessage::PROXY) ;
    }

    UtlString requestData;
    int requestLen;

    msg.getBytes(&requestData, &requestLen) ;

    OsSysLog::add(FAC_ARS, PRI_INFO, "%08X Initiating proxy connect request via %s:%d to %s:%d (user=%s, type=%s)",
                this,
                mHttpsProxyAddress.data(), mHttpsProxyPort,
                mArsServer.data(), mArsPort, mUsername.data(), 
                mArsMimeType == ARS_MIMETYPE_AUDIO ? "audio" : "video") ;

    int len = write(requestData.data(), requestLen) ;
    bool bRC = (len  == requestLen) ;
    if (!bRC)
    {
        setState(ARS_CONNECTION_NETWORK_ERROR) ;
        OsSysLog::add(FAC_ARS, PRI_ERR, "%08X Failed to initiate proxy connect (send failed) to %s:%d (user=%s, type=%s), errno=%d",
                this,
                mArsServer.data(), mArsPort, mUsername.data(), 
                mArsMimeType == ARS_MIMETYPE_AUDIO ? "audio" : "video",
                OsSocketGetERRNO()) ;
    }

    return true ;
}

//////////////////////////////////////////////////////////////////////////////

bool ArsConnectionSocket::doProcessHttpsProxyResponse(const char* pBuf, int nBuf)
{
    HttpMessage msg(pBuf, nBuf) ;    

    int responseCode = msg.getResponseStatusCode() ;
    UtlString responseText ;

    msg.getResponseStatusText(&responseText) ;
    if (responseCode == 200)
    {
        OsSysLog::add(FAC_ARS, PRI_INFO, "%08X https proxy connected: %d/%s",
                this, responseCode, responseText.data()) ;

        switch (mState)
        {
            case ARS_CONNECTION_HTTPS_PROXY_PREALLOCATE:
                doListen() ;
                break ;
            case ARS_CONNECTION_HTTPS_PROXY_PRECONNECT:
                doConnect() ;
                break ;
            default:
                assert(FALSE) ;
                break ;
        }
    }
    else
    {
        OsSysLog::add(FAC_ARS, PRI_ERR, "%08X https proxy connect failed: %d/%s",
                this, responseCode, responseText.data()) ;

        setState(ARS_CONNECTION_PROXY_FAILURE) ;
    }

    return (responseCode == 200) ;
}

//////////////////////////////////////////////////////////////////////////////

bool ArsConnectionSocket::doProcessConnectResponse(const char* pBuf, int nBuf)
{
    bool bRC = false ;

    SnacCodec decoder(pBuf, nBuf) ;

    unsigned short len ;
    unsigned short group ;
    unsigned short type ;
    unsigned short flags ;
    unsigned long  reqId ;

    if (decoder.getShort(len) &&
        decoder.getShort(group) &&
        decoder.getShort(type) &&
        decoder.getShort(flags) &&
        decoder.getLong(reqId))
    {
        if ((group == ARS__) && (type == ARS_TYPE_CONNECT_ACK))
        {
            setState(ARS_CONNECTION_CONNECTED) ;
            OsSysLog::add(FAC_ARS, PRI_INFO, "%08X Connected", this) ;
            bRC = true ;
        }
        else
        {
            setState(ARS_CONNECTION_PROTOCOL_ERROR) ;
            UtlString data ;
            decoder.hexdump(data) ;
            OsSysLog::add(FAC_ARS, PRI_ERR, "%08X Unexpected/Error connect response: group=0x%02X, type=0x%02X\n%s",
                    this, group, type, data.data()) ;            
        }
    }
    else
    {
        UtlString data ;
        decoder.hexdump(data) ;
        OsSysLog::add(FAC_ARS, PRI_ERR, "%08X Unable to parse connect response snac:\n%s", 
                this, data.data()) ;
        setState(ARS_CONNECTION_PROTOCOL_ERROR) ;
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

void ArsConnectionSocket::setState(ARS_CONNECTION_STATE state)
{
    if (mState != state)
    {        
        mState = state ;

        if (OsSysLog::willLog(FAC_ARS, PRI_INFO))
        {
            const char* szState = "ARS_CONNECTION_UNKNOWN" ;

            switch (mState)
            {
            case ARS_CONNECTION_IDLE:
                szState = "ARS_CONNECTION_IDLE" ;
                break ;
            case ARS_CONNECTION_HTTPS_PROXY_PREALLOCATE:
                szState = "ARS_CONNECTION_HTTPS_PROXY_PREALLOCATE" ;
                break ;
            case ARS_CONNECTION_ALLOCATING:
                szState = "ARS_CONNECTION_ALLOCATING" ;
                break ;
            case ARS_CONNECTION_LISTENING:
                szState = "ARS_CONNECTION_LISTENING" ;
                break ;
            case ARS_CONNECTION_HTTPS_PROXY_PRECONNECT:
                szState = "ARS_CONNECTION_HTTPS_PROXY_PRECONNECT" ;
                break ;
            case ARS_CONNECTION_CONNECTING:
                szState = "ARS_CONNECTION_CONNECTING" ;
                break ;
            case ARS_CONNECTION_CONNECTED:
                szState = "ARS_CONNECTION_CONNECTED" ;
                break ;
            case ARS_CONNECTION_PROXY_FAILURE:
                szState = "ARS_CONNECTION_PROXY_FAILURE" ;
                break ;
            case ARS_CONNECTION_PROTOCOL_ERROR:
                szState = "ARS_CONNECTION_PROTOCOL_ERROR" ;
                break ;
            case ARS_CONNECTION_NETWORK_ERROR:
                szState = "ARS_CONNECTION_NETWORK_ERROR" ;
                break ;
            }

            OsSysLog::add(FAC_ARS, PRI_INFO, "%08X State set to %s", 
                this, szState) ;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
