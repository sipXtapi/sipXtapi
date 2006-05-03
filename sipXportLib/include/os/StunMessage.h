//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _StunMessage_h_
#define _StunMessage_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlRandom.h"

// DEFINES
#ifndef MIN
#define MIN(a, b) (((a)<(b)) ? (a) : (b) )
#endif

#define STUN_MAX_STRING_LENGTH              128 
#define STUN_MAX_UNKNOWN_ATTRIBUTES         16
#define STUN_MAX_MESSAGE_INTEGRITY_LENGTH   20
#define STUN_MIN_CHAR_PAD                   4


/**
 * STUN Message IDs
 */
#define MSG_STUN_BIND_REQUEST                   0x0001
#define MSG_STUN_BIND_RESPONSE                  0x0101
#define MSG_STUN_BIND_ERROR_RESPONSE            0x0111
#define MSG_STUN_SHARED_SECRET_REQUEST          0x0002
#define MSG_STUN_SHARED_SECRET_RESPONSE         0x0102
#define MSG_STUN_SHARED_SECRET_ERROR_RESPONSE   0x0112


/**
 * STUN attribute IDs
 */
#define ATTR_STUN_MAPPED_ADDRESS                0x0001
#define ATTR_STUN_RESPONSE_ADDRESS              0x0002
#define ATTR_STUN_CHANGE_REQUEST                0x0003
#define ATTR_STUN_SOURCE_ADDRESS                0x0004
#define ATTR_STUN_CHANGED_ADDRESS               0x0005
#define ATTR_STUN_USERNAME                      0x0006
#define ATTR_STUN_PASSWORD                      0x0007
#define ATTR_STUN_MESSAGE_INTEGRITY             0x0008
#define ATTR_STUN_ERROR_CODE                    0x0009
#define ATTR_STUN_UNKNOWN_ATTRIBUTE             0x000A
#define ATTR_STUN_REFLECTED_FROM                0x000B
#define ATTR_STUN_XOR_MAPPED_ADDRESS            0x0020
#define ATTR_STUN_XOR_ONLY                      0x0021
#define ATTR_STUN_SERVER                        0x0022

#define ATTR_CHANGE_FLAG_PORT                   0x0002
#define ATTR_CHANGE_FLAG_IP                     0x0004

#define ATTR_ADDRESS_FAMILY_IPV4                0x01
#define ATTR_ADDRESS_FAMILY_IPV6                0x02

// Error Defines
#define STUN_ERROR_BAD_REQUEST_CODE             400
#define STUN_ERROR_BAD_REQUEST_TEXT             "Bad request"

#define STUN_ERROR_UNAUTHORIZED_CODE            401
#define STUN_ERROR_UNAUTHORIZED_TEXT            "Unauthorized"

#define STUN_ERROR_UNKNOWN_ATTRIBUTE_CODE       420
#define STUN_ERROR_UNKNOWN_ATTRIBUTE_TEXT       "Unknown attribute"

#define STUN_ERROR_STALE_CREDENTIAL_CODE        430
#define STUN_ERROR_STALE_CREDENTIAL_TEXT        "Stale credential"

#define STUN_ERROR_BAD_INTEGRITY_CODE           431
#define STUN_ERROR_BAD_INTEGRITY_TEXT           "Bad integrity"

#define STUN_ERROR_MISSING_USERNAME_CODE        432
#define STUN_ERROR_MISSING_USERNAME_TEXT        "Missing username"

#define STUN_ERROR_USE_TLS_CODE                 433
#define STUN_ERROR_USE_TLS_TEXT                 "Use TLS"

#define STUN_ERROR_SERVER_CODE                  500
#define STUN_ERROR_SERVER_TEXT                  "Server error"

#define STUN_ERROR_GLOBAL_CODE                  600
#define STUN_ERROR_GLOBAL_TEXT                  "Global error"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

typedef struct {
    unsigned char id[16];
} STUN_TRANSACTION_ID;

typedef struct
{
    unsigned short type ;
    unsigned short length ;
    STUN_TRANSACTION_ID transactionId ;
} STUN_MESSAGE_HEADER ;

typedef struct
{
    unsigned short type;
    unsigned short length;
} STUN_ATTRIBUTE_HEADER ;

typedef struct
{
    unsigned char unused ;
    unsigned char family ;  /* unused today */
    unsigned short port;
    unsigned long address ;    
} STUN_ATTRIBUTE_ADDRESS;

typedef struct
{
    unsigned short unused ;
    unsigned char unused2:4 ;
    unsigned char errorClass:4 ;   // between 1 and 6
    unsigned char errorNumber ; // between 0 and 99
    char szReasonPhrase[STUN_MAX_STRING_LENGTH+1] ;  
} STUN_ATTRIBUTE_ERROR ;

