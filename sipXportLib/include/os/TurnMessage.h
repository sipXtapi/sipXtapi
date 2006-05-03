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
#define MSG_TURN_ALLOCATE_REQUEST               0x0003
#define MSG_TURN_ALLOCATE_RESPONSE              0x0103
#define MSG_TURN_ALLOCATE_ERROR_RESPONSE        0x0113
#define MSG_TURN_SEND_REQUEST                   0x0004
#define MSG_TURN_SEND_RESPONSE                  0x0104
#define MSG_TURN_SEND_ERROR_RESPONSE            0x0114
#define MSG_TURN_DATA_INDICATION                0x0115
#define MSG_TURN_ACTIVE_DESTINATION_REQUEST     0x0006
#define MSG_TURN_ACTIVE_DESTINATION_RESPONSE    0x0106
#define MSG_TURN_ACTIVE_DESTINATION_ERROR_RESPONSE   0x0116

/**
 * TURN attribute IDs
 */
#define ATTR_TURN_LIFETIME                      0x000d
#define ATTR_TURN_ALTERNATE_SERVER              0x000e
#define ATTR_TURN_MAGIC_COOKIE                  0x000f
#define ATTR_TURN_BANDWIDTH                     0x0010
#define ATTR_TURN_DESTINATION_ADDRESS           0x0011
#define ATTR_TURN_SOURCE_ADDRESS                0x0012
#define ATTR_TURN_DATA                          0x0013
#define ATTR_TURN_NONCE                         0x0014
#define ATTR_TURN_REALM                         0x0015

#define ATTR_MAGIC_COOKIE                       0x72c64bc6

// Error Defines
#define TURN_ERROR_TRY_ALT_CODE                 300
#define TURN_ERROR_TRY_ALT_TEXT                 "Try alternate"

#define TURN_ERROR_MISSING_REAL_CODE            434
#define TURN_ERROR_MISSING_REAL_TEXT            "Missing realm"

#define TURN_ERROR_MISSING_NONCE_CODE           435
#define TURN_ERROR_MISSING_NONCE_TEXT           "Missing nonce"

#define TURN_ERROR_UNKNOWN_USERNAME_CODE        436
#define TURN_ERROR_UNKNOWN_USERNAME_TEXT        "Unknown username"

#define TURN_ERROR_NO_BINDING_CODE              437
#define TURN_ERROR_NO_BINDING_TEXT              "No binding"

#define TURN_ERROR_SENDING_DISALLOWED_CODE      438
#define TURN_ERROR_SENDING_DISALLOWED_TEXT      "Sending Failed"

#define TURN_ERROR_TRANSITIONING                439
#define TURN_ERROR_TRANSITIONING_TEXT           "Transitioning"

#define TURN_ERROR_NO_DESTINATION               440
#define TURN_ERROR_NO_DESTINATION_TEXT          "No Destintiation"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
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

    void setAltServer(const char* szIp, unsigned short port) ;

    void setBandwidth(unsigned long rKBPS) ;

    void setDestinationAddress(const char* szIp, unsigned short port) ;

    void setTurnSourceAddress(const char* szIp, unsigned short port) ;

    void setData(const char* pData, unsigned short nLength) ;

/* ============================ ACCESSORS ================================= */

    bool getLifetime(unsigned long& rSecs) ;

    bool getAltServer(char* szIp, unsigned short& rPort) ;

    bool getBandwidth(unsigned long& rKBPS) ;

    bool getDestinationAddress(char* szIp, unsigned short& rPort) ;

    bool getTurnSourceAddress(char* szIp, unsigned short& rPort) ;

    bool getData(char*& rpData, unsigned short& nLength) ;

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
    unsigned long mLifetime ;
    bool mbLifetimeValid ;

    unsigned long mBandwidth ;
    bool mbBandwidthValid ;

    STUN_ATTRIBUTE_ADDRESS mAltServer ;
    bool mbAltServerValid ;

    STUN_ATTRIBUTE_ADDRESS mDestinationAddress ;
    bool mbDestinationAddressValid ;

    STUN_ATTRIBUTE_ADDRESS mTurnSourceAddress ;
    bool mbTurnSourceAddressValid ;

    char* mszTurnData ;
    size_t mnTurnData ;
    bool mbTurnDataValid ;

   /** Disabled Copy constructor */
   TurnMessage(const TurnMessage& rTurnMessage);     

   /** Disabled Assignment operator */
   TurnMessage& operator=(const TurnMessage& rhs);  


};

/* ============================ INLINE METHODS ============================ */

#endif  // _TurnMessage_h_
