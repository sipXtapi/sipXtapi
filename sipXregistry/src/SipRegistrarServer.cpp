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

// SYSTEM INCLUDES
#include <stdlib.h>


// APPLICATION INCLUDES
#include "os/OsDateTime.h"
#include "os/OsQueuedEvent.h"
#include "os/OsTimer.h"
#include "os/OsEventMsg.h"
#include "utl/UtlRegex.h"
#include "utl/PluginHooks.h"
#include "net/SipUserAgent.h"
#include "net/NetMd5Codec.h"
#include "net/NameValueTokenizer.h"
#include "sipdb/RegistrationDB.h"
#include "sipdb/ResultSet.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/CredentialDB.h"
#include "sipdb/RegistrationDB.h"
#include "SipRegistrarServer.h"
#include "registry/RegisterHook.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
#define MIN_EXPIRES_TIME 300

// FORWARD DECLARATIONS
// GLOBAL VARIABLES

static UtlString gUriKey("uri");
static UtlString gCallidKey("callid");
static UtlString gContactKey("contact");
static UtlString gExpiresKey("expires");
static UtlString gCseqKey("cseq");
static UtlString gQvalueKey("qvalue");

SipRegistrarServer::SipRegistrarServer() :
    OsServerTask("SipRegistrarServer", NULL, 2000),
    mIsStarted(FALSE),
    mSipUserAgent(NULL),
    mDefaultRegistryPeriod(3600),
    mNonceExpiration(5*60)
{
    // Set up a periodic timer for nonce garbage collection
    OsMsgQ* queue = getMessageQueue();
    OsQueuedEvent* queuedEvent = new OsQueuedEvent(*queue, 0);
    OsTimer* timer = new OsTimer(*queuedEvent);
    // Once a minute
    OsTime lapseTime(60,0);
    timer->periodicEvery(lapseTime, lapseTime);
}

UtlBoolean
SipRegistrarServer::initialize(
    SipUserAgent* SipUserAgent,
    SipImpliedSubscriptions* sipImpliedSubscriptions,
    PluginHooks* sipRegisterHooks,
    int defaultRegistryPeriod,
    const UtlString& minExpiresTime,
    const UtlString& defaultDomain,
    const UtlString& domainAliases,
    const UtlBoolean& useCredentialDB,
    const UtlString& realm)
{
    if ( !mIsStarted )
    {
        //start the thread
        start();
    }

    if ( !minExpiresTime.isNull() )
    {
        mMinExpiresTimeStr.append(minExpiresTime);
        mMinExpiresTimeint = atoi(minExpiresTime.data());

        if ( mMinExpiresTimeint < 60 )
        {
            mMinExpiresTimeint = 60;
            mMinExpiresTimeStr = "60";
        }
    }

    if ( defaultRegistryPeriod > mMinExpiresTimeint )
    {
        mDefaultRegistryPeriod = defaultRegistryPeriod;
    }

    if ( !defaultDomain.isNull() )
    {
        mDefaultDomain.remove(0);
        mDefaultDomain.append(defaultDomain);
        Url defaultDomainUrl(mDefaultDomain);
        mDefaultDomainPort = defaultDomainUrl.getHostPort();
        defaultDomainUrl.getHostAddress(mDefaultDomainHost);

        addValidDomain(mDefaultDomainHost, mDefaultDomainPort);
    }

    UtlString aliasString;
    int aliasIndex = 0;
    while(NameValueTokenizer::getSubField(domainAliases.data(), aliasIndex,
                                          ", \t", &aliasString))
    {
       Url aliasUrl(aliasString);
       UtlString hostAlias;
       aliasUrl.getHostAddress(hostAlias);
       int port = aliasUrl.getHostPort();

       addValidDomain(hostAlias,port);
       aliasIndex++;
    }

    if ( !realm.isNull() )
    {
        mRealm.remove(0);
        mRealm.append(realm);
    }

    mIsCredentialDB = useCredentialDB;
    mImpliedSubscriptions = sipImpliedSubscriptions;
    mpSipRegisterHooks = sipRegisterHooks;
    
    if ( SipUserAgent )
    {
        mSipUserAgent = SipUserAgent;
    }

    if ( !mSipUserAgent )
    {
        return FALSE;
    }
    return TRUE;
}

