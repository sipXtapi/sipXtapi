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

/**
 * SIP Registrar Implied Subscriptions
 *
 * The SipImpliedSubscriptions::doImpliedSubscriptions method is invoked by
 *   the Registrar whenever a REGISTER request succeeds.  This object determines
 *   whether or not the register needs to generate any SUBSCRIBE requests on behalf
 *   of the originator of the REGISTER, and if so, creates and sends those SUBSCRIBE
 *   requests.
 *
 * Configuration is in the registrar-config file.  Directives that begin with the
 *   ConfigPrefix (const below) specify a regular expression to be checked against
 *   the User-Agent value in the REGISTER request.  When it matches, this module
 *   generates a SUBSCRIBE for message waiting indication on behalf of the phone.
 */


// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "utl/UtlRegex.h"
#include "os/OsDateTime.h"
#include "os/OsSysLog.h"
#include "os/OsConfigDb.h"
#include "net/SipUserAgentStateless.h"
#include "net/NetMd5Codec.h"
#include "net/SipRefreshMgr.h"
#include "sipdb/CredentialDB.h"
#include "SipRegistrarServer.h"
#include "SipImpliedSubscriptions.h"
#include "utl/UtlContainable.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
// GLOBAL VARIABLES


// ImpliedSubscriptionUserAgent - private configuration class
//    Each of these records one implied subscription directive
//      from the registrar configuration file.
//    The mName member is the part of the configuration directive
//      that follows the prefix constant above.
//    These are created as members of the configuredUserAgents
//      list, below.
class ImpliedSubscriptionUserAgent : public UtlContainable
{
public:
  ImpliedSubscriptionUserAgent( const UtlString& name
                               ,const UtlString& recognizer
                               )
    :
    mName( name )
  {
     mUserAgentRegEx = NULL;
     try
     {
        mUserAgentRegEx = new RegEx(recognizer.data());
     }
     catch(const char* compileError)
     {
        OsSysLog::add(FAC_SIP, PRI_ERR,
                      "SipImpliedSubscriptions: Invalid recognizer expression for '%s':"
                      "expression='%s': %s", name.data(), recognizer.data(), compileError
                      );
     }
  }

  ~ImpliedSubscriptionUserAgent()
  {
  }

  UtlBoolean matchesRecognizer(const UtlString& rcvdUA ) const
  {
    UtlBoolean matched = FALSE;
    if (mUserAgentRegEx) // NULL if recognizer did not compile
    {
       OsSysLog::add( FAC_SIP, PRI_DEBUG
                     ,"ImpliedSubscriptionUserAgent checking %s: %s"
                     ,mName.data(), rcvdUA.data()
                     );
       matched = mUserAgentRegEx->Search(rcvdUA);
    }
    
    return matched;
  }

  /**
   * Calculate a unique hash code for this object.  If the equals
   * operator returns true for another object, then both of those
   * objects must return the same hashcode.
   */    
  virtual unsigned hash() const 
  {
    return (unsigned) this ;
  }

  /**
   * Get the ContainableType for a UtlContainable derived class.
   */
  virtual UtlContainableType getContainableType() const
  {
    return "ImpliedSubscriptionUserAgent" ;
  }

  /**
   * Compare the this object to another like-objects.  Results for 
   * designating a non-like object are undefined.
   *
   * @returns 0 if equal, < 0 if less then and >0 if greater.
   */
  virtual int compareTo(UtlContainable const* inVal) const 
  {
    int result ; 

    if (inVal->isInstanceOf(getContainableType()))
    {
      result = (this == inVal) ;
    }
    else
    {
      result = -1; 
    }

    return result;
  }

UtlString mName;

private:
  
  RegEx*   mUserAgentRegEx;
};

// ConfiguredUserAgents - private class
//    This is the list of ImpliedSubscriptionUserAgent objects;
//      one per configuration directive.
class ConfiguredUserAgents
{
public:
  ConfiguredUserAgents()
  {
  }

  ~ConfiguredUserAgents()
  {
  }


  void add( const UtlString& name
           ,const UtlString& recognizer
           )
  {
    mConfiguredUserAgents.append(new ImpliedSubscriptionUserAgent( name, recognizer ));
  }


