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
#include <assert.h>

// APPLICATION INCLUDES
#include "os/StunUtils.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

STUN_NAT_TYPE StunUtils::determineStunNatType(const char* szServer, const int port)
{
    OsDatagramSocket* pSocket = new OsDatagramSocket(0, NULL, 0, NULL) ;
    StunMessage msg ;
    UtlString stunIp ;
    UtlString localIp; 
    STUN_NAT_TYPE rc = STUN_NAT_ERROR_BAD_SERVER ;
    char cMappedIp[64] ;
    unsigned short usMappedPort ;
    char cChangedIp[64] ;
    unsigned short usChangedPort ;

    OsSocket::getHostIp(&localIp) ;
    if (OsSocket::getHostIpByName(szServer, &stunIp))
    {
        if (sendStunNatTest(pSocket, stunIp, port, false, false, cMappedIp, 
                &usMappedPort, cChangedIp, &usChangedPort))
        {
            if ((localIp.compareTo(cMappedIp) == 0) && 
                    usMappedPort == pSocket->getLocalHostPort())
            {
                if (sendStunNatTest(pSocket, stunIp, port, true, true, NULL, NULL,
                        NULL, NULL))
                {
                    rc = STUN_NAT_OPEN ;
                }
                else
                {
                    rc = STUN_NAT_SYMMETRIC_FIREWALL ;
                }
            }
            else
            {
                if (sendStunNatTest(pSocket, stunIp, port, true, true, cMappedIp, &usMappedPort,
                        NULL, NULL))
                {
                    rc = STUN_NAT_FULL_CONE ;
                }
                else
                {
                    char cMappedIp2[64] ;
                    unsigned short usMappedPort2 ;

                    if (sendStunNatTest(pSocket, cChangedIp, usChangedPort, false, false, cMappedIp2, &usMappedPort2,
                            NULL, NULL))
                    {
                        // Check mapped IP
                        if ((strcmp(cMappedIp, cMappedIp2) == 0) && (usMappedPort2 == usMappedPort))
                        {
                            rc = STUN_NAT_SYMMETRIC ;
                        }
                        else
                        {
                            // Send Test III
                            if (sendStunNatTest(pSocket, cChangedIp, usChangedPort, true, false, 
                                    NULL, NULL, NULL, NULL))
                            {
                                rc = STUN_NAT_RESTRICTED_CONE ;
                            }
                            else
                            {
                                rc = STUN_NAT_PORT_RESTRICTED_CONE ;
                            }
                        }
                    }
                    else
                    {
                        // UNKNOWN -- what does this mean?
                        assert(false) ;
                    }                    
                }
            }
        }
        else
        {
            rc = STUN_NAT_BLOCKED ;
        }
    }

    delete pSocket ;

    return rc ;
}


bool StunUtils::allocateTurnAddress(OsDatagramSocket* pSocket,
                                    const char* szServerIp,
                                    const int port,
                                    char* szRelayIp,
                                    unsigned short* piRelayPort,
                                    unsigned long* plLifetime)
{
    bool bSuccess = false ;

    TurnMessage msgSend ;    
    size_t nLength ;
    char cMsgSend[2048] ;
    TurnMessage msgReceive ;
    char cMsgReceive[2048] ;


    msgSend.allocTransactionId() ;
    msgSend.setType(MSG_TURN_ALLOCATE_REQUEST) ;
    msgSend.setRequestXorOnly() ;

    if (msgSend.encode(cMsgSend, sizeof(cMsgSend), nLength))
    {
        for (int i=0; (i<3) && (bSuccess == false); i++)
        {
            if (pSocket->write(cMsgSend, (int) nLength, szServerIp, port) > 0)
            {
                if (pSocket->isReadyToRead(500))
                {
                    int length = pSocket->read(cMsgReceive, sizeof(cMsgReceive)) ;
                    if ((length > 0) && msgReceive.parse(cMsgReceive, length))
                    {
                        if (msgReceive.getType() == MSG_TURN_ALLOCATE_RESPONSE)
                        {        
                            bSuccess = true ;

                            // Get optional mapped address
                            if (szRelayIp && piRelayPort)
                            {
                                msgReceive.getMappedAddress(szRelayIp, *piRelayPort) ;
                            }

                            if (plLifetime)
                            {
                                msgReceive.getLifetime(*plLifetime) ;
                            } 
                            
                        }
                        else
                        {
                            // Abort on error response
                            break ;
                        }
                    }
                }
            }
            else
            {
                break ;
            }
        }
    }
    else
    {
        assert(false) ;
    }    


    return bSuccess ;
}