/// Apply valid changes to the database
///
/// Checks the message against the database, and if it is allowed by
/// those checks, applies the requested changes.
SipRegistrarServer::RegisterStatus
SipRegistrarServer::applyRegisterToDirectory( const int timeNow
                                             ,const SipMessage& registerMessage
                                             )
{
    RegisterStatus returnStatus = REGISTER_SUCCESS;
    UtlBoolean removeAll = FALSE;
    UtlBoolean isExpiresheader = FALSE;
    int longestExpiration = -1; // for duration passed to hooks
    int commonExpires = -1;

    // get the expires header from the register message
    // this may be overridden by the expires parameter on each contact
    if ( registerMessage.getExpiresField( &commonExpires ) )
    {
        isExpiresheader = TRUE; // only for use in testing '*'
    }
    else
    {
        commonExpires = mDefaultRegistryPeriod;
    }

    // get the header 'to' field from the register
    // message and construct a URL with it
    // this is also called the Address of record
    UtlString registerToStr;
    registerMessage.getToUri( &registerToStr );
    Url toUrl( registerToStr );

    // get the header 'callid' from the register message
    UtlString registerCallidStr;
    registerMessage.getCallIdField( &registerCallidStr );

    // get the cseq and the method (should be REGISTER)
    // from the register message
    UtlString method;
    int registerCseqInt = 0;
    registerMessage.getCSeqField( &registerCseqInt, &method );

    RegistrationDB* imdb = RegistrationDB::getInstance();

    // Check that this call-id and cseq are newer than what we have in the db
    if (! imdb->isOutOfSequence( toUrl, registerCallidStr, registerCseqInt))
    {
        // ****************************************************************
        // We now make two passes over all the contacts - the first pass
        // checks each contact for validity, and if valid, stores it in the
        // ResultSet registrations.
        // We act on the stored registrations only if all checks pass, in
        // a second iteration over the ResultSet below.
        // ****************************************************************

        ResultSet registrations; // built up during validation, acted on if all is well

        int contactIndexCount;
        UtlString registerContactStr;
        for ( contactIndexCount = 0;
              (   REGISTER_SUCCESS == returnStatus
               && registerMessage.getContactEntry ( contactIndexCount
                                                   ,&registerContactStr
                                                   )
               );
              contactIndexCount++
             )
        {
            if ( registerContactStr.compareTo("*") != 0 ) // is contact == "*" ?
            {
                // contact != "*"; a normal contact
                int expires;
                UtlString qStr, expireStr;
                Url registerContactURL( registerContactStr );

                // Check for an (optional) Expires field parameter in the Contact
                registerContactURL.getFieldParameter( SIP_EXPIRES_FIELD, expireStr );
                if ( expireStr.isNull() )
                {
                    // no expires parameter on the contact
                    // use the default established above
                    expires = commonExpires;
                }
                else
                {
                    // contact has its own expires parameter, which takes precedence
                    char  expireVal[12]; // more digits than we need...
                    char* end;
                    strncpy( expireVal, expireStr.data(), 10 );
                    expireVal[11] = '\0';// ensure it is null terminated.
                    expires = strtol(expireVal,&end,/*base*/10);
                    if ( '\0' != *end )
                    {
                        // expires value not a valid base 10 number
                        returnStatus = REGISTER_INVALID_REQUEST;
                        OsSysLog::add( FAC_SIP, PRI_WARNING,
                                      "SipRegistrarServer::applyRegisterToDirectory"
                                      " invalid expires parameter value '%s'\n",
                                      expireStr.data()
                                      );
                    }
                }

                if ( REGISTER_SUCCESS == returnStatus )
                {
                    // Ensure that the expires value is within allowed limits
                    if ( 0 == expires )
                    {
                        // unbind this mapping; ok
                    }
                    else if ( expires < mMinExpiresTimeint ) // lower bound
                    {
                        returnStatus = REGISTER_LESS_THAN_MINEXPIRES;
                    }
                    else if ( expires > mDefaultRegistryPeriod ) // upper bound
                    {
                        // our default is also the maximum we'll allow
                        expires = mDefaultRegistryPeriod;
                    }

                    if ( REGISTER_SUCCESS == returnStatus )
                    {
                        // get the qValue from the register message
                        UtlString registerQvalueStr;
                        registerContactURL.getFieldParameter( SIP_Q_FIELD, registerQvalueStr );

                        // remove the parameter fields - they are not part of the contact itself
                        registerContactURL.removeFieldParameters();
                        UtlString contactWithoutExpires = registerContactURL.toString();

                        // Build the row for the validated contacts hash
                        UtlHashMap registrationRow;

                        // value strings
                        UtlString* contactValue =
                            new UtlString ( contactWithoutExpires );
                        UtlInt* expiresValue =
                            new UtlInt ( expires );
                        UtlString* qvalueValue =
                            new UtlString ( registerQvalueStr );

                        // key strings - make shallow copies of static keys
                        UtlString* contactKey = new UtlString( gContactKey );
                        UtlString* expiresKey = new UtlString( gExpiresKey );
                        UtlString* qvalueKey = new UtlString( gQvalueKey );

                        registrationRow.insertKeyAndValue( contactKey, contactValue );
                        registrationRow.insertKeyAndValue( expiresKey, expiresValue );
                        registrationRow.insertKeyAndValue( qvalueKey, qvalueValue );

                        registrations.addValue( registrationRow );
                    }
                }
            }
            else
            {
                // Asterisk '*' requests that we unregister all contacts for the AOR
                removeAll = TRUE;
            }
        } // iteration over Contact entries

        // Now that we've gone over all the Contacts
        // act on them only if all was kosher
        if ( REGISTER_SUCCESS == returnStatus )
        {
            if ( 0 == contactIndexCount )
            {   // no contact entries were found - this is just a query
                returnStatus = REGISTER_QUERY;
            }
            else
            {
                if ( removeAll )
                {
                    // Contact: * special case
                    //  - request to remove all bindings for toUrl
                    if (   isExpiresheader
                        && 0 == commonExpires
                        && 1 == contactIndexCount
                        )
                    {
                        // Expires: 0 && Contact: * - clear all contacts
                        imdb->expireAllBindings( toUrl
                                                ,registerCallidStr, registerCseqInt
                                                ,timeNow
                                                );
                    }
                    else
                    {
                        // not allowed per rfc 3261
                        //  - must use Expires header of zero with Contact: *
                        //  - must not combine this with other contacts
                        returnStatus = REGISTER_INVALID_REQUEST;
                    }
                }
                else
                {
                    // Normal REGISTER request, no Contact: * entry
                    int numRegistrations = registrations.getSize();

                    // act on each valid contact
                    if ( numRegistrations > 0 )
                    {
                        for ( int i = 0; i<numRegistrations; i++ )
                        {
                            UtlHashMap record;
                            registrations.getIndex( i, record );

                            int      expires = ((UtlInt*)record.findValue(&gExpiresKey))->getValue();
                            UtlString contact(*((UtlString*)record.findValue(&gContactKey)));

                            int expirationTime;
                            if ( expires == 0 )
                            {
                                // Unbind this binding
                                //
                                // To cancel a contact, we expire it one second ago.
                                // This allows it to stay in the database until the
                                // explicit cleanAndPersist method cleans it out (which
                                // will be when it is more than one maximum registration
                                // time in the past).
                                // This prevents the problem of an expired registration
                                // being recreated by an old REGISTER request coming in
                                // whose cseq is lower than the one that unregistered it,
                                // which, if we had actually removed the entry would not
                                // be there to compare the out-of-order message to.
                                expirationTime = timeNow-1;

                                OsSysLog::add( FAC_SIP, PRI_DEBUG,
                                              "SipRegistrarServer::applyRegisterToDirectory - Expiring map %s->%s\n",
                                              registerToStr.data(), contact.data()
                                              );
                            }
                            else
                            {
                                // expires > 0, so add the registration
                                expirationTime = expires + timeNow;

                                OsSysLog::add( FAC_SIP, PRI_DEBUG,
                                       "SipRegistrarServer::applyRegisterToDirectory - Adding map %s->%s\n",
                                       registerToStr.data(), contact.data() );
                            }

                            UtlString qvalue(*(UtlString*)record.findValue(&gQvalueKey));

                            imdb->updateBinding( toUrl, contact, qvalue
                                                ,registerCallidStr, registerCseqInt
                                                ,expirationTime
                                                );

                            // track longest expirations for doing implied subscriptions
                            if ( expires > longestExpiration )
                            {
                                longestExpiration = expires;
                            }

                        } // iterate over good contact entries

                        // If there were any bindings not dealt with explicitly in this
                        // message that used the same callid, then expire them.
                        imdb->expireOldBindings( toUrl, registerCallidStr, registerCseqInt, timeNow );
                    }
                    else
                    {
                        OsSysLog::add( FAC_SIP, PRI_ERR
                                      ,"SipRegistrarServer::applyRegisterToDirectory contact count mismatch %d != %d"
                                      ,contactIndexCount, numRegistrations
                                      );
                        returnStatus = REGISTER_QUERY;
                    }
                }
                
                // Only persist the xml and do hooks if this was a good registration
                if ( REGISTER_SUCCESS == returnStatus )
                {
                    // something changed - garbage collect and persist the database
                    int oldestTimeToKeep = timeNow - mDefaultRegistryPeriod;
                    imdb->cleanAndPersist( oldestTimeToKeep );

                    // :TODO: move this into a hook
                    if ( longestExpiration > 0 ) // registration inserted or extended?
                    {
                        // This included at least one actual registration, so see if there are
                        // any subscriptions we need to request on behalf of this AOR
                        mImpliedSubscriptions->checkAndSend( registerMessage, longestExpiration
                                                            ,mNonceDb, mSipUserAgent
                                                            );
                    }
                
                    PluginIterator hooks(*mpSipRegisterHooks);
                    RegisterHook* hook;
                    while(hook = static_cast<RegisterHook*>(hooks.next()))
                    {
                       hook->takeAction(registerMessage, longestExpiration, mSipUserAgent );
                    }
                }
            }
        }
    }
    else
    {
       OsSysLog::add( FAC_SIP, PRI_WARNING,
                      "SipRegistrarServer::applyRegisterToDirectory request out of order\n"
                      "  To: '%s'\n"
                      "  Call-Id: '%s'\n"
                      "  Cseq: %d",
                      toUrl.toString().data(), registerCallidStr.data(), registerCseqInt
                     );

        returnStatus = REGISTER_OUT_OF_ORDER;
    }

    return returnStatus;
}