  ImpliedSubscriptionUserAgent* configurationName( const UtlString& name )
  {
    ImpliedSubscriptionUserAgent* pAgent ;

    UtlSListIterator itor(mConfiguredUserAgents) ;    
    while ((pAgent = (ImpliedSubscriptionUserAgent*) itor()))
    {
      if (pAgent->matchesRecognizer(name))
      {
        break;
      }       
    }

    return pAgent ;
  }

private:
  UtlSList mConfiguredUserAgents ;

};

// the only instance of ConfiguredUserAgents
static ConfiguredUserAgents configuredUserAgents;

const char SipImpliedSubscriptions::ConfigPrefix[]
= "SIP_REGISTRAR_MWI_SUBSCRIBE.";

void SipImpliedSubscriptions::readConfig( OsConfigDb& configDb )
{
  OsConfigDb impliedSubscriptionConfig;
  OsStatus   found;
  UtlString   key;
  UtlString   name;
  UtlString   recognizer;

  // extract the database of implied message waiting subscriptions
  configDb.getSubHash( ConfigPrefix
                      ,impliedSubscriptionConfig
                      );
  for ( key = "", found = impliedSubscriptionConfig.getNext( key
                                                            ,name
                                                            ,recognizer
                                                            );
        found == OS_SUCCESS;
        key = name, found = impliedSubscriptionConfig.getNext( key
                                                              ,name
                                                              ,recognizer
                                                              )
       )
    {
      OsSysLog::add( FAC_SIP, PRI_INFO
                    ,"SipImpliedSubscriptions::readConfig name=\"%s\" recognizer=\"%s\""
                    ,name.data(), recognizer.data()
                    );
      configuredUserAgents.add( name, recognizer );
    }
}


/**
 * doImpliedSubscriptions checks for characteristics in the REGISTER message
 *    that imply that a subscription needs to be requested on behalf of the
 *    party doing the registration, and then invokes the appropriate subscription
 *    request method.
 */

void SipImpliedSubscriptions::checkAndSend( const SipMessage&   registerMessage
                                           ,const unsigned int  registrationDuration
                                           ,SipNonceDb&         nonceDb
                                           ,SipUserAgent*       sipUserAgent
                                           )
{
  UtlString userAgent;
  ImpliedSubscriptionUserAgent* configured;

  registerMessage.getUserAgentField( &userAgent );

  OsSysLog::add( FAC_SIP, PRI_DEBUG
                ,"SipImpliedSubscriptions checking User-Agent \"%s\""
                ,userAgent.data()
                );

  configured = configuredUserAgents.configurationName( userAgent );

  if ( NULL != configured ) // ? did we find a configuration name whose recognizer matched ?
    {
      OsSysLog::add( FAC_SIP, PRI_INFO
                    ,"SipImpliedSubscriptions User-Agent \"%s\" matched rule \"%s%s\""
                    ,userAgent.data()
                    ,ConfigPrefix
                    ,configured->mName.data()
                    );

      // This phone - accepts message waiting notifies,
      // but don't subscribe to them, so we'll do it for them.
      requestMwiSubscription( registerMessage, registrationDuration, nonceDb, sipUserAgent );
    }
}

/**
 * requestMwiSubscription attempts to construct and send a message waiting indication
 * subscription on behalf of the new registration.  It's a best-effort thing - we don't
 * attempt to determine whether or not it works, because there's no one to report the
 * error to if it doesn't.  Hence, no returned status.
 */