bool StunUtils::sendStunNatTest(OsDatagramSocket* pSocket,
                                const char* szServerIp,
                                const int port,
                                bool bChangePort,
                                bool bChangeIP,
                                char* szMappedIp,
                                unsigned short* piMappedPort,
                                char* szChangedIp,
                                unsigned short* piChangedPort) 
{
    StunMessage msgSend ;
    char cMsgSend[2048] ;
    StunMessage msgReceive ;
    char cMsgReceive[2048] ;
    size_t nLength ;
    bool bSuccess = false ;
    
    msgSend.setType(MSG_STUN_BIND_REQUEST) ;
    msgSend.allocTransactionId() ;
    msgSend.setRequestXorOnly() ;

    if (bChangePort)
    {
        msgSend.setChangePort(true) ;
    }
    if (bChangeIP)
    {
        msgSend.setChangeIp(true) ;
    }

    if (msgSend.encode(cMsgSend, sizeof(cMsgSend), nLength))
    {
        for (int i=0; (i<3) && (bSuccess == false); i++)
        {
            if (pSocket->write(cMsgSend, (int) nLength, szServerIp, port) > 0)
            {
                if (pSocket->isReadyToRead(500))
                {
                    int length = pSocket->read(cMsgReceive, sizeof(cMsgReceive)) ;
                    if ((length > 0) && msgReceive.parse(cMsgReceive, length))
                    {
                        if (msgReceive.getType() == MSG_STUN_BIND_RESPONSE)
                        {
                            bSuccess = true ;

                            // Get optional mapped address
                            if (szMappedIp && piMappedPort)
                            {
                                msgReceive.getMappedAddress(szMappedIp, *piMappedPort) ;
                            }

                            // Get optional changed address
                            if (szChangedIp && piChangedPort)
                            {
                                msgReceive.getChangedAddress(szChangedIp, *piChangedPort) ;                        
                            }
                        }
                        else
                        {
                            // Abort on error response
                            break ;
                        }
                    }
                }
            }
            else
            {
                break ;
            }
        }
    }
    else
    {
        assert(false) ;
    }

    return bSuccess ;
}

const char* StunUtils::natTypeToString(STUN_NAT_TYPE type) 
{
    const char* szRC ;

    switch (type)
    {
        case STUN_NAT_ERROR_BAD_SERVER:
            szRC = "STUN_NAT_ERROR_BAD_SERVER" ;
            break ;
        case STUN_NAT_BLOCKED:
            szRC = "STUN_NAT_BLOCKED" ;
            break ;
        case STUN_NAT_SYMMETRIC_FIREWALL:
            szRC = "STUN_NAT_SYMMETRIC_FIREWALL" ;
            break ;
        case STUN_NAT_OPEN:
            szRC = "STUN_NAT_OPEN" ;
            break ;
        case STUN_NAT_FULL_CONE:
            szRC = "STUN_NAT_FULL_CONE" ;
            break ;
        case STUN_NAT_RESTRICTED_CONE:
            szRC = "STUN_NAT_RESTRICTED_CONE" ;
            break ;
        case STUN_NAT_PORT_RESTRICTED_CONE:
            szRC = "STUN_NAT_PORT_RESTRICTED_CONE" ;
            break ;
        case STUN_NAT_SYMMETRIC:
            szRC = "STUN_NAT_SYMMETRIC" ;
            break ;
        default:
            szRC = "UNKNOWN (ERROR)" ;
            break ;
    }

    return szRC ;
}