typedef struct
{
    size_t nTypes ;
    unsigned short type[STUN_MAX_UNKNOWN_ATTRIBUTES] ;
} STUN_ATTRIBUTE_UNKNOWN ;


// FORWARD DECLARATIONS

/**
 * A StunMessage includes all of the parse and encoding for a STUN message
 * as defined by draft-ietf-behave-rfc3489bis-02.txt:
 *
 * 
 * http://www.jdrosen.net/papers/draft-ietf-behave-rfc3489bis-02.txt
 *
 * All data is stored internally in host byte order
 *
                                        Binding  Shared  Shared  Shared
                      Binding  Binding  Error    Secret  Secret  Secret
   Att.                Req.     Resp.    Resp.    Req.    Resp.   Error
                                                                  Resp.
   _____________________________________________________________________
   MAPPED-ADDRESS      N/A      M        N/A      N/A     N/A     N/A
   RESPONSE-ADDRESS    O        N/A      N/A      N/A     N/A     N/A
   CHANGE-REQUEST      O        N/A      N/A      N/A     N/A     N/A
   SOURCE-ADDRESS      N/A      M        N/A      N/A     N/A     N/A
   CHANGED-ADDRESS     N/A      M        N/A      N/A     N/A     N/A
   USERNAME            O        N/A      N/A      N/A     M       N/A
   PASSWORD            N/A      N/A      N/A      N/A     M       N/A
   MESSAGE-INTEGRITY   O        O        N/A      N/A     N/A     N/A
   ERROR-CODE          N/A      N/A      M        N/A     N/A     M
   UNKNOWN-ATTRIBUTES  N/A      N/A      C        N/A     N/A     C
   REFLECTED-FROM      N/A      C        N/A      N/A     N/A     N/A
   XOR-MAPPED-ADDRESS  N/A      M        N/A      N/A     N/A     N/A
   XOR-ONLY            O        N/A      N/A      N/A     N/A     N/A
   SERVER              N/A      O        O        N/A     O       O

 */
class StunMessage
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

    /**
     * Default constructor
     */
    StunMessage() ;


    /**
     * Destructor
     */
    virtual ~StunMessage() ;

/* ============================ MANIPULATORS ============================== */

    virtual void reset() ;

    virtual bool parse(const char* pBuf, size_t nBufLength) ;
    
    virtual bool encode(char* pBuf, size_t nBufLength, size_t& nActualLength) ;

    virtual bool encodeBody(char* pBuf, size_t nBufLength, size_t& nBytesUsed) ;

    void setTransactionId(STUN_TRANSACTION_ID& rTransactionId) ;

    void allocTransactionId() ;

    void setType(unsigned short type) ;

    void setMappedAddress(const char* szIp, const unsigned short port) ;

    void setResponseAddress(const char* szIp, const unsigned short port) ;

    void setChangePort(const bool bChange) ;

    void setChangeIp(const bool bChange) ;

    void setSourceAddress(const char* szIp, const unsigned short port) ;

    void setChangedAddress(const char* szIp, const unsigned short port) ;

    void setUsername(const char* szUsername) ;

    void setPassword(const char* szPassword) ;

    void setError(const unsigned short code, const char* szReason) ;

    void addUnknownAttribute(unsigned short attributeId) ;

    void setReflectedFrom(const char* szIp, const unsigned short port) ;

    void setServer(const char* szServer) ;

    void setRequestXorOnly() ;

    void setSendXorOnly() ;

    void setIncludeMessageIntegrity(bool bInclude) ;

/* ============================ ACCESSORS ================================= */

    void getTransactionId(STUN_TRANSACTION_ID* pTransactionId) ;

    unsigned short getType() ;

    bool getMappedAddress(char* szIp, unsigned short& rPort) ;

    bool getResponseAddress(char* szIp, unsigned short& rPort) ;

    bool getChangePort() ;

    bool getChangeIp() ;

    bool getSourceAddress(char* szIp, unsigned short& rPort) ;

    bool getChangedAddress(char* szIp, unsigned short& rPort) ;

    bool getUsername(char* szUsername) ;

    bool getPassword(char* szPassword) ;

    bool getMessageIntegrity(char* cMessageIntegrity) ;

    bool getError(unsigned short& rCode, char* szReason) ;

    bool getUnknownAttributes(unsigned short* pList, size_t nMaxItems, size_t& nActualItems) ;

    bool getReflectedFrom(char* szIp, unsigned short& rPort) ;

    bool getServer(char* szServer) ;

    bool getUnknownParsedAttributes(unsigned short* pList, size_t nMaxItems, size_t& nActualItems) ;

    bool getRequestXorOnly() ;