void SipImpliedSubscriptions::requestMwiSubscription( const SipMessage& registerMessage
                                                     ,const unsigned int registrationDuration
                                                     ,SipNonceDb&         nonceDb
                                                     ,SipUserAgent*       sipUserAgent
                                                     )
{
  UtlString registrationValue;
  UtlString fromUri;
  UtlString tagNameValuePair;
  UtlString contactUri;
  UtlString callId;
  int      sequenceNumber = 0;

  OsSysLog::add( FAC_SIP, PRI_DEBUG
                ,"SipImpliedSubscriptions requesting mwi subscription duration=%d"
                ,registrationDuration
                );

  // Get the From URL, and change the tag
  Url fromUrl;
  registerMessage.getFromUrl( fromUrl );
  fromUrl.removeFieldParameter("tag"); // discard from tag from REGISTER
  registerMessage.getFromUri( &fromUri );
  (void) registerMessage.getContactUri(0, &contactUri);
  (void) registerMessage.getCSeqField(&sequenceNumber, &registrationValue);

  registerMessage.getCallIdField( &callId );
  callId.prepend("pingtel-");


  // Build a from tag for the SUBSCRIBE
  //   - hash the call id so that it will be the same on each refresh
  UtlString callIdHash;
  NetMd5Codec::encode( callId.data(), callIdHash );
  fromUrl.setFieldParameter("tag", callIdHash.data() );
  UtlString fromTag( callIdHash ); // for constructing the nonce

  SipMessage subscribeMessage;

  subscribeMessage.setVoicemailData( fromUrl.toString() // From:
                                    ,fromUri.data()     // To:
                                    ,fromUri.data()     // request URI
                                    ,contactUri.data()  // taken from registration
                                    ,callId.data()
                                    ,++sequenceNumber
                                    ,registrationDuration
                                    );

  // Construct authentication that the status server will accept
  // We need the user credentials, and a signed nonce like the one
  //    the status server would have generated to challenge this phone.
  UtlString user;
  UtlString realm;
  UtlString registrationNonce;
  UtlString opaque;
  UtlString response;
  UtlString authUri;

  // extract the identity from the authorization of the registration
  if ( registerMessage.getDigestAuthorizationData( &user, &realm       // the identity
                                                  ,NULL // request nonce not used
                                                  ,&opaque            // passed through to aid debugging
                                                  ,NULL, NULL // response & authUri not used
                                                  ,HttpMessage::SERVER, 0
                                                  )
      )
    {
      Url subscribeUser;
      UtlString passToken;
      UtlString authType;

      if (CredentialDB::getInstance()->getCredential( user, realm, subscribeUser
                                                     ,passToken, authType
                                                     )
          )
        {
          // Construct a nonce
          UtlString serverNonce;
          UtlString clientNonce;
          nonceDb.createNewNonce( callId, fromTag, fromUri
                                 ,realm ,serverNonce
                                 );

          // generate a client nonce - doesn't matter what it is, really
          //   because the server doesn't validate this one;
          //   but change an input so that the two won't be the same
          //              UtlString dummyFromTag("different value");
          //              mRegistrarNonceDb.createNewNonce( callId, dummyFromTag, fromUri
          //                                               ,clientNonce
          //                               );

          // Sign the message
          UtlString responseHash;
          HttpMessage::buildMd5Digest(passToken.data(),
                                      HTTP_MD5_ALGORITHM,
                                      serverNonce.data(),
                                      NULL, // client nonce
                                      1, // nonce count
                                      "",
                                      SIP_SUBSCRIBE_METHOD,
                                      fromUri.data(),
                                      NULL,
                                      &responseHash
                                      );

          subscribeMessage.removeHeader( HTTP_AUTHORIZATION_FIELD, 0);
          subscribeMessage.setDigestAuthorizationData(user.data(),
                                                      realm.data(),
                                                      serverNonce.data(),
                                                      fromUri.data(),
                                                      responseHash.data(),
                                                      HTTP_MD5_ALGORITHM,
                                                      NULL,//clientNonce.data(),
                                                      opaque.data(),
                                                      HTTP_QOP_AUTH,
                                                      1, // nonce count
                                                      HttpMessage::SERVER
                                                      );

          OsSysLog::add( FAC_SIP, PRI_INFO,
                        "SipImpliedSubscriptions requesting Mwi on behalf of \"%s\"", fromUri.data());

          sipUserAgent->send( subscribeMessage, NULL, NULL );
        }
      else
        {
          OsSysLog::add( FAC_SIP, PRI_WARNING,
                        "SipImpliedSubscriptions failed implied subscription request: no credentials found for \"%s\"", user.data());
        }
    }
  else
    {
      OsSysLog::add( FAC_SIP, PRI_WARNING,
                    "SipImpliedSubscriptions failed implied subscription request: no credentials in registration");
    }
}



