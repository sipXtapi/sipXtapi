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
#include "os/StunMessage.h"
#include "os/TurnMessage.h"

// EXTERNAL FUNCTIONS
extern "C" void hmac_sha1(const char* pBlob, size_t nBlob, const char* pKey, size_t nKey, char* digest) ;
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
StunMessage::StunMessage()
{
    reset() ;
}

// Destructor
StunMessage::~StunMessage()
{
}

/* ============================ MANIPULATORS ============================== */

void StunMessage::reset() 
{
    memset(&mMsgHeader, 0, sizeof(STUN_MESSAGE_HEADER)) ;
    mbMsgHeaderValid = false ;
    memset(&mMappedAddress, 0, sizeof(STUN_ATTRIBUTE_ADDRESS)) ;
    mbMappedAddressValid = false ;
    memset(&mResponseAddress, 0, sizeof(STUN_ATTRIBUTE_ADDRESS)) ;
    mbResponseAddressValid = false ;
    memset(&mChangedAddress, 0, sizeof(STUN_ATTRIBUTE_ADDRESS)) ;
    mbChangedAddressValid = false ;
    mChangeRequest = 0 ;
    mbChangeRequestValid = false ;
    memset(&mSourceAddress, 0, sizeof(STUN_ATTRIBUTE_ADDRESS)) ;
    mbSourceAddressValid = false ;
    memset(&mUsername, 0, STUN_MAX_STRING_LENGTH+1) ;
    mbUsernameValid = false ;
    memset(&mPassword, 0, STUN_MAX_STRING_LENGTH+1) ;
    mbPasswordValid = false ;
    memset(&mMessageIntegrity, 0, STUN_MAX_MESSAGE_INTEGRITY_LENGTH) ;
    mbMessageIntegrityValid = false ;
    memset(&mError, 0, sizeof(STUN_ATTRIBUTE_ERROR)) ;
    mbErrorValid = false ;
    memset(&mUnknownAttributes, 0, sizeof(STUN_ATTRIBUTE_UNKNOWN)) ;
    mbUnknownAttributesValid = false ;
    memset(&mReflectedFrom, 0, sizeof(STUN_ATTRIBUTE_ADDRESS)) ;
    mbReflectedFromValid = false ;
    memset(&mServer, 0, STUN_MAX_STRING_LENGTH+1) ;
    mbServerValid = false ;
    mbSendXorOnly = true ;
    mbRequestXorOnly = true ;
    mbIncludeMessageIntegrity = false ;

    memset(&mUnknownParsedAttributes, 0, sizeof(STUN_ATTRIBUTE_UNKNOWN)) ;        
}


bool StunMessage::parse(const char* pBuf, size_t nBufLength)
{
    reset() ;

    bool bValid = false ;        
    if (nBufLength >= sizeof(STUN_MESSAGE_HEADER))
    {
        char* pTraverse = (char*) pBuf ;

        // Copy header
        memcpy(&mMsgHeader, pTraverse, sizeof(STUN_MESSAGE_HEADER)) ;
        pTraverse += sizeof(STUN_MESSAGE_HEADER) ;
        mMsgHeader.type = ntohs(mMsgHeader.type) ;
        mMsgHeader.length = ntohs(mMsgHeader.length) ;

        // Validate Header / Sanity
        if (    (nBufLength == (sizeof(STUN_MESSAGE_HEADER) + mMsgHeader.length)) && 
                validateMessageType(mMsgHeader.type))
        {
            bValid = true ;

            // Loop through each attribute
            int iBytesLeft = mMsgHeader.length ;
            while (bValid && (iBytesLeft >= sizeof(STUN_ATTRIBUTE_HEADER))) 
            {
                STUN_ATTRIBUTE_HEADER header ;
                memcpy(&header, pTraverse, sizeof(STUN_ATTRIBUTE_HEADER)) ;
                header.type = ntohs(header.type) ;
                header.length = ntohs(header.length) ;
                pTraverse += sizeof(STUN_ATTRIBUTE_HEADER) ;
                iBytesLeft -= sizeof(STUN_ATTRIBUTE_HEADER) ; 
                if (header.length <= iBytesLeft)
                {
                    bValid = parseAttribute(&header, pTraverse) ;
                    pTraverse += header.length ;
                    iBytesLeft -= header.length ;
                }
                else
                {
                    // Malformed packet: header describes more data then is available
                    bValid = false ;
                }
            }
            
            if (iBytesLeft != 0)
            {
                // Malformed packet: We had bytes left over
                bValid = false ;
            }
        }
    }

    return bValid ;
}


