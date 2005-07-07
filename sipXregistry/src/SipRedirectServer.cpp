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


// APPLICATION INCLUDES
#include <utl/UtlRegex.h>
#include "os/OsDateTime.h"
#include "os/OsFS.h"
#include "net/SipUserAgent.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/ResultSet.h"
#include "sipdb/AliasDB.h"
#include "sipdb/RegistrationDB.h"
#include "sipdb/PermissionDB.h"
#include "SipHuntGroupFilter.h"
#include "SipRedirectServer.h"

// DEFINES
#define URL_MAPPING_RULES_FILENAME "mappingrules.xml"
#define URL_FALLBACK_RULES_FILENAME "fallbackrules.xml"
enum mappingSource
{
   UrlMappingRules = 1,
   UrlFallbackRules = 2
};

// DEFINES
#define CONFIG_ETC_DIR SIPX_CONFDIR

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Ctor
SipRedirectServer::SipRedirectServer() :
    OsServerTask("SipRedirectServer-%d", NULL, 2000),
    mIsStarted(FALSE),
    mSipUserAgent(NULL),
    mUrlMappingRulesLoaded(OS_FAILED)
{}

// Dtor
SipRedirectServer::~SipRedirectServer()
{}

UtlBoolean
SipRedirectServer::initialize(
    SipUserAgent* SipUserAgent,
    const UtlString& configDir,
    const UtlString& mediaServer,
    const UtlString& voicemailServer,
    const UtlString& localDomainHost)
{
    if ( !mIsStarted )
    {   //start the thread
        start();
    }

    mSipUserAgent = SipUserAgent;

    if ( !mSipUserAgent )
    {
        return false;
    }

    UtlString fileName =
        configDir + OsPathBase::separator + URL_MAPPING_RULES_FILENAME;

    mUrlMappingRulesLoaded =
        mMap.loadMappings(
            fileName,
            mediaServer,
            voicemailServer,
            localDomainHost);

    fileName.remove(0);

    fileName =
        configDir + OsPathBase::separator + URL_FALLBACK_RULES_FILENAME;

    mFallbackRulesLoaded =
        mFallback.loadMappings (
            fileName,
            mediaServer,
            voicemailServer,
            localDomainHost );
    return true;
}

UtlBoolean
SipRedirectServer::handleMessage( OsMsg& eventMessage )
{
    OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRedirectServer::handleMessage() - "
                  "Start processing redirect message \n");

    const SipMessage* message =
        ((SipMessageEvent&)eventMessage).getMessage();

    SipMessage response;
    UtlString method;
    message->getRequestMethod(&method);

    if ( !(method.compareTo(SIP_CANCEL_METHOD, UtlString::ignoreCase) == 0) ||
         !(method.compareTo(SIP_ACK_METHOD, UtlString::ignoreCase) == 0) )
    {
        // Start by looking up contacts in the registration DB
        int numValidContacts =
            registrationDBLookUp(
                *message,
                response );

        // Alias database Look Up
        numValidContacts +=
            aliasDBLookUp(
                *message,
                response );

        // Url Mapping database Look Up
        numValidContacts +=
            urlMappingDBLookUp(
                *message, UrlMappingRules,
                response );

        // if we still do not have any contacts look up Mapping Rules 2
        if ( numValidContacts == 0 )
        {
            // if no contacts were added , check the URL mapping 2 dataset
            numValidContacts +=
                urlMappingDBLookUp(
                    *message, UrlFallbackRules, response );

        }

        if ( numValidContacts > 0 )
        {
            // we have contacts apply any hunt group reordering rules
            // to ensure that the
            SipHuntGroupFilter::getInstance()->
                reorderContacts (
                    *message,
                    response );

            response.setResponseData (
                message,
                SIP_TEMPORARY_MOVE_CODE,
                SIP_TEMPORARY_MOVE_TEXT );
        } else
        {   // finally if there are still no contacts send back a 404 not found
            OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRedirectServer::handleMessage() - "
                    "No contacts added, sending 404 response\n");

            response.setResponseData(
                message,
                SIP_NOT_FOUND_CODE,
                SIP_NOT_FOUND_TEXT );
        }
    } else if ( method.compareTo( SIP_CANCEL_METHOD, UtlString::ignoreCase ) == 0 )
    {
        OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRedirectServer::handleMessage() - "
                "sending 200 ok response to CANCEL\n");
        // return a well formed 200 response
        response.setOkResponseData(message);
    }

    UtlString finalMessageStr;
    int finalMessageLen;

    response.getBytes (
        &finalMessageStr,
        &finalMessageLen);

    OsSysLog::add(FAC_SIP, PRI_DEBUG, "\n----------------------------------\n"\
           "Sending final response\n%s\n", finalMessageStr.data());

    mSipUserAgent->send(response);
    return TRUE;
}

