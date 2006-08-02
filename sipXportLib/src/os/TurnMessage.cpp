//
// Copyright (C) 2006 Robert J. Andreasen, Jr.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#if defined(__pingtel_on_posix__)
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netdb.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#endif

// APPLICATION INCLUDES
#include "os/OsSocket.h"
#include "os/OsDatagramSocket.h"
#include "os/TurnMessage.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* /////////////////////////s/// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
TurnMessage::TurnMessage()
    : StunMessage()
{
    mszTurnData = NULL ;
    reset() ;
}

// Destructor
TurnMessage::~TurnMessage()
{
    if (mszTurnData)
    {
        free(mszTurnData) ;
    }
}

/* ============================ MANIPULATORS ============================== */

void TurnMessage::reset() 
{
    StunMessage::reset() ;

    mLifetime = 0 ;
    mbLifetimeValid = false ;
    mBandwidth = 0 ;
    mbBandwidthValid = false ;
    memset(&mDestinationAddress, 0, sizeof(STUN_ATTRIBUTE_ADDRESS)) ;
    mbDestinationAddressValid = false ;
    memset(&mTurnRemoteAddress, 0, sizeof(STUN_ATTRIBUTE_ADDRESS)) ;
    mbTurnRemoteAddressValid = false ;
    if (mszTurnData)
    {
        free(mszTurnData) ;
    }
    mszTurnData = NULL ;
    mnTurnData = 0 ;
    mbTurnDataValid = false ;
    memset(&mRelayAddress, 0, sizeof(STUN_ATTRIBUTE_ADDRESS)) ;
    mbRelayAddressValid = false ;
    mTransport = 0 ;
    mbTransportValid = false ;
    memset(&mRequestedIp, 0, sizeof(STUN_ATTRIBUTE_ADDRESS)) ;
    mbRequestedIpValid = false ;
    setIncludeMessageIntegrity(true) ;
}


bool TurnMessage::encodeBody(char* pBuf, size_t nBufLength, size_t& nBytesUsed) 
{
    bool bError = false ;
    char* pTraverse = pBuf ;
    size_t nBytesLeft = nBufLength ;

    if (!bError)
    {
        bError = !encodeAttributeLong(ATTR_TURN_MAGIC_COOKIE, 
                ATTR_MAGIC_COOKIE, pTraverse, nBytesLeft) ;
    }

    if ((!bError) && mbLifetimeValid)
    {
        bError = !encodeAttributeLong(ATTR_TURN_LIFETIME, mLifetime, 
                pTraverse, nBytesLeft) ;
    }
   
    if ((!bError) && mbBandwidthValid)
    {
        bError = !encodeAttributeLong(ATTR_TURN_BANDWIDTH, mBandwidth, 
                pTraverse, nBytesLeft) ;
    }

    if ((!bError) && mbDestinationAddressValid)
    {
        bError = !encodeAttributeAddress(ATTR_TURN_DESTINATION_ADDRESS, 
                    &mDestinationAddress, pTraverse, nBytesLeft) ;
    }

    if ((!bError) && mbTurnRemoteAddressValid)
    {
        bError = !encodeAttributeAddress(ATTR_TURN_REMOTE_ADDRESS, 
                    &mTurnRemoteAddress, pTraverse, nBytesLeft) ;
    }

    if ((!bError) && mbTurnDataValid)
    {
        bError = !encodeAttributeRaw(ATTR_TURN_DATA, mszTurnData, mnTurnData,
                pTraverse, nBytesLeft) ;
    }

    if ((!bError) && mbRelayAddressValid)
    {
        bError = !encodeAttributeAddress(ATTR_TURN_RELAY_ADDRESS, 
                &mRelayAddress, pTraverse, nBytesLeft) ;
    }

    if ((!bError) && mbTransportValid)
    {
        bError = !encodeAttributeLong(ATTR_TURN_REQUESTED_TRANSPORT, 
                mTransport, pTraverse, nBytesLeft) ;
    }

    if ((!bError) && mbRequestedIpValid)
    {
        bError = !encodeAttributeAddress(ATTR_TURN_REQUESTED_IP, 
                &mRequestedIp, pTraverse, nBytesLeft) ;
    }

      
    // Stun attributes must be last (e.g. message-integrity).
    size_t nStunBytesUsed = 0 ;
    if (!bError)
    {
        bError = StunMessage::encodeBody(pTraverse, nBytesLeft, nStunBytesUsed) ;
    }

    nBytesUsed = (nBufLength - nBytesLeft) + nStunBytesUsed ;  

    return bError ;
}


void TurnMessage::setLifetime(unsigned long secs) 
{
    mLifetime = secs ;
    mbLifetimeValid = true ;
}


