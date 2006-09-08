// $Id$
//
// Copyright (C) 2006 Pingtel Corp.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _OsNatKeepaliveListener_h_	/* [ */
#define _OsNatKeepaliveListener_h_

// SYSTEM INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"

// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef enum
{
    OS_NAT_KEEPALIVE_CRLF,
    OS_NAT_KEEPALIVE_STUN,
    OS_NAT_KEEPALIVE_SIP_PING,
    OS_NAT_KEEPALIVE_SIP_OPTIONS,

    OS_NAT_KEEPALIVE_INVALID = -1,

} OS_NAT_KEEPALIVE_TYPE ;

// FORWARD DECLARATIONS

/**
 * Simple event object/container for the OsNatKeepaliveListener.
 */ 
class OsNatKeepaliveEvent
{
public:
    // Default Constructor
    OsNatKeepaliveEvent()
    {
        type = OS_NAT_KEEPALIVE_INVALID ;
        remotePort = PORT_NONE ;
        keepAliveSecs = 0 ;
        mappedPort = 0 ;
    } ;

    // Default Destructor
    virtual ~OsNatKeepaliveEvent() { } ;

    // Copy Constructor
    OsNatKeepaliveEvent(const OsNatKeepaliveEvent& r)
    {
        type          = r.type ;
        remoteAddress = r.remoteAddress ;
        remotePort    = r.remotePort ;
        keepAliveSecs = r.keepAliveSecs ;
        mappedAddress = r.mappedAddress ;
        mappedPort    = r.mappedPort ;          
    } ;

    // Equals operator
    OsNatKeepaliveEvent& operator=(const OsNatKeepaliveEvent& r)
    {
        type          = r.type ;
        remoteAddress = r.remoteAddress ;
        remotePort    = r.remotePort ;
        keepAliveSecs = r.keepAliveSecs ;
        mappedAddress = r.mappedAddress ;
        mappedPort    = r.mappedPort ;
    } ;

    // Attribute
    OS_NAT_KEEPALIVE_TYPE type ;
    UtlString             remoteAddress ;
    int                   remotePort ;
    int                   keepAliveSecs ;
    UtlString             mappedAddress ;
    int                   mappedPort ;
} ;


/**
 * Simple listener interface for keepalive events
 */
class OsNatKeepaliveListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
public:
    OsNatKeepaliveListener() {} ;
    virtual ~OsNatKeepaliveListener() {} ;

/* ============================ MANIPULATORS ============================== */

    /**
     * A keepalive has been started
     */
    virtual void OnKeepaliveStart(const OsNatKeepaliveEvent& event) = 0 ;

    /**
     * A keepalive has been disabled
     */
    virtual void OnKeepaliveStop(const OsNatKeepaliveEvent& event) = 0 ;

    /**
     * A keepalive response has been received (not applicable to CrLf 
     * keepalives)
     */
    virtual void OnKeepaliveFeedback(const OsNatKeepaliveEvent& event) = 0 ;

    /**
     * A keepalive failure occured (e.g. unable to send -- failures are not
     * send if responses are not received)
     */
    virtual void OnKeepaliveFailure(const OsNatKeepaliveEvent& event) = 0 ;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   
};

/* ============================ INLINE METHODS ============================ */

#endif  /* _OsNatKeepaliveListener_h_ ] */
