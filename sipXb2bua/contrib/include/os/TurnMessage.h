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

#ifndef _TurnMessage_h_
#define _TurnMessage_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/StunMessage.h"

// DEFINES

/**
 * TURN Message IDs
 */
#define MSG_TURN_ALLOCATE_REQUEST                   0x0003
#define MSG_TURN_ALLOCATE_RESPONSE                  0x0103
#define MSG_TURN_ALLOCATE_ERROR_RESPONSE            0x0113
#define MSG_TURN_SEND_REQUEST                       0x0004
#define MSG_TURN_SEND_RESPONSE                      0x0104      // deprecated
#define MSG_TURN_SEND_ERROR_RESPONSE                0x0114      // deprecated
#define MSG_TURN_DATA_INDICATION                    0x0115
#define MSG_TURN_ACTIVE_DESTINATION_REQUEST         0x0006
#define MSG_TURN_ACTIVE_DESTINATION_RESPONSE        0x0106
#define MSG_TURN_ACTIVE_DESTINATION_ERROR_RESPONSE  0x0116
#define MSG_TURN_CONNECTION_STATUS_INDICATION       0x0117
#define MSG_TURN_CLOSE_BINDING_REQUEST              0x0009
#define MSG_TURN_CLOSE_BINDING_RESPONSE             0x0109
#define MSG_TURN_CLOSE_BINDING_ERROR_RESPONSE       0x0119


/**
 * TURN attribute IDs
 */
#define ATTR_TURN_LIFETIME                      0x000D
#define ATTR_TURN_MAGIC_COOKIE                  0x000F  // deprecated
#define ATTR_TURN_BANDWIDTH                     0x0010
#define ATTR_TURN_DESTINATION_ADDRESS           0x0011  // deprecated
#define ATTR_TURN_REMOTE_ADDRESS                0x0012  // rename from ATTR_TURN_SOURCE_ADDRESS
#define ATTR_TURN_DATA                          0x0013
#define ATTR_TURN_RELAY_ADDRESS                 0x0016
// #define ATTR_TURN_REQUESTED_PORT                0x0018
#define ATTR_TURN_REQUESTED_TRANSPORT           0x0019
// #define ATTR_TURN_TIMER_VAL                     0x0021
#define ATTR_TURN_REQUESTED_IP                  0x0022

#define ATTR_MAGIC_COOKIE                       0x72c64bc6

// Error Defines
#define TURN_ERROR_NO_BINDING_CODE              437
#define TURN_ERROR_NO_BINDING_TEXT              "no binding"

#define TURN_ERROR_TRANSITIONING_CODE           439
#define TURN_ERROR_TRANSITIONING_TEXT           "transitioning"

#define TURN_ERROR_WRONG_USERNAME_CODE          441
#define TURN_ERROR_WRONG_USERNAME_TEXT          "wrong username"

#define TURN_ERROR_TRANSPORT_PROTOCOL_CODE      442
#define TURN_ERROR_TRANSPORT_PROTOCOL_TEXT      "bad transport"

#define TURN_ERROR_INVALID_IP_CODE              443
#define TURN_ERROR_INVALID_IP_TEXT              "invalid ip"

#define TURN_ERROR_INVALID_PORT_CODE            444
#define TURN_ERROR_INVALID_PORT_TEXT            "invalid port"

#define TURN_ERROR_TCP_ONLY_CODE                445
#define TURN_ERROR_TCP_ONLY_TEXT                "tcp only"

#define TURN_ERROR_CONNECTION_FAILED_CODE       446
#define TURN_ERROR_CONNECTION_FAILED_TEXT       "connection failed"

#define TURN_ERROR_CONNECTION_TIMEOUT_CODE      447
#define TURN_ERROR_CONNECTION_TIMEOUT_TEXT      "connection timeout"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef enum TURN_TRANSPORT_TYPE
{
    TURN_TRANSPORT_UDP,
    TURN_TRANSPORT_TCP,
} TURN_TRANSPORT_TYPE ;

