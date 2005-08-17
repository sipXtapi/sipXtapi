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
#include "net/SipMessageEvent.h"
#include "net/SipUserAgent.h"
#include "net/NetMd5Codec.h"
#include "sipdb/ResultSet.h"
#include "sipdb/CredentialDB.h"
#include "sipdb/PermissionDB.h"
#include "sipdb/SubscriptionDB.h"
#include "statusserver/Notifier.h"
#include "statusserver/PluginXmlParser.h"
#include "statusserver/SubscribeServerThread.h"
#include "statusserver/SubscribeServerPluginBase.h"
#include "statusserver/StatusPluginReference.h"
#include "utl/UtlSortedList.h"
#include "utl/UtlSortedListIterator.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
#define MIN_EXPIRES_TIME 300
#define DEFAULT_Q_VALUE " "

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SubscribeServerThread::SubscribeServerThread():
    OsServerTask("SubscribeServerThread", NULL, 2000),
    mpSipUserAgent(NULL),
    mIsStarted(FALSE),
    mDefaultSubscribePeriod( 24*60*60 ), //24 hours
    mPluginTable(NULL)
{}

SubscribeServerThread::~SubscribeServerThread()
{
    if( mpSipUserAgent )
        mpSipUserAgent = NULL;

    if( mPluginTable )
        mPluginTable = NULL;
}

///////////////////////////PUBLIC///////////////////////////////////
UtlBoolean
SubscribeServerThread::initialize (
    SipUserAgent* sipUserAgent,
    int defaultSubscribePeriod,
    const UtlString& minExpiresTime,
    const UtlString& defaultDomain,
    const UtlBoolean& useCredentialDB,
    const UtlString& realm,
    PluginXmlParser* pluginTable)
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
    }

    mDefaultSubscribePeriod = defaultSubscribePeriod;
    if ( mMinExpiresTimeint > mDefaultSubscribePeriod )
    {
       OsSysLog::add(FAC_SIP, PRI_ERR,
                     "SubscribeServerThread::initialize - minimum expiration time (%d)"
                     " > default/maximum expiration time (%d); reset to equal",
                     mMinExpiresTimeint,
                     mDefaultSubscribePeriod
          );
       mMinExpiresTimeint = mDefaultSubscribePeriod;
    }

    if ( !defaultDomain.isNull() )
    {
        mDefaultDomain.remove(0);
        mDefaultDomain.append( defaultDomain );
        Url defaultDomainUrl( mDefaultDomain );
        mDefaultDomainPort = defaultDomainUrl.getHostPort();

        // the isValidDomain compares the server's domain to
        // that in the incoming URI, sometimes the incoming
        // URI is a dotted IP address.
        UtlString ipOrFQDNHost;

        defaultDomainUrl.getHostAddress(ipOrFQDNHost);

        // sometimes the request URI will contain
        // an IP Address and at others it contains a FQDN
        if ( OsSocket::isIp4Address( ipOrFQDNHost.data() ) )
        {
            mDefaultDomainHostIP = ipOrFQDNHost;
            OsSocket::getDomainName( mDefaultDomainHostFQDN );
        } else
        {
            mDefaultDomainHostFQDN = ipOrFQDNHost;
            OsSocket::getHostIp( &mDefaultDomainHostIP );
        }
    }

    if ( !realm.isNull() )
    {
        mRealm.remove(0);
        mRealm.append(realm);
    }

    mIsCredentialDB = useCredentialDB;

    //get sip user agent
    if ( sipUserAgent )
    {
        mpSipUserAgent = sipUserAgent;
    } else
    {
        mpSipUserAgent = NULL;
        return FALSE;
    }

    //get Plugin table
    if ( pluginTable )
    {
        mPluginTable = pluginTable;
    } else
    {
        return FALSE;
    }
    return TRUE;
}