bool StunMessage::encode(char* pBuf, size_t nBufLength, size_t& nActualLength)
{
    bool bError = true ;
    char* pTraverse = pBuf ;
    size_t nBytesLeft = nBufLength ;

    // Skip length for Header
    if (nBufLength >= sizeof(STUN_MESSAGE_HEADER))
    {
        pTraverse += sizeof(STUN_MESSAGE_HEADER) ;
        nBytesLeft -= sizeof(STUN_MESSAGE_HEADER) ;
        bError = false ;
    }

    // Add Body
    if (!bError)
    {
        bError = encodeBody(pTraverse, nBytesLeft, nActualLength) ;
        pTraverse += nActualLength ;
        nActualLength += sizeof(STUN_MESSAGE_HEADER) ;        

        // Make room for message integrity
        if (mbIncludeMessageIntegrity && isRequestOrNonErrorResponse())
        {
            if (nBytesLeft >= sizeof(STUN_ATTRIBUTE_HEADER) + sizeof(mMessageIntegrity))
            {
                nBytesLeft -= sizeof(STUN_ATTRIBUTE_HEADER) + sizeof(mMessageIntegrity) ;
                nActualLength += sizeof(STUN_ATTRIBUTE_HEADER) + sizeof(mMessageIntegrity) ;
            }
            else
            {
                bError = true ;
            }
        }
    }
    else
    {
        nActualLength = 0 ;
    }

    // Added message header w/ correct length    
    if (!bError)
    {
        char* pStart = pBuf ;
        size_t nIgnore = nBufLength ;
        mMsgHeader.length = (unsigned short) nActualLength - sizeof(STUN_MESSAGE_HEADER) ;
        bError = !encodeHeader(&mMsgHeader, pStart, nIgnore) ;
    }

    // Lastly, calc/add message integrity (already made room)
    if (mbIncludeMessageIntegrity && isRequestOrNonErrorResponse())
    {
        calculateHmacSha1(pBuf, nActualLength - (sizeof(STUN_ATTRIBUTE_HEADER) + sizeof(mMessageIntegrity)), 
                NULL, STUN_MAX_MESSAGE_INTEGRITY_LENGTH, mMessageIntegrity) ;

        mbMessageIntegrityValid = true ;

        size_t nIgnore = nBytesLeft + sizeof(STUN_ATTRIBUTE_HEADER) + 
                sizeof(mMessageIntegrity) ;

        bError = !(encodeAttributeHeader(ATTR_STUN_MESSAGE_INTEGRITY, 
                sizeof(mMessageIntegrity), pTraverse, nBytesLeft) &&
                encodeRaw(mMessageIntegrity, STUN_MAX_MESSAGE_INTEGRITY_LENGTH,
                pTraverse, nBytesLeft)) ;                    
    }

    return !bError ;
}


bool StunMessage::encodeBody(char* pBuf, size_t nBufLength, size_t& nBytesUsed) 
{
    bool bError = false ;
    char* pTraverse = pBuf ;
    size_t nBytesLeft = nBufLength ;

    // Add Mapped Address
    if ((!bError) && mbMappedAddressValid)
    {
        if (!mbSendXorOnly)
        {
            bError = !encodeAttributeAddress(ATTR_STUN_MAPPED_ADDRESS, 
                    &mMappedAddress, pTraverse, nBytesLeft) ;
        }

        if (!bError)
        {
            bError = !encodeXorAttributeAddress(ATTR_STUN_XOR_MAPPED_ADDRESS, 
                    &mMappedAddress, pTraverse, nBytesLeft) ;        
        }
    }

    // Add Response Address
    if ((!bError) && mbResponseAddressValid)
    {
        bError = !encodeAttributeAddress(ATTR_STUN_RESPONSE_ADDRESS, 
                &mResponseAddress, pTraverse, nBytesLeft) ;
    }

    // Add Changed Address
    if ((!bError) && mbChangedAddressValid)
    {
        bError = !encodeAttributeAddress(ATTR_STUN_CHANGED_ADDRESS, 
                &mChangedAddress, pTraverse, nBytesLeft) ;
    }

    // Add Change Request
        if ((!bError) && mbChangeRequestValid)
        {
            bError = !(encodeAttributeHeader(ATTR_STUN_CHANGE_REQUEST, 
                sizeof(unsigned long), pTraverse, nBytesLeft) &&
                encodeLong(mChangeRequest, pTraverse, nBytesLeft)) ;
        }

    // Add Source Address
    if ((!bError) && mbSourceAddressValid)
    {
        bError = !encodeAttributeAddress(ATTR_STUN_SOURCE_ADDRESS, 
                &mSourceAddress, pTraverse, nBytesLeft) ;
    }

    // Add Username
    if ((!bError) && mbUsernameValid)
    {
        bError = !encodeString(ATTR_STUN_USERNAME, mUsername, pTraverse, 
                nBytesLeft) ;
    }
    
    // Add Password
    if ((!bError) && mbPasswordValid)
    {
        bError = !encodeString(ATTR_STUN_PASSWORD, mPassword, pTraverse, 
                nBytesLeft) ;
    }

    // Add Error
    if ((!bError) && mbErrorValid)
    {
        bError = !encodeAttributeError(&mError, pTraverse, nBytesLeft) ;
    }

    // Add Unknown Attributes
    if ((!bError) && mbUnknownAttributesValid)
    {
        bError = !encodeAttributesUnknown(&mUnknownAttributes, pTraverse, nBytesLeft) ;
    }

    // Add Reflected From
    if ((!bError) && mbReflectedFromValid)
    {
        bError = !encodeAttributeAddress(ATTR_STUN_REFLECTED_FROM, 
                &mReflectedFrom, pTraverse, nBytesLeft) ;
    }

    nBytesUsed = nBufLength - nBytesLeft ;

    return bError ;
}