int
SipRedirectServer::registrationDBLookUp(
    const SipMessage& message,
    SipMessage& response)
{
    int numValidRegistrationContacts = 0;
    UtlString uri;
    UtlString requestIdentity;
    UtlString contactIdentity;

    message.getRequestUri(&uri);
    Url requestUri(uri);
    requestUri.getIdentity(requestIdentity);

    int timeNow = OsDateTime::getSecsSinceEpoch();
    ResultSet registrations;
    RegistrationDB::getInstance()->
        getUnexpiredContacts(
            requestUri, timeNow, registrations );

    int numUnexpiredContacts = registrations.getSize();

    if ( numUnexpiredContacts > 0 )
    {
        // create 3xx (temporaily moved) response
        OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRedirectServer::registrationDBLookUp() - "
                "got %d unexpired contacts\n", numUnexpiredContacts );

        for ( int i = 0; i<numUnexpiredContacts; i++ )
        {
            // Query the Registration DB for the contact,
            // expires and qvalue columns
            UtlHashMap record;
            registrations.getIndex( i, record );
            UtlString contactKey("contact");
            UtlString expiresKey("expires");
            UtlString qvalueKey("qvalue");
            UtlString contact= *((UtlString*)record.findValue(&contactKey));
            UtlString qvalue = *((UtlString*)record.findValue(&qvalueKey));

            ((UtlInt*)record.findValue(&expiresKey))->getValue();

            Url contactUri( contact );

            // check if the contact uri is not the same as the request Uri
            // if that is the only contact then proxy the request to the destination URI
            if ( !contactUri.isUserHostPortEqual( requestUri ) )
            {
                if ( !qvalue.isNull() && qvalue.compareTo( SPECIAL_IMDB_NULL_VALUE ) !=0 )
                {
                    // :TODO: (XPL-3) need a RegEx copy constructor here
                    //check if q value is numeric and between the range 0.0 and 1.0
                    static RegEx qValueValid("^(0(\\.\\d{0,3})?|1(\\.0{0,3})?)$"); 
                    if (qValueValid.Search(qvalue.data()))
                    {
                       contactUri.setFieldParameter(SIP_Q_FIELD, qvalue);
                    }
                }

                // query the # contacts already added
                int numContactsInHeader =
                    response.getCountHeaderFields( SIP_CONTACT_FIELD );

                // set the contact field in the response
                response.setContactField (
                    contactUri.toString().data(), numContactsInHeader );

                numValidRegistrationContacts++;

                OsSysLog::add( FAC_SIP, PRI_NOTICE, "SipRedirectServer::registrationDBLookUp() - "
                              "URI MAP '%s' to '%s'\n",
                              requestIdentity.data(), contactUri.toString().data());
            }
        }
    }
    return numValidRegistrationContacts;
}

int
SipRedirectServer::aliasDBLookUp(
    const SipMessage& message,
    SipMessage& response)
{
    int numValidAliasContacts = 0;
    UtlString uri;
    UtlString requestIdentity;

    message.getRequestUri(&uri);
    Url requestUri(uri);
    requestUri.getIdentity(requestIdentity);

    ResultSet aliases;
    AliasDB::getInstance()->getContacts( requestUri, aliases );
    int numAliasContacts = aliases.getSize();
    if ( numAliasContacts > 0 )
    {
        OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRedirectServer::aliasDBLookUp() - "
                "got %d AliasDB contacts\n", numAliasContacts );

        for ( int i=0; i<numAliasContacts; i++ )
        {
            static UtlString contactKey ("contact");

            UtlHashMap record;
            if ( aliases.getIndex( i, record ) )
            {   // should be a valid contact, set mailboxUrl to it and
                // loop around again, hopefully we will not recurse here!!
                UtlString contactStr;
                UtlString contact = *((UtlString*)record.findValue(&contactKey));
                Url contactUri(contact);

                // prevent loops here
                if ( !contactUri.isUserHostPortEqual(requestUri) )
                {
                    // query the # contacts already added
                    int numContactsInHeader =
                        response.getCountHeaderFields( SIP_CONTACT_FIELD );

                    // set the contact field in the response
                    response.setContactField (
                        contact, numContactsInHeader );

                    numValidAliasContacts++;
                    OsSysLog::add( FAC_SIP, PRI_NOTICE, "SipRedirectServer::aliasDBLookUp() - "
                                  "URI MAP '%s' to '%s'\n",
                                  requestIdentity.data(), contact.data());
                }
            }
        }
    }
    return numValidAliasContacts;
}