//functions
UtlBoolean
SipRegistrarServer::handleMessage( OsMsg& eventMessage )
{
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();

    // Timer event
    if(msgType == OsMsg::OS_EVENT &&
                msgSubType == OsEventMsg::NOTIFY)
    {
        // Garbage collect nonces
        //osPrintf("Garbage collecting nonces\n");

        OsTime time;
        OsDateTime::getCurTimeSinceBoot(time);
        long now = time.seconds();
        // Remove nonces more than 5 minutes old
        long oldTime = now - mNonceExpiration;
        mNonceDb.removeOldNonces(oldTime);
    }

    // SIP message event
    else if (msgType == OsMsg::PHONE_APP)
    {
        // osPrintf("SipRegistrarServer::handleMessage() - Start processing REGISTER Message\n");
        OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRegistrarServer::handleMessage() - "
                "Start processing REGISTER Message\n" );

        const SipMessage& message = *((SipMessageEvent&)eventMessage).getMessage();
        UtlString userKey, uri;
        SipMessage finalResponse;

        if ( isValidDomain( message, finalResponse ) )
        {
            // check in credential database if authentication needed
            if ( isAuthorized( message, finalResponse ) )
            {
                int port;
                int tagNum = 0;
                UtlString address, protocol, tag;
                message.getToAddress( &address, &port, &protocol, NULL, NULL, &tag );
                if ( tag.isNull() )
                {
                    tagNum = rand();     //generate to tag for response;
                }

                // process REQUIRE Header Field
                // add new contact values - update or insert
                int timeNow = OsDateTime::getSecsSinceEpoch();

                RegisterStatus applyStatus = applyRegisterToDirectory( timeNow, message );
                switch (applyStatus)
                {
                    case REGISTER_SUCCESS:
                    case REGISTER_QUERY:
                    {
                        OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRegistrarServer::handleMessage() - "
                               "contact successfully added\n");

                        //create response - 200 ok reseponse
                        finalResponse.setOkResponseData(&message);

                        UtlString toField;
                        message.getToUri(&toField);
                        Url toFieldUri(toField);

                        //get all current contacts now for the response
                        ResultSet registrations;

                        RegistrationDB::getInstance()->
                            getUnexpiredContacts(
                                toFieldUri, timeNow, registrations );

                        int numRegistrations = registrations.getSize();
                        for ( int i = 0 ; i<numRegistrations; i++ )
                        {
                            UtlHashMap record;
                            registrations.getIndex( i, record );
                            UtlString contactKey("contact");
                            UtlString expiresKey("expires");
                            UtlString qvalueKey("qvalue");
                            UtlString contact= *((UtlString*)record.findValue(&contactKey));
                            UtlString qvalue = *((UtlString*)record.findValue(&qvalueKey));
                            int expires     = ((UtlInt*)record.findValue(&expiresKey))->getValue();
                            expires = expires - timeNow;

                            Url contactUri( contact );

                            char buffexpires[32];
                            sprintf(buffexpires, "%d", expires);

                            contactUri.setFieldParameter(SIP_EXPIRES_FIELD, buffexpires);
                            if ( !qvalue.isNull() && qvalue.compareTo(SPECIAL_IMDB_NULL_VALUE)!=0 )
                            {
                               // :TODO: (XPL-3) need a RegEx copy constructor here
                               //check if q value is numeric and between the range 0.0 and 1.0
                               static RegEx qValueValid("^(0(\\.\\d{0,3})?|1(\\.0{0,3})?)$"); 
                               if (qValueValid.Search(qvalue.data()))
                               {
                                  contactUri.setFieldParameter(SIP_Q_FIELD, qvalue);
                               }
                            }
                            finalResponse.setContactField(contactUri.toString(),i);
                        }
                    }
                    break;

                    case REGISTER_OUT_OF_ORDER:
                        finalResponse.setResponseData(&message,SIP_5XX_CLASS_CODE,"Out Of Order");
                        break;

                    case REGISTER_LESS_THAN_MINEXPIRES:
                        //send 423 Registration Too Brief response
                        //must contain Min-Expires header field
                        finalResponse.setResponseData(&message,SIP_TOO_BRIEF_CODE,SIP_TOO_BRIEF_TEXT);
                        finalResponse.setHeaderValue(SIP_MIN_EXPIRES_FIELD, mMinExpiresTimeStr, 0);
                        break;

                    case REGISTER_INVALID_REQUEST:
                        finalResponse.setResponseData(&message,SIP_BAD_REQUEST_CODE, SIP_BAD_REQUEST_TEXT);
                        break;

                    case REGISTER_FORBIDDEN:
                        finalResponse.setResponseData(&message,SIP_FORBIDDEN_CODE, SIP_FORBIDDEN_TEXT);
                        break;

                    case REGISTER_NOT_FOUND:
                        finalResponse.setResponseData(&message,SIP_NOT_FOUND_CODE, SIP_NOT_FOUND_TEXT);
                        break;

                    default:
                       OsSysLog::add( FAC_SIP, PRI_ERR, 
                                     "Invalid result %d from applyRegisterToDirectory",
                                     applyStatus
                                     );
                        finalResponse.setResponseData(&message,SIP_SERVER_INTERNAL_ERROR_CODE, SIP_SERVER_INTERNAL_ERROR_TEXT);
                        break;
                }

                if ( tag.isNull() )
                {
                    finalResponse.setToFieldTag(tagNum);
                }

                UtlString finalMessageStr;
                int finalMessageLen;
                finalResponse.getBytes(&finalMessageStr, &finalMessageLen);
                OsSysLog::add( FAC_SIP, PRI_DEBUG, "\n----------------------------------\n"
                        "Sending final response\n%s\n", finalMessageStr.data());
                mSipUserAgent->send(finalResponse);

            } else
            {   //authentication error
                mSipUserAgent->send(finalResponse);
            }
        } else   // Invalid domain for registration
        {
            UtlString requestUri;
            message.getRequestUri(&requestUri);
            Url reqUri(requestUri);

            Url routeUrlParser(requestUri, TRUE);
            UtlString dummyValue;
            UtlBoolean previousHopStrictRoutes = routeUrlParser.getUrlParameter("lr", dummyValue, 0);
            //act as a proxy server and forward the request to the addresses domain
            UtlString lastViaAddress;
            int lastViaPort;
            UtlString protocol;
            int receivedPort;
            UtlBoolean receivedSet;
            UtlBoolean maddrSet;
            UtlBoolean receivedPortSet;
            message.getLastVia(
                &lastViaAddress,
                &lastViaPort,
                &protocol,
                &receivedPort,
                &receivedSet,
                &maddrSet,
                &receivedPortSet);

            UtlString localContactUriString;
            mSipUserAgent->getContactUri(&localContactUriString);
            Url localContactUrl(localContactUriString);
            UtlString contactAddress;
            int contactPort;
            localContactUrl.getHostAddress(contactAddress);
            contactPort = localContactUrl.getHostPort();

            // Simple loop detection
            if ( !receivedSet &&
                 lastViaAddress.compareTo(contactAddress, UtlString::ignoreCase) == 0 &&
                 lastViaPort == contactPort )
            {
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "Loop Detected while forwarding the request\n");
                //osPrintf("Loop Detected while forwarding the request\n");
                finalResponse.setResponseData(
                    &message, SIP_LOOP_DETECTED_CODE, SIP_LOOP_DETECTED_TEXT);
                mSipUserAgent->send(finalResponse);
            } else
            {
                if ( !previousHopStrictRoutes )
                {
                    OsSysLog::add(FAC_SIP, PRI_DEBUG, "Forwarded invalid domain request to correct domain\n");
                    //osPrintf("Forwarded invalid domain request to correct domain\n");
                    // Forward the register onward to the destination proxy
                    // When the SIP Registrar receives a REGISTER request for a domain
                    // not supported by the Registrar, it should remove the louse route that the
                    // forking proxy added.
                    // THis is a loose router pop my route off
                    UtlString routeUri;
                    SipMessage forwardedMessage(message);
                    forwardedMessage.removeRouteUri(0, &routeUri);
                    // Decrement max forwards
                    int maxForwards;
                    if ( forwardedMessage.getMaxForwards(maxForwards) )
                    {
                        maxForwards--;
                    } else
                    {
                        maxForwards = mSipUserAgent->getMaxForwards();
                    }
                    forwardedMessage.setMaxForwards(maxForwards);
                    mSipUserAgent->send(forwardedMessage);
                }
            }
        }
    }
    return TRUE;
}