void StunMessage::setTransactionId(STUN_TRANSACTION_ID& rTransactionId)
{
    memcpy(&mMsgHeader.transactionId, &rTransactionId, sizeof(STUN_TRANSACTION_ID)) ;
}


void StunMessage::allocTransactionId()
{
    for (int i=0; i<16; i++) 
    {
        mMsgHeader.transactionId.id[i] = (unsigned char) (mbRandomGenerator.rand() % 0x0100) ;
    }
}


void StunMessage::setType(unsigned short type)
{
    mMsgHeader.type = type ;
}


void StunMessage::setMappedAddress(const char* szIp, const unsigned short port) 
{
    mMappedAddress.family = ATTR_ADDRESS_FAMILY_IPV4 ;
    mMappedAddress.address = ntohl(inet_addr(szIp)) ;
    mMappedAddress.port = port ;
    mbMappedAddressValid = true ;
}


void StunMessage::setResponseAddress(const char* szIp, const unsigned short port)
{
    mResponseAddress.family = ATTR_ADDRESS_FAMILY_IPV4 ;
    mResponseAddress.address = ntohl(inet_addr(szIp)) ;
    mResponseAddress.port = port ;
    mbResponseAddressValid = true ;
}


void StunMessage::setChangePort(const bool bChange) 
{
    if (bChange)
    {
        mChangeRequest |= ATTR_CHANGE_FLAG_PORT ;
    }
    else
    {
        mChangeRequest &= ~ATTR_CHANGE_FLAG_PORT ;
    }

    mbChangeRequestValid = true ; 
}


void StunMessage::setChangeIp(const bool bChange) 
{
    if (bChange)
    {
        mChangeRequest |= ATTR_CHANGE_FLAG_IP ;
    }
    else
    {
        mChangeRequest &= ~ATTR_CHANGE_FLAG_IP ;
    }

    mbChangeRequestValid = true ; 
}

void StunMessage::setSourceAddress(const char* szIp, const unsigned short port) 
{
    mSourceAddress.family = ATTR_ADDRESS_FAMILY_IPV4 ;
    mSourceAddress.address = ntohl(inet_addr(szIp)) ;
    mSourceAddress.port = port ;
    mbSourceAddressValid = true ;
}

void StunMessage::setChangedAddress(const char* szIp, const unsigned short port) 
{
    mChangedAddress.family = ATTR_ADDRESS_FAMILY_IPV4 ;
    mChangedAddress.address = ntohl(inet_addr(szIp)) ;
    mChangedAddress.port = port ;
    mbChangedAddressValid = true ;
}

void StunMessage::setUsername(const char* szUsername) 
{
    if (szUsername && strlen(szUsername))
    {
        strncpy(mUsername, szUsername, STUN_MAX_STRING_LENGTH) ;
        mbUsernameValid = true ;
    }
}

void StunMessage::setPassword(const char* szPassword) 
{
    if (szPassword && strlen(szPassword))
    {
        strncpy(mPassword, szPassword, STUN_MAX_STRING_LENGTH) ;
        mbPasswordValid = true ;
    }
}

void StunMessage::setError(const unsigned short code, const char* szReason) 
{
    mError.errorClass = code / 100 ;
    mError.errorNumber = code % 100 ;
    strncpy(mError.szReasonPhrase, szReason, STUN_MAX_STRING_LENGTH) ;

    mbErrorValid = true ;
}

void StunMessage::addUnknownAttribute(unsigned short attributeId) 
{        
    if (mUnknownAttributes.nTypes < STUN_MAX_UNKNOWN_ATTRIBUTES)
    {
        mUnknownAttributes.type[mUnknownAttributes.nTypes++] = attributeId ;
    }
    mbUnknownAttributesValid = true ;
}

void StunMessage::setReflectedFrom(const char* szIp, const unsigned short port)
{
    mReflectedFrom.family = ATTR_ADDRESS_FAMILY_IPV4 ;
    mReflectedFrom.address = ntohl(inet_addr(szIp)) ;
    mReflectedFrom.port = port ;
    mbReflectedFromValid = true ;
}

void StunMessage::setServer(const char* szServer)
{
    if (szServer && strlen(szServer))
    {
        strncpy(mServer, szServer, STUN_MAX_STRING_LENGTH) ;
        mbServerValid = true ;
    }    
}

void StunMessage::setRequestXorOnly() 
{
    mbRequestXorOnly = true ;
}

void StunMessage::setSendXorOnly() 
{
    mbSendXorOnly = true ;
}

void StunMessage::setIncludeMessageIntegrity(bool bInclude) 
{
    mbIncludeMessageIntegrity = bInclude ;
}

/* ============================ ACCESSORS ================================= */

void StunMessage::getTransactionId(STUN_TRANSACTION_ID* pTransactionId)
{
    memcpy(pTransactionId, &mMsgHeader.transactionId, sizeof(STUN_TRANSACTION_ID)) ;
}