/* ============================ INQUIRY =================================== */

    virtual bool validateMessageType(unsigned short type) ;

    static bool isStunMessage(const char* pBuf, unsigned short nBufLength) ;

    virtual bool isRequestOrNonErrorResponse() ;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

    bool encodeByte(char c, char*& pBuf, size_t& nBytesLeft) ;

    bool encodeShort(unsigned short value, char*& pBuf, size_t& nBytesLeft) ;

    bool encodeLong(unsigned long value, char*& pBuf, size_t& nBytesLeft) ;

    bool encodeRaw(const char* cRaw, size_t length, char*& pBuf, size_t& nBytesLeft) ;

    bool encodeHeader(STUN_MESSAGE_HEADER* pHeader, char*& pBuf, size_t& nBytesLeft) ;

    bool encodeAttributeHeader(short type, short length, char*& pBuf, size_t& nBytesLeft) ;

    bool encodeAttributeAddress(unsigned short type, STUN_ATTRIBUTE_ADDRESS* pAddress, char*& pBuf, size_t& nBytesLeft) ;

    bool encodeXorAttributeAddress(unsigned short type, STUN_ATTRIBUTE_ADDRESS* pAddress, char*& pBuf, size_t& nBytesLeft) ;

    bool encodeString(unsigned short type, const char* szString, char*& pBuf, size_t& nBytesLeft) ;

    bool encodeAttributeError(STUN_ATTRIBUTE_ERROR* pError, char*& pBuf, size_t& nBytesLeft) ;

    bool encodeAttributesUnknown(STUN_ATTRIBUTE_UNKNOWN* pAttributes, char*& pBuf, size_t& nBytesLeft) ;

    virtual bool parseAttribute(STUN_ATTRIBUTE_HEADER* pHeader, char* pBuf) ;

    bool parseAddressAttribute(char *pBuf, size_t nLength, STUN_ATTRIBUTE_ADDRESS* pAddress) ;

    bool parseXorAddressAttribute(char *pBuf, size_t nLength, STUN_ATTRIBUTE_ADDRESS* pAddress) ;

    bool parseShortAttribute(char *pBuf, size_t nLength, unsigned short* pShort) ;

    bool parseLongAttribute(char *pBuf, size_t nLength, unsigned long* pLong) ;

    bool parseStringAttribute(char* pBuf, size_t nLength, char* pString) ;

    bool parseRawAttribute(char* pBuf, size_t nLength, char* pDest, size_t nDest) ;

    bool parseErrorAttribute(char *pBuf, size_t nLength, STUN_ATTRIBUTE_ERROR* pError) ;

    bool parseUnknownAttribute(char* pBuf, size_t nLength, STUN_ATTRIBUTE_UNKNOWN* pAttributes) ;

    /**
     * zero pads to 64 boundry and results will be 20 bytes long for hmac/sha1
     */
    void calculateHmacSha1(char *pDataIn, size_t nDataIn, char *pKey, size_t nKey, char* results) ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
    STUN_MESSAGE_HEADER    mMsgHeader ;
    bool                   mbMsgHeaderValid ;
    STUN_ATTRIBUTE_ADDRESS mMappedAddress ;
    bool                   mbMappedAddressValid ;
    STUN_ATTRIBUTE_ADDRESS mResponseAddress ;
    bool                   mbResponseAddressValid ;
    STUN_ATTRIBUTE_ADDRESS mChangedAddress ;
    bool                   mbChangedAddressValid ;
    unsigned long          mChangeRequest ;
    bool                   mbChangeRequestValid ;
    STUN_ATTRIBUTE_ADDRESS mSourceAddress ;
    bool                   mbSourceAddressValid ;
    char                   mUsername[STUN_MAX_STRING_LENGTH+1] ;
    bool                   mbUsernameValid ;
    char                   mPassword[STUN_MAX_STRING_LENGTH+1] ;
    bool                   mbPasswordValid ;
    char                   mMessageIntegrity[STUN_MAX_MESSAGE_INTEGRITY_LENGTH] ;
    bool                   mbMessageIntegrityValid ;
    STUN_ATTRIBUTE_ERROR   mError ;
    bool                   mbErrorValid;
    STUN_ATTRIBUTE_UNKNOWN mUnknownAttributes ;
    bool                   mbUnknownAttributesValid ;    
    STUN_ATTRIBUTE_ADDRESS mReflectedFrom ;
    bool                   mbReflectedFromValid ;
    char                   mServer[STUN_MAX_STRING_LENGTH+1];
    bool                   mbServerValid ;
    bool                   mbSendXorOnly ;
    bool                   mbRequestXorOnly ;
    UtlRandom              mbRandomGenerator ;
    bool                   mbIncludeMessageIntegrity ;

    STUN_ATTRIBUTE_UNKNOWN mUnknownParsedAttributes ;

   /** Disabled copy constructor */
   StunMessage(const StunMessage& rStunMessage);   

   /** Disabled equals operator */
   StunMessage& operator=(const StunMessage& rhs);  
};

/* ============================ INLINE METHODS ============================ */

#endif  // _StunMessage_h_
