// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPIMPLSUB_H
#define SIPIMPLSUB_H

/**
 * SIP Registrar Implied Subscriptions
 *
 * The SipImpliedSubscriptions::checkAndSend method is invoked by
 *   the Registrar whenever a REGISTER request succeeds.  This object determines
 *   whether or not the register needs to generate any SUBSCRIBE requests on behalf
 *   of the originator of the REGISTER, and if so, creates and sends those SUBSCRIBE
 *   requests.
 */

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "net/SipNonceDb.h"
#include "net/SipUserAgent.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * The Registrar Server is responsible for registering and unregistering
 * phones.  The servertask also looks up contacts for invite URI's. These
 * contacts are taken from the Url Mapping rules and also from the
 * registration database.
 */
class SipImpliedSubscriptions
{
public:
    /** ctor */
    SipImpliedSubscriptions()
      {
      }


    /** dtor */
    virtual ~SipImpliedSubscriptions()
      {
      }

    void SipImpliedSubscriptions::readConfig( OsConfigDb& configDb );

    void checkAndSend( const SipMessage&   registerMessage
                      ,const unsigned int  registrationDuration
                      ,SipNonceDb&         nonceDb
                      ,SipUserAgent*       sipUserAgent
                      );

private:
//  ConfigPrefix -
//     The registrar-config file may contain any number of directives
//     that begin with this prefix - the value of each is a regular
//     expression that matches a User-Agent header value.  When a matching
//     REGISTER request is received,
    static const char ConfigPrefix[];

    void requestMwiSubscription( const SipMessage& registerMessage
                                ,const unsigned int registrationDuration
                                ,SipNonceDb&         nonceDb
                                ,SipUserAgent*       sipUserAgent
                                );
};

#endif // SIPIMPLSUB_H