unsigned short StunMessage::getType() 
{
    return mMsgHeader.type ; 
}

bool StunMessage::getMappedAddress(char* szIp, unsigned short& rPort) 
{
    if (mbMappedAddressValid)
    {
        unsigned long address = htonl(mMappedAddress.address) ;
        strcpy(szIp, inet_ntoa(*((in_addr*) &address))) ;
        rPort = mMappedAddress.port ;
    }
    
    return mbMappedAddressValid ;
}


bool StunMessage::getResponseAddress(char* szIp, unsigned short& rPort)
{
    if (mbResponseAddressValid)
    {
        unsigned long address = htonl(mResponseAddress.address) ;
        strcpy(szIp, inet_ntoa(*((in_addr*) &address))) ;        
        rPort = mResponseAddress.port ;
    }
    
    return mbResponseAddressValid ;
}


bool StunMessage::getChangePort() 
{
    bool bChange = false ;

    if (mbChangeRequestValid)
    {
        if (mChangeRequest & ATTR_CHANGE_FLAG_PORT)
        {
            bChange = true ;
        }
    }

    return  bChange ; 
}


bool StunMessage::getChangeIp() 
{ 
    bool bChange = false ;

    if (mbChangeRequestValid)
    {
        if (mChangeRequest & ATTR_CHANGE_FLAG_IP)
        {
            bChange = true ;
        }
    }

    return  bChange ; 
}

bool StunMessage::getSourceAddress(char* szIp, unsigned short& rPort) 
{
    if (mbSourceAddressValid)
    {
        unsigned long address = htonl(mSourceAddress.address) ;
        strcpy(szIp, inet_ntoa(*((in_addr*) &address))) ;        
        rPort = mSourceAddress.port ;
    }
    
    return mbSourceAddressValid ;
}

bool StunMessage::getChangedAddress(char* szIp, unsigned short& rPort) 
{
    if (mbChangedAddressValid)
    {
        unsigned long address = htonl(mChangedAddress.address) ;
        strcpy(szIp, inet_ntoa(*((in_addr*) &address))) ;        
        rPort = mChangedAddress.port ;
    }
    
    return mbChangedAddressValid ;
}

bool StunMessage::getUsername(char* szUsername) 
{
    if (mbUsernameValid)
    {
        strncpy(szUsername, mUsername, STUN_MAX_STRING_LENGTH) ;
    }

    return mbUsernameValid ;
}

bool StunMessage::getPassword(char* szPassword) 
{
    if (mbPasswordValid)
    {
        strncpy(szPassword, mPassword, STUN_MAX_STRING_LENGTH) ;
    }

    return mbPasswordValid ;
}

bool StunMessage::getMessageIntegrity(char* cMessageIntegrity) 
{
    if (mbMessageIntegrityValid)
    {
        memcpy(cMessageIntegrity, mMessageIntegrity, STUN_MAX_MESSAGE_INTEGRITY_LENGTH) ;
    }
    return mbMessageIntegrityValid ;
}

bool StunMessage::getError(unsigned short& rCode, char* szReason) 
{
    if (mbErrorValid)
    {
        rCode = mError.errorClass * 100 + mError.errorNumber ;
        strncpy(szReason, mError.szReasonPhrase, STUN_MAX_STRING_LENGTH) ;
    }

    return mbErrorValid ;
}

bool StunMessage::getUnknownAttributes(unsigned short* pList, size_t nMaxItems, size_t& nActualItems)
{
    nActualItems = 0 ;        
    if (mbUnknownAttributesValid)
    {
        nActualItems = MIN(nMaxItems, mUnknownAttributes.nTypes) ;
        for (size_t i=0; i<nActualItems; i++)
        {
            pList[i] = mUnknownAttributes.type[i] ;
        }
    }

    return mbUnknownAttributesValid ;
}


bool StunMessage::getReflectedFrom(char* szIp, unsigned short& rPort)
{
    if (mbReflectedFromValid)
    {
        unsigned long address = htonl(mReflectedFrom.address) ;
        strcpy(szIp, inet_ntoa(*((in_addr*) &address))) ;        
        rPort = mReflectedFrom.port ;
    }

    return mbReflectedFromValid ;
}

bool StunMessage::getServer(char* szServer) 
{
    if (mbServerValid)
    {
        strncpy(szServer, mServer, STUN_MAX_STRING_LENGTH) ;
    }

    return mbServerValid ;
}


bool StunMessage::getRequestXorOnly()
{
    return mbRequestXorOnly ;   
}


bool StunMessage::getUnknownParsedAttributes(unsigned short* pList, size_t nMaxItems, size_t& nActualItems)
{
    nActualItems = 0 ;
    if (mUnknownParsedAttributes.nTypes)
    {
        nActualItems = MIN(nMaxItems, mUnknownParsedAttributes.nTypes) ;
        for (size_t i=0; i<nActualItems; i++)
        {
            pList[i] = mUnknownParsedAttributes.type[i] ;
        }
    }

    return (nActualItems > 0) ;
}


/* ============================ INQUIRY =================================== */

