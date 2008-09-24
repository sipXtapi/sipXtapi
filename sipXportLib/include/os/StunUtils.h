//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _StunUtils_h_
#define _StunUtils_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDatagramSocket.h"
#include "os/StunMessage.h"
#include "os/TurnMessage.h"

// DEFINES
#define DEFAULT_STUN_PORT       3478
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef enum 
{
    STUN_NAT_ERROR_BAD_SERVER,
    STUN_NAT_BLOCKED,
    STUN_NAT_SYMMETRIC_FIREWALL,
    STUN_NAT_OPEN,
    STUN_NAT_FULL_CONE,
    STUN_NAT_RESTRICTED_CONE,
    STUN_NAT_PORT_RESTRICTED_CONE,
    STUN_NAT_SYMMETRIC
} STUN_NAT_TYPE ;

// FORWARD DECLARATIONS

/**
 * Various Stun Utility helpers
 */
class StunUtils
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

static STUN_NAT_TYPE determineStunNatType(const char* szServer, const int port) ;

static bool sendStunNatTest(OsDatagramSocket* pSocket,
                        const char* szServerIp,
                        const int port,
                        bool bChangePort,
                        bool bChangeIP,
                        char* szMappedIp,
                        unsigned short* piMappedPort,
                        char* szChangedIp,
                        unsigned short* piChangedPort) ;

static bool allocateTurnAddress(OsDatagramSocket* pSocket,
                                const char* szServerIp,
                                const int port,
                                char* szRelayIp,
                                unsigned short* piRelayPort,
                                unsigned long* plLifetime) ;


static const char* natTypeToString(STUN_NAT_TYPE type) ;

static void debugDump(char* pPacket, size_t nPacket, UtlString& output) ;


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

    /** Private Default constructor -- use statics */
    StunUtils();
     
    /** Disabled Copy constructor */
    StunUtils(const StunUtils& rStunUtils);     

    /** Disables equals operator */
    StunUtils& operator=(const StunUtils& rhs);  

    /** Private Destructor -- use statics */
    virtual ~StunUtils();
};

/* ============================ INLINE METHODS ============================ */

#endif  // _StunUtils_h_
