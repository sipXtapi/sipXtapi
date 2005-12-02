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
#include "registry/RegisterPlugin.h"

// DEFINES

/*
 * GRUUs are constructed by hashing the AOR, the IID, and the primary
 * SIP domain.  The SIP domain is included so that GRUUs constructed by
 * different systems will be different, but that any registrars that
 * form a redundant set will generate the same GRUU for an AOR/IID pair.
 * (Strictly, we use the entire value of the +sip.instance field parameter
 * on the Contact: header, which should be <...> around the IID, but there
 * is no point enforcing or parsing that rule.)
 */

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const RegEx RegQValue("^(0(\\.\\d{0,3})?|1(\\.0{0,3})?)$"); // syntax for a valid q parameter value

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
static UtlString gInstanceIdKey("instance_id");
static UtlString gGruuKey("gruu");

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
    PluginHooks* sipRegisterPlugins,
    int defaultRegistryPeriod,
    const UtlString& minExpiresTime,
    const UtlString& defaultDomain,
    const UtlString& domainAliases,
    int              proxyNormalPort,
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

    mProxyNormalPort = proxyNormalPort;
    
    mIsCredentialDB = useCredentialDB;
    mpSipRegisterPlugins = sipRegisterPlugins;
    
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
SipRegistrarServer::applyRegisterToDirectory( const Url& toUrl
                                             ,const int timeNow
                                             ,const SipMessage& registerMessage
                                             )
{
    RegisterStatus returnStatus = REGISTER_SUCCESS;
    UtlBoolean removeAll = FALSE;
    UtlBoolean isExpiresheader = FALSE;
    int longestExpiration = -1; // for duration passed to hooks
    int commonExpires = -1;
    UtlString registerToStr;
    toUrl.getIdentity(registerToStr);
    
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
           OsSysLog::add( FAC_SIP, PRI_WARNING,
                          "SipRegistrarServer::applyRegisterToDirectory - processing '%s'\n",
                          registerContactStr.data()
                               );
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
                        // Get the qValue from the register message.
                        UtlString registerQvalueStr;
                        registerContactURL.getFieldParameter( SIP_Q_FIELD, registerQvalueStr );
                        // Get the Instance ID (if any) from the REGISTER message Contact field.
                        UtlString instanceId;
                        registerContactURL.getFieldParameter( "+sip.instance", instanceId );
                        // See if the Contact field has a "gruu" URI field..
                        // :TODO: Have to check whether the gruu URI parameter
                        // is still favored.  And what exactly this check is about.
                        UtlBoolean gruuPresent;
                        UtlString gruuDummy;
                        gruuPresent = registerContactURL.getUrlParameter( "gruu", gruuDummy );
                        OsSysLog::add( FAC_SIP, PRI_DEBUG,
                                       "SipRegistrarServer::applyRegisterToDirectory instance ID = '%s'",
                                       instanceId.data());

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
                        UtlString* instanceIdValue = new UtlString ( instanceId );
                        UtlString* gruuValue;

                        // key strings - make shallow copies of static keys
                        UtlString* contactKey = new UtlString( gContactKey );
                        UtlString* expiresKey = new UtlString( gExpiresKey );
                        UtlString* qvalueKey = new UtlString( gQvalueKey );
                        UtlString* instanceIdKey = new UtlString( gInstanceIdKey );
                        UtlString* gruuKey = new UtlString( gGruuKey );

                        // Calculate GRUU if gruu is in Supported, +sip.instance is provided, but
                        // gruu is not a URI parameter.
                        if (!instanceId.isNull() &&
                            !gruuPresent &&
                            registerMessage.isInSupportedField("gruu"))
                        {
                           // Hash the GRUU base, the AOR, and IID to
                           // get the variable part of the GRUU.
                           NetMd5Codec encoder;
                           UtlString temp;
                           // Use the trick that the MD5 of a series of null-
                           // separated strings is effectively a unique function
                           // of all of the strings.
                           // Include "sipX" as the signature of this software.
                           temp.append("sipX", 5);
                           temp.append(mDefaultDomain);
                           temp.append("\0", 1);
                           temp.append(toUrl.toString());
                           temp.append("\0", 1);
                           temp.append(instanceId);
                           UtlString hash;
                           encoder.encode(temp, hash);
                           hash.remove(16);
                           // Now construct the GRUU URI,
                           // "gruu~XXXXXXXXXXXXXXXX@[principal SIP domain]".
                           // That is what we store in IMDB, so it can be
                           // searched for by the redirector, since it searches
                           // for the "identity" part of the URI, which does
                           // not contain the scheme.
                           gruuValue = new UtlString(GRUU_PREFIX);
                           gruuValue->append(hash);
                           gruuValue->append("@");
                           gruuValue->append(mDefaultDomain);
                           OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                         "SipRegistrarServer::applyRegisterToDirectory gruu = '%s'",
                                         gruuValue->data());
                        }
                        else
                        {
                           gruuValue = new UtlString( "" );
                        }

                        registrationRow.insertKeyAndValue( contactKey, contactValue );
                        registrationRow.insertKeyAndValue( expiresKey, expiresValue );
                        registrationRow.insertKeyAndValue( qvalueKey, qvalueValue );
                        registrationRow.insertKeyAndValue( instanceIdKey, instanceIdValue );
                        registrationRow.insertKeyAndValue( gruuKey, gruuValue );

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
                            UtlString instance_id(*(UtlString*)record.findValue(&gInstanceIdKey));
                            UtlString gruu(*(UtlString*)record.findValue(&gGruuKey));

                            imdb->updateBinding( toUrl, contact, qvalue
                                                ,registerCallidStr, registerCseqInt
                                                ,expirationTime
                                                ,instance_id
                                                ,gruu
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

                    // give each RegisterPlugin a chance to do its thing
                    PluginIterator plugins(*mpSipRegisterPlugins);
                    RegisterPlugin* plugin;
                    while(plugin = static_cast<RegisterPlugin*>(plugins.next()))
                    {
                       plugin->takeAction(registerMessage, longestExpiration, mSipUserAgent );
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
                      registerToStr.data(), registerCallidStr.data(), registerCseqInt
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
           // get the header 'to' field from the register
           // message and construct a URL with it
           // this is also called the Address of record
           UtlString registerToStr;
           message.getToUri( &registerToStr );
           Url toUrl( registerToStr );

           /*
            * Normalize the port in the Request URI
            *   This is not strictly kosher, but it solves interoperability problems.
            *   Technically, user@foo:5060 != user@foo , but many implementations
            *   insist on including the explicit port even when they should not, and
            *   it causes registration mismatches, so we normalize the URI when inserting
            *   and looking up in the database so that if explicit port is the same as
            *   the proxy listening port, then we remove it.
            *   (Since our proxy has mProxyNormalPort open, no other SIP entity
            *   can use sip:user@domain:mProxyNormalPort, so this normalization
            *   cannot interfere with valid addresses.)
            *
            * For the strict rules, set the configuraiton parameter
            *   SIP_REGISTRAR_PROXY_PORT : PORT_NONE
            */
           if (   mProxyNormalPort != PORT_NONE
               && toUrl.getHostPort() == mProxyNormalPort
               )
           {
              toUrl.setHostPort(PORT_NONE);
           }
           UtlString registeredIdentity;
           toUrl.getIdentity(registeredIdentity);
           
           // check in credential database if authentication needed
           if ( isAuthorized( toUrl, message, finalResponse ) )
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

                RegisterStatus applyStatus
                   = applyRegisterToDirectory( toUrl, timeNow, message );

                switch (applyStatus)
                {
                    case REGISTER_SUCCESS:
                    case REGISTER_QUERY:
                    {
                        OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRegistrarServer::handleMessage() - "
                               "contact successfully added\n");

                        //create response - 200 ok reseponse
                        finalResponse.setOkResponseData(&message);

                        //get all current contacts now for the response
                        ResultSet registrations;

                        RegistrationDB::getInstance()->
                            getUnexpiredContacts(
                                toUrl, timeNow, registrations );

                        int numRegistrations = registrations.getSize();
                        for ( int i = 0 ; i<numRegistrations; i++ )
                        {
                            UtlHashMap record;
                            registrations.getIndex( i, record );
                            UtlString contactKey("contact");
                            UtlString expiresKey("expires");
                            UtlString qvalueKey("qvalue");
                            UtlString instanceIdKey("instance_id");
                            UtlString gruuKey("gruu");
                            UtlString contact = *((UtlString*)record.findValue(&contactKey));
                            UtlString qvalue = *((UtlString*)record.findValue(&qvalueKey));
                            int expires = ((UtlInt*)record.findValue(&expiresKey))->getValue();
                            expires = expires - timeNow;

                            OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRegistrarServer::handleMessage - "
                               "processing contact '%s'", contact.data());
                            Url contactUri( contact );

                            char buffexpires[32];
                            sprintf(buffexpires, "%d", expires);

                            contactUri.setFieldParameter(SIP_EXPIRES_FIELD, buffexpires);
                            if ( !qvalue.isNull() && qvalue.compareTo(SPECIAL_IMDB_NULL_VALUE)!=0 )
                            {
                               OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRegistrarServer::handleMessage - "
                               "adding q '%s'", qvalue.data());

                               //check if q value is numeric and between the range 0.0 and 1.0
                               RegEx qValueValid(RegQValue); 
                               if (qValueValid.Search(qvalue.data()))
                               {
                                  contactUri.setFieldParameter(SIP_Q_FIELD, qvalue);
                               }
                            }

                            // Add the +sip.instance and gruu
                            // parameters if an instance ID is recorded.
                            UtlString* instance_id = dynamic_cast<UtlString*> (record.findValue(&instanceIdKey));
                            OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRegistrarServer::handleMessage - value %p, instance_id %p, instanceIdKey = '%s'", 
                                           record.findValue(&instanceIdKey),
                                           instance_id, instanceIdKey.data());
                            if (instance_id && !instance_id->isNull() &&
                                instance_id->compareTo(SPECIAL_IMDB_NULL_VALUE) !=0 )
                            {
                               OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRegistrarServer::handleMessage - add instance '%s'", instance_id->data());
                               contactUri.setFieldParameter("+sip.instance",
                                                            *instance_id);
                               // Prepend "sip:" to the GRUU, since it is stored
                               // in the database in identity form.
                               UtlString temp("sip:");
                               temp.append(
                                  *(dynamic_cast<UtlString*>
                                    (record.findValue(&gruuKey))));
                               contactUri.setFieldParameter("gruu", temp);
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
            }
           else
            {
               // authentication error - response data was set in isAuthorized
            }
        }
        else   
        {
           // Invalid domain for registration - response data was set in isValidDomain
        }

        if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
        {
           UtlString finalMessageStr;
           int finalMessageLen;
           finalResponse.getBytes(&finalMessageStr, &finalMessageLen);
           OsSysLog::add( FAC_SIP, PRI_DEBUG, "\n----------------------------------\n"
                         "Sending final response\n%s\n", finalMessageStr.data());
        }
        
        mSipUserAgent->send(finalResponse);
    }
    return TRUE;
}