static const char* convertRequestToString(unsigned short request)
{
    const char* szRequest = "Unknown" ;

    switch (request)
    {
        case MSG_STUN_BIND_REQUEST:
            szRequest = "MSG_STUN_BIND_REQUEST" ;
            break ;
        case MSG_STUN_BIND_RESPONSE:
            szRequest = "MSG_STUN_BIND_RESPONSE" ;
            break ;
        case MSG_STUN_BIND_ERROR_RESPONSE:
            szRequest = "MSG_STUN_BIND_ERROR_RESPONSE" ;
            break ;
        case MSG_STUN_SHARED_SECRET_REQUEST:
            szRequest = "MSG_STUN_SHARED_SECRET_REQUEST" ;
            break ;
        case MSG_STUN_SHARED_SECRET_RESPONSE:
            szRequest = "MSG_STUN_SHARED_SECRET_RESPONSE" ;
            break ;
        case MSG_STUN_SHARED_SECRET_ERROR_RESPONSE:
            szRequest = "MSG_STUN_SHARED_SECRET_ERROR_RESPONSE" ;
            break ;
        case MSG_TURN_ALLOCATE_REQUEST:
            szRequest = "MSG_TURN_ALLOCATE_REQUEST" ;
            break ;
        case MSG_TURN_ALLOCATE_RESPONSE:
            szRequest = "MSG_TURN_ALLOCATE_RESPONSE" ;
            break ;
        case MSG_TURN_ALLOCATE_ERROR_RESPONSE:
            szRequest = "MSG_TURN_ALLOCATE_ERROR_RESPONSE" ;
            break ;
        case MSG_TURN_SEND_REQUEST:
            szRequest = "MSG_TURN_SEND_REQUEST" ;
            break ;
        case MSG_TURN_SEND_RESPONSE:
            szRequest = "MSG_TURN_SEND_RESPONSE" ;
            break ;
        case MSG_TURN_SEND_ERROR_RESPONSE:
            szRequest = "MSG_TURN_SEND_ERROR_RESPONSE" ;
            break ;
        case MSG_TURN_DATA_INDICATION:
            szRequest = "MSG_TURN_DATA_INDICATION" ;
            break ;
        case MSG_TURN_ACTIVE_DESTINATION_REQUEST:
            szRequest = "MSG_TURN_ACTIVE_DESTINATION_REQUEST" ;
            break ;
        case MSG_TURN_ACTIVE_DESTINATION_RESPONSE:
            szRequest = "MSG_TURN_ACTIVE_DESTINATION_RESPONSE" ;
            break ;
        case MSG_TURN_ACTIVE_DESTINATION_ERROR_RESPONSE:
            szRequest = "MSG_TURN_ACTIVE_DESTINATION_ERROR_RESPONSE" ;
            break ;
    }
    return szRequest ;
}

static const char* convertAttributeToString(unsigned short attribute)
{
    const char* szAttribute = "Unknown" ;

    switch (attribute)
    {
        case ATTR_STUN_MAPPED_ADDRESS:
            szAttribute = "ATTR_STUN_MAPPED_ADDRESS" ;
            break ;
        case ATTR_STUN_RESPONSE_ADDRESS:
            szAttribute = "ATTR_STUN_RESPONSE_ADDRESS" ;
            break ;
        case ATTR_STUN_CHANGE_REQUEST:
            szAttribute = "ATTR_STUN_CHANGE_REQUEST" ;
            break ;
        case ATTR_STUN_SOURCE_ADDRESS:
            szAttribute = "ATTR_STUN_SOURCE_ADDRESS" ;
            break ;
        case ATTR_STUN_CHANGED_ADDRESS:    
            szAttribute = "ATTR_STUN_CHANGED_ADDRESS" ;
            break ;
        case ATTR_STUN_USERNAME:           
            szAttribute = "ATTR_STUN_USERNAME" ;
            break ;
        case ATTR_STUN_PASSWORD:           
            szAttribute = "ATTR_STUN_PASSWORD" ;
            break ;
        case ATTR_STUN_MESSAGE_INTEGRITY:  
            szAttribute = "ATTR_STUN_MESSAGE_INTEGRITY" ;
            break ;
        case ATTR_STUN_ERROR_CODE:
            szAttribute = "ATTR_STUN_ERROR_CODE" ;
            break ;
        case ATTR_STUN_UNKNOWN_ATTRIBUTE:
            szAttribute = "ATTR_STUN_UNKNOWN_ATTRIBUTE" ;
            break ;
        case ATTR_STUN_REFLECTED_FROM:
            szAttribute = "ATTR_STUN_REFLECTED_FROM" ;
            break ;
        case ATTR_STUN_XOR_MAPPED_ADDRESS:
            szAttribute = "ATTR_STUN_XOR_MAPPED_ADDRESS" ;
            break ;
        case ATTR_STUN_XOR_ONLY:
            szAttribute = "ATTR_STUN_XOR_ONLY" ;
            break ;
        case ATTR_STUN_SERVER:
            szAttribute = "ATTR_STUN_SERVER" ;
            break ;
        case ATTR_TURN_LIFETIME:
            szAttribute = "ATTR_TURN_LIFETIME" ;
            break ;
        case ATTR_TURN_ALTERNATE_SERVER:
            szAttribute = "ATTR_TURN_ALTERNATE_SERVER" ;
            break ;
        case ATTR_TURN_MAGIC_COOKIE:
            szAttribute = "ATTR_TURN_MAGIC_COOKIE" ;
            break ;
        case ATTR_TURN_BANDWIDTH:
            szAttribute = "ATTR_TURN_BANDWIDTH" ;
            break ;
        case ATTR_TURN_DESTINATION_ADDRESS:
            szAttribute = "ATTR_TURN_DESTINATION_ADDRESS" ;
            break ;
        case ATTR_TURN_SOURCE_ADDRESS:
            szAttribute = "ATTR_TURN_SOURCE_ADDRESS" ;
            break ;
        case ATTR_TURN_DATA:
            szAttribute = "ATTR_TURN_DATA" ;
            break ;
        case ATTR_TURN_NONCE:
            szAttribute = "ATTR_TURN_NONCE" ;
            break ;
        case ATTR_TURN_REALM:
            szAttribute = "ATTR_TURN_REALM" ;
            break ;
    }

    return szAttribute ;
}