void TurnMessage::setBandwidth(unsigned long rKBPS) 
{
    mBandwidth = rKBPS ;
    mbBandwidthValid = true ;
}

void TurnMessage::setDestinationAddress(const char* szIp, unsigned short port) 
{
    mDestinationAddress.family = ATTR_ADDRESS_FAMILY_IPV4 ;
    mDestinationAddress.address = ntohl(inet_addr(szIp)) ;
    mDestinationAddress.port = port ;
    mbDestinationAddressValid = true ;
}

void TurnMessage::setTurnRemoteAddress(const char* szIp, unsigned short port) 
{
    mTurnRemoteAddress.family = ATTR_ADDRESS_FAMILY_IPV4 ;
    mTurnRemoteAddress.address = ntohl(inet_addr(szIp)) ;
    mTurnRemoteAddress.port = port ;
    mbTurnRemoteAddressValid = true ;
}

void TurnMessage::setData(const char* pData, unsigned short nLength) 
{
    if (mszTurnData)
    {
        free(mszTurnData) ;
    }

    mszTurnData = (char*) malloc(nLength) ;
    memcpy(mszTurnData, pData, nLength) ;
    mnTurnData = nLength ;
    mbTurnDataValid = true ;
}


void TurnMessage::setRelayAddress(const char* szIp, unsigned short port) 
{
    mRelayAddress.family = ATTR_ADDRESS_FAMILY_IPV4 ;
    mRelayAddress.address = ntohl(inet_addr(szIp)) ;
    mRelayAddress.port = port ;
    mbRelayAddressValid = true ;
}


void TurnMessage::setRequestedTransport(TURN_TRANSPORT_TYPE transportType)
{
    mTransport = (unsigned long) transportType ;
    mbTransportValid = true ;

    assert( (mTransport == 0) || (mTransport == 1) )  ;
}


void TurnMessage::setRequestedIp(const char* szIp, unsigned short port) 
{
    mRequestedIp.family = ATTR_ADDRESS_FAMILY_IPV4 ;
    mRequestedIp.address = ntohl(inet_addr(szIp)) ;
    mRequestedIp.port = port ;
    mbRequestedIpValid = true ;
}


/* ============================ ACCESSORS ================================= */

bool TurnMessage::getLifetime(unsigned long& rSecs) 
{
    if (mbLifetimeValid)
    {
        rSecs = mLifetime ;
    } 

    return mbLifetimeValid ;    
}

bool TurnMessage::getBandwidth(unsigned long& rKBPS) 
{
    if (mbBandwidthValid)
    {
        rKBPS = mBandwidth ;
    } 

    return mbBandwidthValid ;        
}


bool TurnMessage::getDestinationAddress(char* szIp, unsigned short& rPort) 
{
    if (mbDestinationAddressValid)
    {
        unsigned long address = htonl(mDestinationAddress.address) ;
        strcpy(szIp, inet_ntoa(*((in_addr*) &address))) ;        
        rPort = mDestinationAddress.port ;          
    } 

    return mbDestinationAddressValid ;
}

bool TurnMessage::getTurnRemoteAddress(char* szIp, unsigned short& rPort) 
{
    if (mbTurnRemoteAddressValid)
    {
        unsigned long address = htonl(mTurnRemoteAddress.address) ;
        strcpy(szIp, inet_ntoa(*((in_addr*) &address))) ;        
        rPort = mTurnRemoteAddress.port ;          
    } 

    return mbTurnRemoteAddressValid ;
}

bool TurnMessage::getData(char*& rpData, unsigned short& nLength) 
{
    if (mbTurnDataValid)
    {
        rpData = mszTurnData ;
        nLength = (unsigned short) mnTurnData ;
    }

    return mbTurnDataValid ;        
}


bool TurnMessage::getRelayAddress(char* szIp, unsigned short& rPort) 
{
    if (mbRelayAddressValid)
    {
        unsigned long address = htonl(mRelayAddress.address) ;
        strcpy(szIp, inet_ntoa(*((in_addr*) &address))) ;        
        rPort = mRelayAddress.port ;          
    } 

    return mbRelayAddressValid ;
}


bool TurnMessage::getRequestedTransport(TURN_TRANSPORT_TYPE& rTransportType)
{
    if (mbTransportValid)
    {
        rTransportType = (TURN_TRANSPORT_TYPE) mTransport ;
    }

    return mbTransportValid ;
}


bool TurnMessage::getRequestedIp(char* szIp, unsigned short& rPort)
{
    if (mbRequestedIpValid)
    {
        unsigned long address = htonl(mRequestedIp.address) ;
        strcpy(szIp, inet_ntoa(*((in_addr*) &address))) ;        
        rPort = mRequestedIp.port ;          
    } 

    return mbRequestedIpValid ;
}