UtlBoolean
SipRegistrarServer::isAuthorized (
    const Url&  toUrl,
    const SipMessage& message,
    SipMessage& responseMessage )
{
    UtlString fromUri;
    UtlBoolean isAuthorized = FALSE;

    message.getFromUri(&fromUri);
    Url fromUrl(fromUri);

    UtlString identity;
    toUrl.getIdentity(identity);
    
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
                ": fromUri='%s', toUri='%s', realm='%s' \n", fromUri.data(), toUrl.toString().data(), mRealm.data() );

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
                                      identity.data(), mRealm.data(), requestUser.data());
                    }
                }
                else // nonce is not valid
                {
                    OsSysLog::add(FAC_AUTH, PRI_ERR,
                                  "Invalid nonce for '%s'\nnonce='%s'\ncallId='%s'\nreqUri='%s'",
                                  identity.data(), requestNonce.data(), callId.data(), reqUri.data());
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
    UtlBoolean isValid;
   
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
        isValid = TRUE;
    }
    else
    {
       UtlString requestedDomain;
       reqUri.getHostAddress(requestedDomain);

       OsSysLog::add(FAC_AUTH, PRI_WARNING,
                     "SipRegistrarServer::isValidDomain('%s' == '%s') Invalid\n",
                     requestedDomain.data(), lookupDomain.data()) ;

       UtlString responseText;
       responseText.append("Domain '");
       responseText.append(requestedDomain);
       responseText.append("' is not valid at this registrar");
       responseMessage.setResponseData(&message, SIP_NOT_FOUND_CODE, responseText.data() );
       isValid = FALSE;
    }

    return isValid;
}

SipRegistrarServer::~SipRegistrarServer()
{
   mValidDomains.destroyAll();
}

void RegisterPlugin::takeAction( const SipMessage&   registerMessage  
                                ,const unsigned int  registrationDuration 
                                ,SipUserAgent*       sipUserAgent
                                )
{
   assert(false);
   
   OsSysLog::add(FAC_SIP, PRI_ERR,
                 "RegisterPlugin::takeAction not resolved by configured hook"
                 );
}