UtlBoolean
SipRegistrarServer::isAuthorized (
    const SipMessage& message,
    SipMessage& responseMessage )
{
    UtlString fromUri;
    UtlString toUri;
    UtlBoolean isAuthorized = FALSE;

    message.getFromUri(&fromUri);
    Url fromUrl(fromUri);
    message.getToUri(&toUri) ;
    Url toUrl(toUri) ; ;

    if ( !mIsCredentialDB )
    {
        OsSysLog::add( FAC_AUTH, PRI_DEBUG, "SipRegistrar::isAuthorized() "
                ":: No Credential DB - request is always AUTHORIZED\n" );
        isAuthorized = TRUE;
    }
    else
    {
        // realm and auth type should be default for server!!!!!!!!!!
        // if URI not defined in DB, the user is not authorized to modify bindings - NOT DOING ANYMORE
        // check if we requested authentication and this is the req with
        // authorization,validate the authorization
        OsSysLog::add( FAC_AUTH, PRI_DEBUG, "SipRegistrar::isAuthorized()"
                ": fromUri='%s', toUri='%s', realm='%s' \n", fromUri.data(), toUri.data(), mRealm.data() );

        UtlString requestNonce, requestRealm, requestUser, uriParam;
        int requestAuthIndex = 0;
        UtlString callId;
        UtlString fromTag;

        message.getCallIdField(&callId);
        fromUrl.getFieldParameter("tag", fromTag);

        while ( ! isAuthorized
               && message.getDigestAuthorizationData(
                   &requestUser, &requestRealm, &requestNonce,
                   NULL, NULL, &uriParam,
                   HttpMessage::SERVER, requestAuthIndex)
               )
        {
           OsSysLog::add( FAC_AUTH, PRI_DEBUG, "Message Authorization received: "
                    "reqRealm='%s', reqUser='%s'\n", requestRealm.data() , requestUser.data());

            if ( mRealm.compareTo(requestRealm) == 0 ) // case sensitive check that realm is correct
            {
                OsSysLog::add(FAC_AUTH, PRI_DEBUG, "SipRegistrar::isAuthorized() Realm Matches\n");

                // need the request URI to validate the nonce
                UtlString reqUri;
                message.getRequestUri(&reqUri);
                UtlString authTypeDB;
                UtlString passTokenDB;

                // validate the nonce
                if (mNonceDb.isNonceValid(requestNonce, callId, fromTag,
                                          reqUri, mRealm, mNonceExpiration))
                {
                    Url discardUriFromDB;

                    // then get the credentials for this user & realm
                    if (CredentialDB::getInstance()->getCredential( toUrl
                                                                   ,requestRealm
                                                                   ,requestUser
                                                                   ,passTokenDB
                                                                   ,authTypeDB
                                                                   ))
                    {
                      // only DIGEST is used, so the authTypeDB above is ignored
                      if ((isAuthorized = message.verifyMd5Authorization(requestUser.data(),
                                                                         passTokenDB.data(),
                                                                         requestNonce,
                                                                         requestRealm.data(),
                                                                         uriParam)
                           ))
                        {
                          OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                                        "SipRegistrar::isAuthorized() "
                                        "response auth hash matches\n");
                        }
                      else
                        {
                          OsSysLog::add(FAC_AUTH, PRI_ERR,
                                        "Response auth hash does not match (bad password?)"
                                        "\n toUrl='%s' requestUser='%s'",
                                        toUrl.toString().data(), requestUser.data());
                        }
                    }
                    else // failed to get credentials
                    {
                        OsSysLog::add(FAC_AUTH, PRI_ERR,
                                      "Unable to get credentials for '%s'\nrealm='%s'\nuser='%s'",
                                      toUri.data(), mRealm.data(), requestUser.data());
                    }
                }
                else // nonce is not valid
                {
                    OsSysLog::add(FAC_AUTH, PRI_ERR,
                                  "Invalid nonce for '%s'\nnonce='%s'\ncallId='%s'\nreqUri='%s'",
                                  toUri.data(), requestNonce.data(), callId.data(), reqUri.data());
                }
            }
            requestAuthIndex++;
        } //end while

        if ( !isAuthorized )
        {
           // Generate a new challenge
            UtlString newNonce;
            UtlString challangeRequestUri;
            message.getRequestUri(&challangeRequestUri);
            UtlString opaque;

            mNonceDb.createNewNonce(callId,
                                    fromTag,
                                    challangeRequestUri,
                                    mRealm,
                                    newNonce);

            responseMessage.setRequestUnauthorized ( &message, HTTP_DIGEST_AUTHENTICATION, mRealm,
                                                    newNonce, NULL // opaque
                                                    );
            
        }
    }
    return isAuthorized;
}