/* ============================ INQUIRY =================================== */

bool TurnMessage::validateMessageType(unsigned short type) 
{
    bool bValid = false ;

    switch (type)
    {
        case MSG_TURN_ALLOCATE_REQUEST:
        case MSG_TURN_ALLOCATE_RESPONSE:
        case MSG_TURN_ALLOCATE_ERROR_RESPONSE:
        case MSG_TURN_SEND_REQUEST:
        case MSG_TURN_SEND_RESPONSE:
        case MSG_TURN_SEND_ERROR_RESPONSE:
        case MSG_TURN_DATA_INDICATION:
        case MSG_TURN_ACTIVE_DESTINATION_REQUEST:
        case MSG_TURN_ACTIVE_DESTINATION_RESPONSE:
        case MSG_TURN_ACTIVE_DESTINATION_ERROR_RESPONSE:
        case MSG_TURN_CONNECTION_STATUS_INDICATION:
        case MSG_TURN_CLOSE_BINDING_REQUEST:
        case MSG_TURN_CLOSE_BINDING_RESPONSE:
        case MSG_TURN_CLOSE_BINDING_ERROR_RESPONSE:
            bValid = true ;
            break ;
        default:
            bValid = StunMessage::validateMessageType(type) ;
            break ;
    }

    return bValid ;
}


bool TurnMessage::isTurnMessage(const char* pBuf, unsigned short nBufLength) 
{
    bool bValid = false ;

    if (nBufLength >= (sizeof(STUN_MESSAGE_HEADER) + 
            sizeof(STUN_ATTRIBUTE_HEADER) + sizeof(unsigned long)))
    {
        STUN_MESSAGE_HEADER header ;
        STUN_ATTRIBUTE_HEADER attrHeader ;
        unsigned long magicCookie ;
        char* pTraverse = (char*) pBuf ;

        // Copy header
        memcpy(&header, pTraverse, sizeof(STUN_MESSAGE_HEADER)) ;
        header.type = ntohs(header.type) ;
        header.length = ntohs(header.length) ;        
        
        // Valid Length
        if (nBufLength == (sizeof(STUN_MESSAGE_HEADER) + header.length))
        {
            switch (header.type)
            {
                case MSG_TURN_ALLOCATE_REQUEST:
                case MSG_TURN_ALLOCATE_RESPONSE:
                case MSG_TURN_ALLOCATE_ERROR_RESPONSE:
                case MSG_TURN_SEND_REQUEST:
                case MSG_TURN_SEND_RESPONSE:
                case MSG_TURN_SEND_ERROR_RESPONSE:
                case MSG_TURN_DATA_INDICATION:
                case MSG_TURN_ACTIVE_DESTINATION_REQUEST:
                case MSG_TURN_ACTIVE_DESTINATION_RESPONSE:
                case MSG_TURN_ACTIVE_DESTINATION_ERROR_RESPONSE:
                case MSG_TURN_CONNECTION_STATUS_INDICATION:
                case MSG_TURN_CLOSE_BINDING_REQUEST:
                case MSG_TURN_CLOSE_BINDING_RESPONSE:
                case MSG_TURN_CLOSE_BINDING_ERROR_RESPONSE:

                    // Validate Magic Cookie
                    pTraverse += sizeof(STUN_MESSAGE_HEADER) ;
                    memcpy(&attrHeader, pTraverse, sizeof(STUN_ATTRIBUTE_HEADER)) ;
                    attrHeader.type = ntohs(attrHeader.type) ;
                    attrHeader.length = ntohs(attrHeader.length) ;
                    pTraverse += sizeof(STUN_ATTRIBUTE_HEADER) ;
                    memcpy(&magicCookie, pTraverse, sizeof(unsigned long)) ;
                    magicCookie = ntohl(magicCookie) ;

                    if (    (attrHeader.type == ATTR_TURN_MAGIC_COOKIE) &&
                            (attrHeader.length == sizeof(unsigned long)) &&
                            (magicCookie == ATTR_MAGIC_COOKIE)  )
                    {                                        
                        bValid = true ;
                    }
                    break ;
                default:
                    break ;
            }
        }
    }

    return bValid ;
}