int
SipRedirectServer::urlMappingDBLookUp (
    const SipMessage& message,
    const int& passNumber,
    SipMessage& response)
{
    int numValidMappingContacts = 0;
    UtlString uri;
    UtlBoolean isPSTNNumber = false;
    UtlString permissionName;
    ResultSet urlMappingRegistrations;
    ResultSet urlMappingPermissions;

    message.getRequestUri(&uri);
    Url requestUri(uri);

    // @JC This variable is strangely overloaded
    // If we have no permissions then add any encountered
    // contacts. If we do have permissions then the
    // permission must match
    UtlBoolean permissionFound = TRUE;

    switch ( passNumber )
    {
    case UrlMappingRules:
        if ( mUrlMappingRulesLoaded == OS_SUCCESS )
        {
            mMap.getContactList(
                requestUri,
                urlMappingRegistrations,
                isPSTNNumber,
                urlMappingPermissions);
        }
        break;

    case UrlFallbackRules:
        if ( mFallbackRulesLoaded == OS_SUCCESS )
        {
            mFallback.getContactList(
                requestUri,
                urlMappingRegistrations,
                isPSTNNumber,
                urlMappingPermissions);
        }
        break;
        
    default:
       OsSysLog::add( FAC_SIP, PRI_ERR, "SipRedirectServer::urlMappingDBLookUp() - "
                     "invalid passNumber %d\n", passNumber );
       break;
    }

    int numUrlMappingPermissions = urlMappingPermissions.getSize();

    OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRedirectServer::urlMappingDBLookUp(%d) - "
                  "got %d UrlMapping Permission requirements for %d contacts\n",
                  passNumber, numUrlMappingPermissions, urlMappingRegistrations.getSize() );

    if ( numUrlMappingPermissions > 0 )
    {
        for ( int i = 0; i<numUrlMappingPermissions; i++ )
        {
            UtlHashMap record;
            urlMappingPermissions.getIndex( i, record );
            UtlString permissionKey ("permission");
            UtlString urlMappingPermissionStr =
                *((UtlString*) record.findValue(&permissionKey));

            // Try to match the permission
            // so assume it cannot be found unless we
            // see a match in the IMDB
            permissionFound = FALSE;

            // See if we can find a matching permission in the IMDB
            ResultSet dbPermissions;

            // check in permission database is user has permisssion for voicemail
            PermissionDB::getInstance()->
               getPermissions( requestUri, dbPermissions );

            int numDBPermissions = dbPermissions.getSize();

            OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRedirectServer::urlMappingDBLookUp"
                          " %d permissions configured for %s", numDBPermissions, requestUri.toString().data() );
                
            if ( numDBPermissions > 0 )
            {
               for ( int j=0; j<numDBPermissions; j++ )
               {
                  dbPermissions.getIndex( j, record );
                  UtlString dbPermissionStr =
                     *((UtlString*)record.
                       findValue(&permissionKey));

                  OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRedirectServer::urlMappingDBLookUp"
                                " checking '%s'.", dbPermissionStr.data() );
                
                  if ( dbPermissionStr.compareTo(urlMappingPermissionStr, UtlString::ignoreCase) == 0 )
                  {
                     // matching permission found in IMDB
                     permissionFound = TRUE;
                     break;
                  }
                  dbPermissionStr.remove(0);
               }
            }

            if (permissionFound)
            {
               break;               
            }
            urlMappingPermissionStr.remove(0);
        }
    }


    // either there were no requirements to match against voicemail
    // or there were and we found a match in the IMDB for the URI
    // so now add the contacts to the SIP message
    if ( permissionFound )
    {
        int numUrlMappingRegistrations = urlMappingRegistrations.getSize();

        OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRedirectServer::urlMappingDBLookUp() - "
                      "got %d UrlMapping Contacts\n", numUrlMappingRegistrations);

        if ( numUrlMappingRegistrations > 0 )
        {
            UtlString requestIdentity;
            requestUri.getIdentity(requestIdentity);

            for ( int i=0 ; i<numUrlMappingRegistrations; i++ )
            {
                UtlHashMap record;
                urlMappingRegistrations.getIndex( i, record );
                UtlString contactKey("contact");
                UtlString contact= *((UtlString*)record.findValue(&contactKey));
                Url contactUri(contact);

                // prevent recursive loops
                if ( !contactUri.isUserHostPortEqual(requestUri) )
                {
                    // query the # contacts already added
                    int numContactsInHeader =
                        response.getCountHeaderFields( SIP_CONTACT_FIELD );

                    // set the contact field in the response
                    response.setContactField (
                        contact, numContactsInHeader );

                    numValidMappingContacts++;

                    OsSysLog::add(FAC_SIP, PRI_NOTICE, "SipRedirectServer::urlMappingDBLookUp(%s) - "
                                  "URI MAP '%s' to '%s'\n",
                                  (  passNumber==UrlMappingRules  ? URL_MAPPING_RULES_FILENAME
                                   : passNumber==UrlFallbackRules ? URL_FALLBACK_RULES_FILENAME
                                   : "INVALID"
                                   ),
                                  requestIdentity.data(), 
                                  contact.data());
                }
            }
        }
        // the urlMappingRegistrations destructor cleans up its contents
        // urlMappingRegistrations.clear();
    }
    return numValidMappingContacts;
}