//functions
UtlBoolean
SubscribeServerThread::handleMessage(OsMsg& eventMessage)
{
    syslog(FAC_SIP, PRI_DEBUG, "SubscribeServerThread::handleMessage() -"
        " Start processing SUBSCRIBE Message\n");

    const SipMessage* message =
        ((SipMessageEvent&)eventMessage).getMessage();

    UtlString userKey;
    UtlString uri;
    SipMessage finalResponse;

    // Test for request/response processing code path
    if (!message->isResponse())
    {
        // this is a request, so authenticate and authorize the request
        if ( isValidDomain( message, &finalResponse ) )
        {
            UtlString eventPackage;
            UtlString id;

            message->getEventField(&eventPackage, &id);
            
            StatusPluginReference* pluginContainer =
                mPluginTable->getPlugin( eventPackage );

            if( pluginContainer )
            {
               //check in credential database if authentication needed
               UtlString authenticatedUser, authenticatedRealm;
               if( isAuthenticated ( message, &finalResponse, authenticatedUser, authenticatedRealm ) )
               {
                  if ( isAuthorized ( message, &finalResponse, pluginContainer ) )
                  {
                     // fetch the plugin
                     SubscribeServerPluginBase* plugin =
                        pluginContainer->getPlugin();

                     if (plugin)
                     {
                        // Make sure the to tag is set
                        Url toUrl;
                        UtlString tag;
                        message->getToUrl(toUrl);
                        if (!toUrl.getFieldParameter("tag", tag))
                        {
                           int toTagNum = rand();
                           char tagString[32];
                           sprintf(tagString, "%d", toTagNum);
                           ((SipMessage*)message)->setToFieldTag( tagString );
                        }

                        int timeNow = OsDateTime::getSecsSinceEpoch();

                        // add the subscription to the IMDB
                        SubscribeStatus isSubscriptionAdded
                           = addSubscription(timeNow,
                                             message,
                                             mDefaultDomain,
                                             eventPackage,
                                             id,
                                             finalResponse);

                        switch ( isSubscriptionAdded )
                        {
                        case STATUS_SUCCESS:
                        case STATUS_QUERY:
                           // create response - 202 Accepted Response
                           finalResponse.setResponseData( message,
                                                          SIP_ACCEPTED_CODE,
                                                          SIP_ACCEPTED_TEXT);

                           plugin->handleSubscribeRequest( *message,
                                                           finalResponse,
                                                           authenticatedUser.data(),
                                                           authenticatedRealm.data(),
                                                           mDefaultDomain.data());

                           // ensure that the contact returned will route back to here
                           // (the default supplied by SipUserAgent will not).
                           {
                              UtlString requestUri;
                              message->getRequestUri(&requestUri);
                              finalResponse.setContactField(requestUri);
                           }
                           break;
                           
                        case STATUS_LESS_THAN_MINEXPIRES:
                           // (already logged in addSubscription)

                           // send 423 Subscription Too Brief response
                           finalResponse.setResponseData(
                              message,
                              SIP_TOO_BRIEF_CODE,
                              SIP_TOO_BRIEF_TEXT );

                           finalResponse.setHeaderValue(
                              SIP_MIN_EXPIRES_FIELD,
                              mMinExpiresTimeStr,
                              0 );
                           break;

                        case STATUS_INVALID_REQUEST:
                           OsSysLog::add(FAC_SIP, PRI_ERR, 
                                         "SubscribeServerThread::handleMessage()"
                                         "Subscription Could Not Be Added "
                                         SIP_BAD_REQUEST_TEXT
                              );

                           finalResponse.setResponseData(
                              message,
                              SIP_BAD_REQUEST_CODE,
                              SIP_BAD_REQUEST_TEXT );
                           break;

                        case STATUS_FORBIDDEN:
                           OsSysLog::add(FAC_SIP, PRI_ERR, 
                                         "SubscribeServerThread::handleMessage()"
                                         "Subscription Could Not Be Added "
                                         SIP_FORBIDDEN_TEXT
                              );

                           finalResponse.setResponseData(
                              message,
                              SIP_FORBIDDEN_CODE,
                              SIP_FORBIDDEN_TEXT);
                           break;

                        case STATUS_NOT_FOUND:
                           OsSysLog::add(FAC_SIP, PRI_ERR, 
                                         "SubscribeServerThread::handleMessage()"
                                         "Subscription Could Not Be Added "
                                         SIP_NOT_FOUND_TEXT
                              );
                           finalResponse.setResponseData(
                              message,
                              SIP_NOT_FOUND_CODE,
                              SIP_NOT_FOUND_TEXT );
                           break;

                        default:
                           OsSysLog::add(FAC_SIP, PRI_ERR, 
                                         "SubscribeServerThread::handleMessage()"
                                         "Subscription Could Not Be Added "
                                         "Unknown status %d from addSubscription",
                                         isSubscriptionAdded
                              );
                           finalResponse.setResponseData(
                              message,
                              SIP_SERVER_INTERNAL_ERROR_CODE,
                              SIP_SERVER_INTERNAL_ERROR_TEXT );
                        }
                     }
                     else
                     {
                        OsSysLog::add(FAC_SIP, PRI_CRIT, 
                                      "SubscribeServerThread::handleMessage()"
                                      " container->getPlugin failed for '%s'",
                                      eventPackage.data()
                           );
                        finalResponse.setResponseData(
                           message,
                           SIP_SERVER_INTERNAL_ERROR_CODE,
                           SIP_SERVER_INTERNAL_ERROR_TEXT );
                     }
                  }
                  else
                  {
                     // not authorized - the response was created in isAuthorized
                  }
               }
               else
               {
                  // not authenticated - the response was created in isAuthenticated
               }
            }
            else // no plugin found for this event type
            {
               OsSysLog::add(FAC_SIP, PRI_WARNING, 
                             "SubscribeServerThread::handleMessage()"
                             " Request denied - "
                             SIP_BAD_EVENT_TEXT
                  );
               finalResponse.setResponseData( message,
                                              SIP_BAD_EVENT_CODE,
                                              SIP_BAD_EVENT_TEXT );
            }

            // send final response
            UtlString finalMessageStr;
            int finalMessageLen;
            finalResponse.getBytes(&finalMessageStr, &finalMessageLen);
            syslog(FAC_SIP, PRI_DEBUG, "\n----------------------------------\n"
                "Sending final response\n%s\n",finalMessageStr.data());
            mpSipUserAgent->send( finalResponse );
        } 
        else // Invalid domain
        {
           const char* notFoundMsg = SIP_NOT_FOUND_TEXT " Invalid Domain";
           finalResponse.setResponseData(message,
                                         SIP_NOT_FOUND_CODE,
                                         notFoundMsg
                                         );
           mpSipUserAgent->send( finalResponse );
        }
    } 
    else // response
    {
       // The server may send us back a "481" response, if it does we need
       // to remove the subscription from the SubscriptionDB as the callid
       // that it corresponds to is stale (probably the phone was rebooted)
       // In the above case, RFC 3265 says we MUST remove the subscription.
       // It also says (essentially) that any error that does not imply a retry
       // SHOULD remove the subscription.  We will interpret this to be any
       // 4xx code _except_ 408 timeout (because that may be a transient error).
       int responseCode = message->getResponseStatusCode();
       if (   responseCode >= SIP_4XX_CLASS_CODE
           && responseCode != SIP_REQUEST_TIMEOUT_CODE )
       {
          SipMessage* pRequest = mpSipUserAgent->getRequest(*message);
          if ( pRequest != NULL )
          {
             UtlString eventType;
             UtlString eventId;
             message->getEventField( &eventType, &eventId );

             StatusPluginReference* pluginContainer =
                mPluginTable->getPlugin( eventType );

             if( pluginContainer != NULL )
             {
                // fetch the plugin
                SubscribeServerPluginBase* plugin =
                   pluginContainer->getPlugin();

                syslog(FAC_SIP, PRI_DEBUG, "SubscribeServerThread::handleMessage() -"
                       " error (%d) response from UA - removing subscription", responseCode );

                // we should call the plugin to handle the removal
                plugin->handleNotifyResponse( *message, finalResponse );

                // remove the subscription
                removeSubscription ( eventType, eventId, *message );
             }
             delete pRequest;
          }
       }
    }
    return TRUE;
}