bool StunMessage::validateMessageType(unsigned short type) 
{
    bool bValid = false ;
    switch (type)
    {
        case MSG_STUN_BIND_REQUEST:
        case MSG_STUN_BIND_RESPONSE:
        case MSG_STUN_BIND_ERROR_RESPONSE:
        case MSG_STUN_SHARED_SECRET_REQUEST:
        case MSG_STUN_SHARED_SECRET_RESPONSE:
        case MSG_STUN_SHARED_SECRET_ERROR_RESPONSE:
            bValid = true ;
            break ;
        default:
            break ;
    }

    return bValid ;
}


bool StunMessage::isStunMessage(const char* pBuf, unsigned short nBufLength) 
{
    bool bValid = false ;

    if (nBufLength >= sizeof(STUN_MESSAGE_HEADER))
    {
        STUN_MESSAGE_HEADER header ;
        char* pTraverse = (char*) pBuf ;

        // Copy header
        memcpy(&header, pTraverse, sizeof(STUN_MESSAGE_HEADER)) ;
        header.type = ntohs(header.type) ;
        header.length = ntohs(header.length) ;
        
        if (nBufLength == (sizeof(STUN_MESSAGE_HEADER) + header.length))
        {
            switch (header.type)
            {
                case MSG_STUN_BIND_REQUEST:
                case MSG_STUN_BIND_RESPONSE:
                case MSG_STUN_BIND_ERROR_RESPONSE:
                case MSG_STUN_SHARED_SECRET_REQUEST:
                case MSG_STUN_SHARED_SECRET_RESPONSE:
                case MSG_STUN_SHARED_SECRET_ERROR_RESPONSE:
                    bValid = true ;
                    break ;
                default:
                    break ;
            }
        }
    }

    return bValid ;
}

bool StunMessage::isRequestOrNonErrorResponse() 
{
   bool bRequestOrNonErrorResponse ;

   switch (getType())
   {
        case MSG_STUN_BIND_REQUEST:
        case MSG_STUN_BIND_RESPONSE:
        case MSG_STUN_SHARED_SECRET_REQUEST:
        case MSG_STUN_SHARED_SECRET_RESPONSE:
            bRequestOrNonErrorResponse = true ;
            break ;
        case MSG_STUN_BIND_ERROR_RESPONSE:
        case MSG_STUN_SHARED_SECRET_ERROR_RESPONSE:
        default:
            bRequestOrNonErrorResponse = false ;
            break ;
    }

    return bRequestOrNonErrorResponse ;
}
 
/* //////////////////////////// PROTECTED ///////////////////////////////// */

bool StunMessage::encodeByte(char c, char*& pBuf, size_t& nBytesLeft)
{
    bool bRC = false ;

    if (nBytesLeft >= sizeof(char))
    {
        *pBuf = c ;

        nBytesLeft -= sizeof(char) ;
        pBuf += sizeof(char) ;
        
        bRC = true ;
    }

    return bRC ;
}

bool StunMessage::encodeShort(unsigned short value, char*& pBuf, size_t& nBytesLeft)
{
    bool bRC = false ;

    if (nBytesLeft >= sizeof(unsigned short))
    {
        value = htons(value) ;
        memcpy(pBuf, &value, sizeof(unsigned short)) ;

        nBytesLeft -= sizeof(unsigned short) ;
        pBuf += sizeof(unsigned short) ;
        
        bRC = true ;
    }

    return bRC ;
}


bool StunMessage::encodeLong(unsigned long value, char*& pBuf, size_t& nBytesLeft)
{
    bool bRC = false ;

    if (nBytesLeft >= sizeof(unsigned long))
    {
        value = htonl(value) ;
        memcpy(pBuf, &value, sizeof(unsigned long)) ;

        nBytesLeft -= sizeof(unsigned long) ;
        pBuf += sizeof(unsigned long) ;
        
        bRC = true ;
    }

    return bRC ;
}


bool StunMessage::encodeRaw(const char* cRaw, size_t length, char*& pBuf, size_t& nBytesLeft)
{
    bool bRC = false ;

    if (nBytesLeft >= length)
    {
        if (length > 0)
        {
            memcpy(pBuf, cRaw, length) ;

            nBytesLeft -= length ;
            pBuf += length ;
        }
        
        bRC = true ;
    }

    return bRC ;
}


bool StunMessage::encodeHeader(STUN_MESSAGE_HEADER* pHeader, char*& pBuf, size_t& nBytesLeft)
    {
    bool bRC = false ;

    if (    (nBytesLeft >= sizeof(STUN_MESSAGE_HEADER)) &&
            encodeShort(pHeader->type, pBuf, nBytesLeft) &&
            encodeShort(pHeader->length, pBuf, nBytesLeft) &&
            encodeRaw((char*) pHeader->transactionId.id, sizeof(STUN_TRANSACTION_ID), 
            pBuf, nBytesLeft))
    {
        bRC = true ;
    }

    return bRC ;
}

bool StunMessage::encodeAttributeHeader(short type, short length, char*& pBuf, size_t& nBytesLeft)
{
    bool bRC = false ;

    if (    (nBytesLeft >= sizeof(STUN_ATTRIBUTE_HEADER)) &&
            encodeShort(type, pBuf, nBytesLeft) &&
            encodeShort(length, pBuf, nBytesLeft))
    {
        bRC = true ;
    }

    return bRC ;
}