// FORWARD DECLARATIONS

/**
 */
class TurnMessage : public StunMessage
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   TurnMessage();
     
   /**
    * Destructor
    */
   virtual ~TurnMessage();

/* ============================ MANIPULATORS ============================== */

    virtual void reset() ;
    
    virtual bool encodeBody(char* pBuf, size_t nBufLength, size_t& nBytesUsed) ;

    void setLifetime(unsigned long secs) ;

    void setBandwidth(unsigned long rKBPS) ;

    /** deprecated */
    void setDestinationAddress(const char* szIp, unsigned short port) ;

    void setTurnRemoteAddress(const char* szIp, unsigned short port) ;

    void setData(const char* pData, unsigned short nLength) ;

    void setRelayAddress(const char* szIp, unsigned short port) ;

    void setRequestedTransport(TURN_TRANSPORT_TYPE transportType) ;

    void setRequestedIp(const char* szIp, unsigned short port) ;

/* ============================ ACCESSORS ================================= */

    bool getLifetime(unsigned long& rSecs) ;

    bool getBandwidth(unsigned long& rKBPS) ;

    /** deprecated */
    bool getDestinationAddress(char* szIp, unsigned short& rPort) ;

    bool getTurnRemoteAddress(char* szIp, unsigned short& rPort) ;

    bool getData(char*& rpData, unsigned short& nLength) ;

    bool getRelayAddress(char* szIp, unsigned short& rPort) ;

    bool getRequestedTransport(TURN_TRANSPORT_TYPE& rTransportType) ;

    bool getRequestedIp(char* szIp, unsigned short& rPort) ;

/* ============================ INQUIRY =================================== */

    virtual bool validateMessageType(unsigned short type) ;

    static bool isTurnMessage(const char* pBuf, unsigned short nBufLength) ;

    virtual bool isRequestOrNonErrorResponse() ;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

    virtual bool parseAttribute(STUN_ATTRIBUTE_HEADER* pHeader, char* pBuf) ;

    bool encodeAttributeLong(unsigned short type, 
                             unsigned long value, 
                             char*& pBuf, 
                             size_t& nBytesLeft) ;

    bool encodeAttributeRaw(unsigned short type, 
                            const char* cBuf, 
                            size_t length, 
                            char*& pBuf, 
                            size_t& nBytesLeft) ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
    unsigned long mLifetime ;                       // ATTR_TURN_LIFETIME
    bool mbLifetimeValid ;

    unsigned long mBandwidth ;                      // ATTR_TURN_BANDWIDTH
    bool mbBandwidthValid ;

    STUN_ATTRIBUTE_ADDRESS mDestinationAddress ;    // ATTR_TURN_DESTINATION_ADDRESS
    bool mbDestinationAddressValid ;

    STUN_ATTRIBUTE_ADDRESS mTurnRemoteAddress ;     // ATTR_TURN_REMOTE_ADDRESS
    bool mbTurnRemoteAddressValid ;

    char* mszTurnData ;                             // ATTR_TURN_DATA
    size_t mnTurnData ;
    bool mbTurnDataValid ;
    
    STUN_ATTRIBUTE_ADDRESS mRelayAddress ;          // ATTR_TURN_RELAY_ADDRESS
    bool mbRelayAddressValid ;
    
    unsigned long mTransport ;                      // ATTR_TURN_REQUESTED_TRANSPORT
    bool mbTransportValid ;

    STUN_ATTRIBUTE_ADDRESS mRequestedIp ;           // ATTR_TURN_REQUESTED_IP
    bool mbRequestedIpValid ;

   /** Disabled Copy constructor */
   TurnMessage(const TurnMessage& rTurnMessage);     

   /** Disabled Assignment operator */
   TurnMessage& operator=(const TurnMessage& rhs);  


};

/* ============================ INLINE METHODS ============================ */

#endif  // _TurnMessage_h_