UtlBoolean
SubscribeServerThread::isAuthorized (
    const SipMessage* message,
    SipMessage *responseMessage,
    StatusPluginReference* pluginContainer)
{
    UtlBoolean isAuthorized = FALSE;
    UtlString  requestUser;
    Url       identityUrl;
    message->getUri(NULL, NULL, NULL, &requestUser);
    identityUrl.setUserId(requestUser);
    identityUrl.setHostAddress(mDefaultDomain);

    if( pluginContainer )
    {
        // if the plugin has permissions, we must match all these against the IMDB
        if( pluginContainer->hasPermissions() )
        {
            // permission required. Check for required permission in permission IMDB
            // All required permissions should match
            ResultSet dbPermissions;

            PermissionDB::getInstance()->getPermissions( identityUrl, dbPermissions );

            int numDBPermissions = dbPermissions.getSize();

            if( numDBPermissions > 0 )
            {
                UtlBoolean nextPermissionMatched = TRUE;

                UtlSListIterator* pluginPermissionIterator = pluginContainer->permissionsIterator();
                UtlString* pluginPermission;
                // Iterated through the plugin permissions matching
                // them one by one against the IMDB
                while(   (pluginPermission = (UtlString*)(*pluginPermissionIterator)())
                      && nextPermissionMatched
                      )
                {
                    //check againt all permissions in IMDB
                    nextPermissionMatched = FALSE;
                    UtlString identity, permission;
                    for ( int dbIndex = 0; dbIndex < numDBPermissions; dbIndex++ )
                    {

                        UtlHashMap record;
                        dbPermissions.getIndex( dbIndex, record );
                        // note not interested in identity here
                        UtlString permissionKey ("permission");
                        UtlString permission = *((UtlString*)record.findValue(&permissionKey));
                        if( permission.compareTo( *pluginPermission, UtlString::ignoreCase ) == 0)
                        {
                            nextPermissionMatched = TRUE;
                            break;
                        }
                    }
                }
                delete pluginPermissionIterator;

                // after going thru all permissions find out if all matched or not
                if( nextPermissionMatched )
                {
                    syslog(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isAuthorized() -"
                        " All permissions matched - request is AUTHORIZED\n");
                    isAuthorized = TRUE;
                }
                else
                {
                    syslog(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isAuthorized() -"
                        " One or more Permissions did not match - request is UNAUTHORIZED\n");
                    isAuthorized = FALSE;
                }
            }
            else
            {
                // one or more permissions needed by plugin and none in IMDB => UNAUTHORIZED
                syslog(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isAuthorized() -"
                    " No Permissions in IMDB - request is UNAUTHORIZED\n");
                isAuthorized = FALSE;
            }
        }
        else
        {
            syslog(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isAuthorized() -"
                " No Permissions required - request is always AUTHORIZED\n");
            isAuthorized = TRUE;
        }
    }
    //set the error response message id unauthorized
    if(!isAuthorized)
    {
        responseMessage->setResponseData(message,SIP_FORBIDDEN_CODE, SIP_FORBIDDEN_TEXT);
    }
    return isAuthorized;
}