bool StunMessage::encodeAttributeAddress(unsigned short type, STUN_ATTRIBUTE_ADDRESS* pAddress, char*& pBuf, size_t& nBytesLeft)
{
    bool bRC = false ;

    if (    (nBytesLeft >= (sizeof(STUN_ATTRIBUTE_ADDRESS) + sizeof(STUN_ATTRIBUTE_HEADER))) &&
            encodeAttributeHeader(type, sizeof(STUN_ATTRIBUTE_ADDRESS), pBuf, nBytesLeft) &&
            encodeByte(pAddress->unused, pBuf, nBytesLeft) &&
            encodeByte(pAddress->family, pBuf, nBytesLeft) &&
            encodeShort(pAddress->port, pBuf, nBytesLeft) &&
            encodeLong(pAddress->address, pBuf, nBytesLeft))
    {
        bRC = true ;
    }

    return bRC ;
}

bool StunMessage::encodeXorAttributeAddress(unsigned short type, STUN_ATTRIBUTE_ADDRESS* pAddress, char*& pBuf, size_t& nBytesLeft)
{
    bool bRC = false ;
    char* ptr ;

    unsigned short usPort = pAddress->port ;
    unsigned long ulLong = pAddress->address ;

    ptr = (char*) &usPort ;
    ptr[0] ^= mMsgHeader.transactionId.id[1] ;
    ptr[1] ^= mMsgHeader.transactionId.id[0] ;

    ptr = (char*) &ulLong ;
    ptr[0] ^= mMsgHeader.transactionId.id[3] ;
    ptr[1] ^= mMsgHeader.transactionId.id[2] ;
    ptr[2] ^= mMsgHeader.transactionId.id[1] ;
    ptr[3] ^= mMsgHeader.transactionId.id[0] ;

    if (    (nBytesLeft >= (sizeof(STUN_ATTRIBUTE_ADDRESS) + sizeof(STUN_ATTRIBUTE_HEADER))) &&
            encodeAttributeHeader(type, sizeof(STUN_ATTRIBUTE_ADDRESS), pBuf, nBytesLeft) &&
            encodeByte(pAddress->unused, pBuf, nBytesLeft) &&
            encodeByte(pAddress->family, pBuf, nBytesLeft) &&
            encodeShort(usPort, pBuf, nBytesLeft) &&
            encodeLong(ulLong, pBuf, nBytesLeft))
    {
        bRC = true ;
    }

    return bRC ;
}


bool StunMessage::encodeString(unsigned short type, const char* szString, char*& pBuf, size_t& nBytesLeft)
{
    bool bRC = false ;
    size_t nActualLength ;
    size_t nPaddedLength ;
    char cPadding[STUN_MIN_CHAR_PAD] ;

    nActualLength = strlen(szString) ;
    nPaddedLength = ((nActualLength + (STUN_MIN_CHAR_PAD-1)) / STUN_MIN_CHAR_PAD) * \
            STUN_MIN_CHAR_PAD ;
    memset(cPadding, 0, sizeof(cPadding)) ;

    if (    (nBytesLeft >= (nPaddedLength + sizeof(STUN_ATTRIBUTE_HEADER))) &&
            encodeAttributeHeader(type, (short) nPaddedLength, pBuf, nBytesLeft) &&
            encodeRaw(szString, nActualLength, pBuf, nBytesLeft) &&
            encodeRaw(cPadding, nPaddedLength - nActualLength, pBuf, nBytesLeft))
    {
        bRC = true ;
    }

    return bRC ;
}


bool StunMessage::encodeAttributeError(STUN_ATTRIBUTE_ERROR* pError, char*& pBuf, size_t& nBytesLeft)
{
    bool bRC = false;

    size_t nReason = strlen(pError->szReasonPhrase) ;
    size_t nPaddedReason = ((nReason + (STUN_MIN_CHAR_PAD-1)) / STUN_MIN_CHAR_PAD) * 
            STUN_MIN_CHAR_PAD ;
    char cPad[STUN_MIN_CHAR_PAD] ;

    size_t nTotalLength = sizeof(STUN_ATTRIBUTE_HEADER) + nPaddedReason + 4 ;
    memset(cPad, 0, sizeof(cPad)) ;

    if (    (nBytesLeft >= nTotalLength) &&
            encodeAttributeHeader(ATTR_STUN_ERROR_CODE, 
                (short) nTotalLength - sizeof(STUN_ATTRIBUTE_HEADER), pBuf, 
                nBytesLeft) &&
            encodeShort(pError->unused, pBuf, nBytesLeft) &&
            encodeByte(pError->errorClass & 0x0F, pBuf, nBytesLeft) &&
            encodeByte(pError->errorNumber, pBuf, nBytesLeft) &&
            encodeRaw(pError->szReasonPhrase, nReason, pBuf, nBytesLeft) &&
            encodeRaw(cPad, nPaddedReason - nReason, pBuf, nBytesLeft)) 
    {
        bRC = true ;
    }

    return bRC ;
}