// WARNING: nOutput is currently ignored -- should be a UtlString or actually checked
void StunUtils::debugDump(char* pPacket, size_t nPacket, UtlString& output) 
{
    char cTemp[1024] ;
    output.remove(0) ;

    char* pTraverse = pPacket ;

    if (    StunMessage::isStunMessage(pPacket, nPacket) || 
            TurnMessage::isTurnMessage(pPacket, nPacket))
    {
        STUN_MESSAGE_HEADER msgHeader ;
        memcpy(&msgHeader, pPacket, sizeof(msgHeader)) ;
        pTraverse += sizeof(msgHeader) ;

        sprintf(cTemp, "Msg t=0x%04X/%s, l=%d, id=", 
            ntohs(msgHeader.type), 
            convertRequestToString(ntohs(msgHeader.type)),
            ntohs(msgHeader.length)) ;

        output.append(cTemp) ;

        for (int i=0; i<16; i++)
        {           
            sprintf(cTemp, "%02X", msgHeader.transactionId.id[i]) ;
            cTemp[3] = 0 ;
            output.append(cTemp) ;
        }
        output.append("\n") ;

        while (pTraverse < (pPacket + nPacket) )
        {
            if (((pPacket + nPacket) - pTraverse) >= sizeof(STUN_ATTRIBUTE_HEADER))
            {
                STUN_ATTRIBUTE_HEADER attrHeader ;
                memcpy(&attrHeader, pTraverse, sizeof(attrHeader)) ;
                pTraverse += sizeof(attrHeader) ;            
                
                sprintf(cTemp, "attr t=0x%04X/%s, l=%d\n  v=", 
                    ntohs(attrHeader.type), 
                    convertAttributeToString(ntohs(attrHeader.type)),
                    ntohs(attrHeader.length)) ;
                output.append(cTemp) ;           

                if (((pPacket + nPacket) - pTraverse) >= ntohs(attrHeader.length))
                {
                    for (int i=0; i<ntohs(attrHeader.length); i++)
                    {
                        sprintf(cTemp, "%02X", *pTraverse++) ;
                        cTemp[3] = 0 ;
                        output.append(cTemp) ;
                    }
                    output.append("\n") ;
                }
                else
                {
                    output.append("**CORRUPT PACKET LENGTH**\n") ;
                    break ;
                }
            }
            else
            {
                output.append("**CORRUPT PACKET ATTR HEADER**\n") ;
                break ;
            }
        }
    }
    else
    {
        output.append("**CORRUPT PACKET**\n") ;
    }
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