UtlBoolean
SubscribeServerThread::isAuthenticated (
    const SipMessage* message,
    SipMessage *responseMessage,
    UtlString& authenticatedUser,
    UtlString& authenticatedRealm )
{
    UtlBoolean isAuthorized = FALSE;

    // if we are not using a database we must assume authenticated
    if ( !mIsCredentialDB )
    {
        syslog(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isAuthenticated() "
            ":: No Credential DB - request is always AUTHENTICATED\n");
        isAuthorized = TRUE;
    } else
    {
        // realm and auth type should be default for server
        // if URI not defined in DB, the user is not authorized to modify bindings -
        syslog( FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isAuthenticated():TRUE realm=\"%s\" \n",
                mRealm.data());

        UtlString requestNonce;
        UtlString requestRealm;
        UtlString requestUser;
        UtlString requestUriParam;
        int requestAuthIndex = 0;
        // can have multiphe authorization / authorization-proxy headers
        // headers search for a realm match
        while ( message->getDigestAuthorizationData (
                &requestUser,
                &requestRealm,
                &requestNonce,
                NULL,
                NULL,
                &requestUriParam,
                HttpMessage::SERVER,
                requestAuthIndex) )
        {
            syslog(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isAuthenticated() "
                   "- Authorization header set in message, validate it.\n"
                   "- reqRealm=\"%s\", reqUser=\"%s\"\n", requestRealm.data(), requestUser.data());

            // case sensitive comparison of realm
            if ( mRealm.compareTo( requestRealm ) == 0 )
            {
                syslog(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isAuthenticated()"
                    "- Realm matches, now validate userid/password\n");

                // See if the nonce is valid - see net/SipNonceDb.cpp
                UtlString reqUri;
                message->getRequestUri(&reqUri);
                Url mailboxUrl (reqUri);

                UtlString authTypeDB;
                UtlString passTokenDB;
                UtlString callId;
                UtlString fromTag;
                long     nonceExpires = (5*60); // five minutes

                Url fromUrl;
                message->getFromUrl(fromUrl);
                fromUrl.getFieldParameter("tag", fromTag);

                message->getCallIdField(&callId);

                if (mNonceDb.isNonceValid(requestNonce, callId, fromTag,
                                          reqUri, mRealm, nonceExpires))
                {
                    // then get the credentials for this realm
                    if ( CredentialDB::getInstance()->
                        getCredentialByUserid (
                        mailboxUrl,
                        mRealm,
                        requestUser,
                        passTokenDB,
                        authTypeDB) )
                    {
                        // the Digest Password is calculated from the request
                        // user, passtoken, nonce and request URI

                        isAuthorized = message->verifyMd5Authorization(requestUser.data(),
                            passTokenDB.data(), requestNonce, requestRealm.data(),
                            requestUriParam.data());
                        if (isAuthorized)
                        {
                            // can have multiple credentials for same realm so only break out
                            // when we have a positive match
                            syslog(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isAuthenticated() "
                                "- request is AUTHENTICATED\n");
                            // copy the authenticated user/realm for subsequent authorization
                            authenticatedUser = requestUser;
                            authenticatedRealm = requestRealm;
                            break;
                        }
                        else
                        {
                            syslog(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isAuthenticated() "
                                "- digest authorization failed\n");
                        }
                    }
                    else
                    {
                        syslog(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isAuthenticated() "
                            "- No Credentials for mailboxUrl=\"%s\", reqRealm=\"%s\", reqUser=\"%s\"\n",
                            mailboxUrl.toString().data(),
                            requestRealm.data(),
                            requestUser.data());
                    }
                }
                else
                {
                    syslog(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isAuthenticated() "
                           "- Invalid nonce \"%s\" for mailboxUrl=\"%s\", reqRealm=\"%s\", reqUser=\"%s\"\n",
                           requestNonce.data(),
                           mailboxUrl.toString().data(),
                           requestRealm.data(),
                           requestUser.data());

                }
                // end check credentials
            }
            requestAuthIndex++;
        } //end while

        if ( !isAuthorized )
        {
            // Generate the 401 Unauthorized response to challenge for credentials
            // Use the SipNonceDB to generate a nonce
            UtlString newNonce;
            UtlString challangeRequestUri;
            UtlString callId;
            UtlString fromTag;

            Url fromUrl;
            message->getFromUrl(fromUrl);
            fromUrl.getFieldParameter("tag", fromTag);

            message->getRequestUri(&challangeRequestUri);
            message->getCallIdField(&callId);

            mNonceDb.createNewNonce(callId,
                                    fromTag,
                                    challangeRequestUri,
                                    mRealm,
                                    newNonce);

            responseMessage->setRequestUnauthorized (
                message,
                "DIGEST",
                mRealm,
                newNonce,
                "change4");  // opaque :TBD: eliminate?
        }
    }
    return isAuthorized;
}