bool StunMessage::encodeAttributesUnknown(STUN_ATTRIBUTE_UNKNOWN* pAttributes, char*& pBuf, size_t& nBytesLeft)
{
    bool bRC = false;

    size_t nAttributes = pAttributes->nTypes ;
    size_t nPaddedAttributes = ((nAttributes + 1) / 2) * 2 ;

    if (nBytesLeft >= (sizeof(STUN_ATTRIBUTE_HEADER) + nPaddedAttributes * sizeof(unsigned short)))
    {
        // Repeat last attribute if an odd number
        if (nPaddedAttributes != nAttributes)
        {
            pAttributes->type[nPaddedAttributes] = pAttributes->type[nAttributes] ;
        }

        bRC = encodeAttributeHeader(ATTR_STUN_UNKNOWN_ATTRIBUTE, 
                (short) (nPaddedAttributes * sizeof(unsigned short)),
                pBuf, nBytesLeft) ;

        for (size_t i = 0; (i<nPaddedAttributes) && bRC; i++)
        {
            bRC = encodeShort(pAttributes->type[i], pBuf, nBytesLeft) ;
        }
    }

    return bRC ;
}


bool StunMessage::parseAttribute(STUN_ATTRIBUTE_HEADER* pHeader, char* pBuf)
{
    bool bValid = false ;

    switch (pHeader->type)
    {
        case ATTR_STUN_MAPPED_ADDRESS:
            if (!mbMappedAddressValid)
            {
                bValid = parseAddressAttribute(pBuf, pHeader->length, &mMappedAddress) ;
                mbMappedAddressValid = bValid ;                
            }
            else
            {
                bValid = true ;
            }
            break ;
        case ATTR_STUN_RESPONSE_ADDRESS:
            bValid = parseAddressAttribute(pBuf, pHeader->length, &mResponseAddress) ;
            mbResponseAddressValid = bValid ;
            break ;
        case ATTR_STUN_CHANGE_REQUEST:
            bValid = parseLongAttribute(pBuf, pHeader->length, &mChangeRequest) ;
            mbChangeRequestValid = bValid ;
            break ;
        case ATTR_STUN_SOURCE_ADDRESS:
            bValid = parseAddressAttribute(pBuf, pHeader->length, &mSourceAddress) ;
            mbSourceAddressValid = bValid ;
            break ;
        case ATTR_STUN_CHANGED_ADDRESS:
            bValid = parseAddressAttribute(pBuf, pHeader->length, &mChangedAddress) ;
            mbChangedAddressValid = bValid ;
            break ;
        case ATTR_STUN_USERNAME:
            bValid = parseStringAttribute(pBuf, pHeader->length, mUsername) ;                
            mbUsernameValid = bValid ;
            break ;
        case ATTR_STUN_PASSWORD:
            bValid = parseStringAttribute(pBuf, pHeader->length, mPassword) ;
            mbPasswordValid = bValid ;
            break ;
        case ATTR_STUN_MESSAGE_INTEGRITY:
            bValid = parseRawAttribute(pBuf, pHeader->length, mMessageIntegrity, sizeof(mMessageIntegrity)) ;
            mbMessageIntegrityValid = bValid ;
            break ;
        case ATTR_STUN_ERROR_CODE:
            bValid = parseErrorAttribute(pBuf, pHeader->length, &mError) ;
            mbErrorValid = bValid ;
            break ;
        case ATTR_STUN_UNKNOWN_ATTRIBUTE:
            bValid = parseUnknownAttribute(pBuf, pHeader->length, &mUnknownAttributes) ;
            mbUnknownAttributesValid = bValid ;
            break ;
        case ATTR_STUN_REFLECTED_FROM:
            bValid = parseAddressAttribute(pBuf, pHeader->length, &mReflectedFrom) ;
            mbReflectedFromValid = bValid ;
            break ;
        case 0x8020:    // backwards compatibility
        case ATTR_STUN_XOR_MAPPED_ADDRESS:
            bValid = parseXorAddressAttribute(pBuf, pHeader->length, &mMappedAddress) ;
            mbMappedAddressValid = bValid ;            
            break ;
        case ATTR_STUN_XOR_ONLY:
            mbRequestXorOnly = true ;
            bValid = true ;
            break ;
        case 0x8022:    // backwards compatibility
        case ATTR_STUN_SERVER:
            bValid = parseStringAttribute(pBuf, pHeader->length, mServer) ;
            mbServerValid = bValid ;
            break ;
        default:
            if ((pHeader->type <= 0x7FFF) && (mUnknownParsedAttributes.nTypes < STUN_MAX_UNKNOWN_ATTRIBUTES))
            {
                mUnknownParsedAttributes.type[mUnknownParsedAttributes.nTypes++] = pHeader->type ;
            }
            break ;
    }

    return bValid ;
}


bool StunMessage::parseAddressAttribute(char *pBuf, size_t nLength, STUN_ATTRIBUTE_ADDRESS* pAddress)
{
    bool bValid = false ;

    if (nLength == sizeof(STUN_ATTRIBUTE_ADDRESS))
    {
        memcpy(pAddress, pBuf, sizeof(STUN_ATTRIBUTE_ADDRESS)) ;
        pAddress->port = ntohs(pAddress->port) ;
        pAddress->address = ntohl(pAddress->address) ;

        bValid = true ;
    }
    return bValid ;
}