void
SipRegistrarServer::addValidDomain(const UtlString& host, int port)
{
   UtlString* valid = new UtlString(host);
   valid->toLower();

   char explicitPort[20];
   sprintf(explicitPort,":%d", PORT_NONE==port ? SIP_PORT : port );
   valid->append(explicitPort);
   
   OsSysLog::add(FAC_AUTH, PRI_DEBUG, "SipRegistrarServer::addValidDomain(%s)\n",valid->data()) ;

   mValidDomains.insert(valid);
}

UtlBoolean
SipRegistrarServer::isValidDomain(
    const SipMessage& message,
    SipMessage& responseMessage )
{
    // Fetch the domain and port from the request URI
    UtlString lookupDomain, requestUri;
    message.getRequestUri( &requestUri );
    Url reqUri(requestUri);

    reqUri.getHostAddress(lookupDomain);
    lookupDomain.toLower();

    int port = reqUri.getHostPort();
    if (port == PORT_NONE)
    {
       port = SIP_PORT;
    }
    char portNum[15];
    sprintf(portNum,"%d",port);

    lookupDomain.append(":");
    lookupDomain.append(portNum);

    if ( mValidDomains.contains(&lookupDomain) )
    {
        OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                      "SipRegistrarServer::isValidDomain(%s) VALID\n",
                      lookupDomain.data()) ;
        return TRUE;
    }
    OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                  "SipRegistrarServer::isValidDomain(%s) INVALID\n",
                  lookupDomain.data()) ;
    return FALSE;
}

SipRegistrarServer::~SipRegistrarServer()
{
   mValidDomains.destroyAll();
}

void RegisterHook::takeAction( const SipMessage&   registerMessage  
                              ,const unsigned int  registrationDuration 
                              ,SipUserAgent*       sipUserAgent
                              )
{
   assert(false);
   
   OsSysLog::add(FAC_SIP, PRI_ERR,
                 "RegisterHook::takeAction not resolved by configured hook"
                 );
}