UtlBoolean
SubscribeServerThread::isValidDomain(
    const SipMessage* message,
    SipMessage* responseMessage )
{
    UtlString address;
    UtlString requestUri;
    message->getRequestUri(&requestUri);
    Url reqUri(requestUri);
    reqUri.getHostAddress(address);
    int port = reqUri.getHostPort();

    // Compare against either an IP address of the
    // local host or a FullyQalified Domain Name
    if ( ( (address.compareTo(mDefaultDomainHostIP.data(), UtlString::ignoreCase) == 0) ||
           (address.compareTo(mDefaultDomainHostFQDN.data(), UtlString::ignoreCase) == 0) )
         && ( (mDefaultDomainPort == PORT_NONE) || (port == mDefaultDomainPort) ) )
    {
        syslog(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isValidDomain() - VALID Domain\n") ;
        return TRUE;
    }
    syslog(FAC_AUTH, PRI_DEBUG, "SubscribeServerThread::isValidDomain() - INVALID Domain\n") ;
    return FALSE;
}


SubscribeServerThread::SubscribeStatus SubscribeServerThread::addSubscription(
    const int timeNow,
    const SipMessage* subscribeMessage,
    const char* domain,
    const UtlString& eventType,
    const UtlString& eventId,
    SipMessage& response)
{
    SubscribeStatus returnStatus = STATUS_SUCCESS;
    int subscribeCseqInt = 0;
    UtlString callId;
    UtlString contactEntry;
    UtlString to;
    UtlString from;
    UtlString route;
    UtlString key;
    UtlString method;
    Url identity;

    //  Construct the identity
    UtlString uriUser, requestUri;
    subscribeMessage->getUri( NULL, NULL, NULL, &uriUser );
    subscribeMessage->getRequestUri( &requestUri );
    identity.setUserId( uriUser );
    identity.setUrlType( "sip" );
    identity.setHostAddress( domain );

    subscribeMessage->getToField(&to);
    subscribeMessage->getFromField(&from);
    subscribeMessage->getCallIdField(&callId);
    subscribeMessage->getCSeqField(&subscribeCseqInt, &method);
    subscribeMessage->getContactEntry(0, &contactEntry);

    subscribeMessage->buildRouteField(&route);
    Url toUrl;
    subscribeMessage->getToUrl(toUrl);
    int commonExpires = -1;
    if ( subscribeMessage->getExpiresField( &commonExpires ) )
    {
       if( commonExpires > 0 ) // came from request
       {
          if (commonExpires < mMinExpiresTimeint) 
          {
             returnStatus = STATUS_LESS_THAN_MINEXPIRES;
             OsSysLog::add( FAC_SIP, PRI_ERR, "addSubscription: "
                            "Expires (%d) less than Minimum (%d)",
                           commonExpires, mMinExpiresTimeint);
             return returnStatus;
          }
          else if (commonExpires > mDefaultSubscribePeriod)
          {
             commonExpires = mDefaultSubscribePeriod;
          }
          else
          {
             // commonExpires is in the allowed range - use the requested value
          }
        }
        else if( commonExpires == 0 )
        {
            // remove subscription binding
            // remove all bindings  because one contact value is *
            OsSysLog::add(FAC_SIP, PRI_DEBUG,"SubscribeServerThread::addSubscription -"
                " Removing subscription for url %s and event %s",
                toUrl.toString().data(), eventType.data());

            // Ideally the from field should be used to remove
            // the row also @TODO
            // note that the subscribe's csequence is used
            // as a remove filter here so that all
            SubscriptionDB::getInstance()->
                removeRow( toUrl.toString(),
                           from,
                           callId,
                           eventType,
                           eventId,
                           subscribeCseqInt );
            // Changes to the IMDB are reflected in the filesystem immediately
            SubscriptionDB::getInstance()->store();
            return returnStatus;
        }
        else if( commonExpires == -1) // no expires value in request
        {
            // Assume the default value
            commonExpires = mDefaultSubscribePeriod;
            OsSysLog::add(FAC_SIP, PRI_DEBUG,"SubscribeServerThread::addSubscription -"
                " No Expires Value, assigning default value (%d)", commonExpires);
        }
        response.setExpiresField(commonExpires);
    }

    OsSysLog::add(FAC_SIP, PRI_DEBUG,
        "SubscribeServerThread::addSubscription -"
        " Adding Subscription for url %s event %s",
        toUrl.toString().data(), eventType.data());

    // add bindings
    identity.getIdentity( key );

    if ( !SubscriptionDB::getInstance()->
            insertRow( requestUri, // identity,
                       callId,
                       contactEntry,
                       commonExpires + timeNow,
                       subscribeCseqInt,
                       eventType,
                       eventId,
                       to,
                       from,
                       key,                 // this will be searched for later
                       route,
                       1))                  // initial notify cseq (sent to phone)
    {
        // log the error
        OsSysLog::add(FAC_SIP, PRI_ERR,
            "SubscribeServerThread::addSubscription -"
            " Could not insert record in Database");
    }

    // persist the IMDB to its xml file
    SubscriptionDB::getInstance()->store();
    return returnStatus;
}

int SubscribeServerThread::removeSubscription (const UtlString& eventType,
                                               const UtlString& eventId,
                                               const SipMessage& sipMessage ) const
{
    int returnStatus = STATUS_SUCCESS;
    int subscribeCseqInt = 0;
    UtlString callId;
    UtlString contactEntry;
    UtlString to;
    UtlString from;
    UtlString route;
    UtlString key;
    UtlString method;
    sipMessage.getToField(&to);
    sipMessage.getFromField(&from);
    sipMessage.getCallIdField(&callId);
    sipMessage.getCSeqField(&subscribeCseqInt, &method);
    sipMessage.getContactEntry(0, &contactEntry);

    sipMessage.buildRouteField(&route);
    Url toUrl;
    sipMessage.getToUrl(toUrl);
    OsSysLog::add(FAC_SIP, PRI_DEBUG,"SubscribeServerThread::removeSubscription -"
        " Removing subscription for url %s, from %s, callid %s and event %s",
        toUrl.toString().data(), from.data(), callId.data(), eventType.data());

    // Ideally the from field should be used to remove
    // the row also @TODO
    // note that the subscribe's csequence is used
    // as a remove filter here so that we reject
    // duplicate messages
    SubscriptionDB::getInstance()->
        removeRow( toUrl.toString(),
                   from,
                   callId,
                   eventType,
                   eventId,
                   subscribeCseqInt );
    // Changes to the IMDB are reflected in the filesystem immediately
    SubscriptionDB::getInstance()->store();
    return returnStatus;
}