bool StunMessage::parseXorAddressAttribute(char *pBuf, size_t nLength, STUN_ATTRIBUTE_ADDRESS* pAddress)
{
    bool bValid = false ;
    char* ptr ;

    if (nLength == sizeof(STUN_ATTRIBUTE_ADDRESS))
    {
        memcpy(pAddress, pBuf, sizeof(STUN_ATTRIBUTE_ADDRESS)) ;

        pAddress->port = ntohs(pAddress->port) ;
        pAddress->address = ntohl(pAddress->address) ;

        ptr = (char*) &pAddress->port ;
        ptr[0] ^= mMsgHeader.transactionId.id[1] ;
        ptr[1] ^= mMsgHeader.transactionId.id[0] ;

        ptr = (char*) &pAddress->address ;
        ptr[0] ^= mMsgHeader.transactionId.id[3] ;
        ptr[1] ^= mMsgHeader.transactionId.id[2] ;
        ptr[2] ^= mMsgHeader.transactionId.id[1] ;
        ptr[3] ^= mMsgHeader.transactionId.id[0] ;

        bValid = true ;
    }
    return bValid ;
}


bool StunMessage::parseShortAttribute(char *pBuf, size_t nLength, unsigned short* pShort)
{
    bool bValid = false ;

    if (nLength == sizeof(unsigned short)) 
    {
        memcpy(pShort, pBuf, sizeof(unsigned short)) ;
        *pShort = ntohs(*pShort) ;

        bValid = true ;
    }
    return bValid ;
}

bool StunMessage::parseLongAttribute(char *pBuf, size_t nLength, unsigned long* pLong)
{
    bool bValid = false ;

    if (nLength == sizeof(unsigned long)) 
    {
        memcpy(pLong, pBuf, sizeof(unsigned long)) ;
        *pLong = ntohl(*pLong) ;

        bValid = true ;
    }
    return bValid ;
}


bool StunMessage::parseStringAttribute(char* pBuf, size_t nLength, char* pString)
{
    bool bValid = false ;

    if ((nLength > 0) && (nLength % STUN_MIN_CHAR_PAD) == 0)
    {
        memset(pString, 0, STUN_MAX_STRING_LENGTH+1) ;
        memcpy(pString, pBuf, MIN(nLength, STUN_MAX_STRING_LENGTH)) ;
        bValid = true ;
    }

    return bValid ;
}

bool StunMessage::parseRawAttribute(char* pBuf, size_t nLength, char* pDest, size_t nDest)
{
    bool bValid = false ;

    if (nLength == nDest)
    {
        memcpy(pDest, pBuf, nDest) ;
        bValid = true ;
    }

    return bValid ;
}


bool StunMessage::parseErrorAttribute(char *pBuf, size_t nLength, STUN_ATTRIBUTE_ERROR* pError)
{
    bool bValid = false ;

    if (nLength > sizeof(unsigned long) && ((nLength % STUN_MIN_CHAR_PAD) == 0))
    {                 
        pError->unused = 0 ;
        pError->unused2 = 0 ;
        pBuf += 2 ;
        pError->errorClass = (*pBuf++ & 0xF0) ;
        pError->errorNumber = *pBuf++ ;
        nLength -= 4 ;
        memset(pError->szReasonPhrase, 0, STUN_MAX_STRING_LENGTH) ;
        memcpy(pError->szReasonPhrase, pBuf, MIN(nLength, STUN_MAX_STRING_LENGTH)) ;
        bValid = true ;
    }
    return bValid ;
}


bool StunMessage::parseUnknownAttribute(char* pBuf, size_t nLength, STUN_ATTRIBUTE_UNKNOWN* pAttributes)
{
    bool bValid = false ;

    if ((nLength > 0) && ((nLength % STUN_MIN_CHAR_PAD) == 0))
    {
        size_t i ;

        unsigned short* pShort = (unsigned short*) pBuf ;
        for (i=0; (i<nLength/2) && (i<STUN_MAX_UNKNOWN_ATTRIBUTES); i++)
        {                  
            pAttributes->type[i] = *pShort++ ;
        }
        pAttributes->nTypes = i ;

        bValid = true ;
    }

    return bValid ;
}

// zero pads to 64 boundry and results will be 20 bytes long for hmac/sha1
void StunMessage::calculateHmacSha1(char *pDataIn, size_t nDataIn, char *pKey, size_t nKey, char* results) 
{
    size_t nPaddedLength = ((nDataIn + 63) / 64) * 64 ;
    char* pTempBuf = (char*) malloc(nPaddedLength) ;
    memset(pTempBuf, 0, nPaddedLength) ;
    memcpy(pTempBuf, pDataIn, nDataIn) ;

    memset(results, 0, 20) ;
    hmac_sha1(pTempBuf, nPaddedLength, NULL, 0, results) ;
    free(pTempBuf) ;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