bool TurnMessage::isRequestOrNonErrorResponse() 
{
    bool bRequestOrNonErrorResponse ;

    switch (getType())
    {
        case MSG_TURN_ALLOCATE_REQUEST:
        case MSG_TURN_ALLOCATE_RESPONSE:    
        case MSG_TURN_SEND_REQUEST:
        case MSG_TURN_SEND_RESPONSE:       
        case MSG_TURN_ACTIVE_DESTINATION_REQUEST:
        case MSG_TURN_ACTIVE_DESTINATION_RESPONSE: 
        case MSG_TURN_CLOSE_BINDING_REQUEST:
        case MSG_TURN_CLOSE_BINDING_RESPONSE:
            bRequestOrNonErrorResponse = true ;
            break ;
        case MSG_TURN_ALLOCATE_ERROR_RESPONSE:
        case MSG_TURN_SEND_ERROR_RESPONSE:
        case MSG_TURN_DATA_INDICATION:
        case MSG_TURN_ACTIVE_DESTINATION_ERROR_RESPONSE:
        case MSG_TURN_CONNECTION_STATUS_INDICATION:
        case MSG_TURN_CLOSE_BINDING_ERROR_RESPONSE:
            break ;
            bRequestOrNonErrorResponse = false ;
            break ;
        default:
            bRequestOrNonErrorResponse = StunMessage::isRequestOrNonErrorResponse() ;
            break ;
    }

    return bRequestOrNonErrorResponse ;
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

bool TurnMessage::parseAttribute(STUN_ATTRIBUTE_HEADER* pHeader, char* pBuf) 
{
    unsigned long magicCookie ;
    bool bValid = false ;

    switch (pHeader->type)
    {
        case ATTR_TURN_LIFETIME:
            bValid = parseLongAttribute(pBuf, pHeader->length, &mLifetime) ;
            mbLifetimeValid = bValid ;
            break ;
        case ATTR_TURN_MAGIC_COOKIE:
            bValid = parseLongAttribute(pBuf, pHeader->length, &magicCookie) ;
            // Validate Cookie
            if (bValid)
            {
                bValid = (magicCookie == ATTR_MAGIC_COOKIE) ;
            }            
            break ;
        case ATTR_TURN_BANDWIDTH:
            bValid = parseLongAttribute(pBuf, pHeader->length, &mBandwidth) ;
            mbBandwidthValid = bValid ;            
            break ;
        case ATTR_TURN_DESTINATION_ADDRESS:
            bValid = parseAddressAttribute(pBuf, pHeader->length, &mDestinationAddress) ;
            mbDestinationAddressValid = bValid ;
            break ;
        case ATTR_TURN_REMOTE_ADDRESS:
            bValid = parseAddressAttribute(pBuf, pHeader->length, &mTurnRemoteAddress) ;
            mbTurnRemoteAddressValid = bValid ;
            break ;
        case ATTR_TURN_DATA:
            if (pHeader->length)
            {
                mszTurnData = (char*) malloc(pHeader->length) ;
                if (mszTurnData)
                {
                    bValid = parseRawAttribute(pBuf, pHeader->length, mszTurnData, pHeader->length) ;
                    mbTurnDataValid = bValid ;
                    if (!bValid)
                    {
                        free(mszTurnData) ;
                        mszTurnData = NULL ;
                    }
                }
            }
            break ;
        case ATTR_TURN_RELAY_ADDRESS:
            bValid = parseAddressAttribute(pBuf, pHeader->length, &mRelayAddress) ;
            mbRelayAddressValid = bValid ;
            break ;
        case ATTR_TURN_REQUESTED_TRANSPORT:
            bValid = parseLongAttribute(pBuf, pHeader->length, &mTransport) ;
            mbTransportValid = bValid ;            
            break ;
        case ATTR_TURN_REQUESTED_IP:
            bValid = parseAddressAttribute(pBuf, pHeader->length, &mRequestedIp) ;
            mbRequestedIpValid = bValid ;
            break ;
        default:
            bValid = StunMessage::parseAttribute(pHeader, pBuf) ;
            break ;
    }

    return bValid ;
}

bool TurnMessage::encodeAttributeLong(unsigned short type, unsigned long value, char*& pBuf, size_t& nBytesLeft)
{
    bool bRC = false ;

    if (    (nBytesLeft >= (sizeof(unsigned long) + sizeof(STUN_ATTRIBUTE_HEADER))) &&
            encodeAttributeHeader(type, sizeof(unsigned long), pBuf, nBytesLeft) &&
            encodeLong(value, pBuf, nBytesLeft))
    {
        bRC = true ;
    }

    return bRC ;
}

bool TurnMessage::encodeAttributeRaw(unsigned short type, const char* cBuf, size_t nBuf, char*& pBuf, size_t& nBytesLeft) 
{
    bool bRC = false;

    size_t nTotalLength = sizeof(STUN_ATTRIBUTE_HEADER) + nBuf ;
    if (    (nBytesLeft >= nTotalLength) &&
            encodeAttributeHeader(type, (unsigned short) nBuf, pBuf, nBytesLeft) &&
            encodeRaw(cBuf, nBuf, pBuf, nBytesLeft))
    {
        bRC = true ;
    }

    return bRC ;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


