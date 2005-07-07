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
#ifndef SIPXAUTHHANDLER_H
#define SIPXAUTHHANDLER_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
// mod_cpluscplus
#include "apache_handler.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class Url;
class ApacheRequestRec;

/**
 * Authentication handler for SIP Exchange web servers.  The module
 * uses httpd.conf to specify a set of protected directory specs and how
 * these directories should be challenged.
 *
 * @author John P. Coffey
 * @version 1.0
 */
class SIPXAuthHandler : public ApacheHandler
{
public:
    /**
     * List of authentication types in the module, the USERUI reqires
     * a valid IMDB userid and password where as the mailbox requires
     * either the system administrator or a valid mailbox id
     */
    enum AuthResource
    {
       MEDIASERVER_CGI,         /* for now do not requires administrator userid/password */
       PROCESS_CGI,         /* requires administrator userid/password */
       REPLICATION_CGI,         /* requires administrator userid/password */
       MAILBOXES,   /* url must contain realm (domain) and a valid sip credential */
       UNKNOWN      /* this is the error case */
    };

    /** Ctor */
    SIPXAuthHandler (void);

    /** Dtor */
    ~SIPXAuthHandler(void);

    // int handler(ApacheRequestRec *pRequest);
    /**
     * The check_user_id callback allows modules to perform user
     * authentication before allowing them to proceed.
     * This can be done in a variety of ways, such as a database,
     * configuration directive, or anything the module wishes.
     *
     * <P>This routine should typically return one of 3 values,
     * HTTP_UNAUTHORIZED, OK, and DECLINED, but need not be
     * limited by these. All check_user_id handlers are run until
     * one returns a value other than DECLINED.
     *
     * @return The result of check_user_id is one of the standard
     * Apache hook result codes.
     */
    int check_user_id(ApacheRequestRec *pRequest);

    /**    writeToLog ( "~SIPXAuthHandler", "closing IMDB...", PRI_INFO );

     * The auth_checker callback allows modules to grant or deny access
     * to data based upon their user data. All auth_checker handlers
     * are run until one returns a value other than DECLINED.
     *
     * @return The result of auth_checker is one of the standard
     * Apache hook result codes.
     */
    int auth_checker(ApacheRequestRec *pRequest);

    // @JC Added the access_checker so that it might act as a
    // kind of mailbox manager - this is experimental
    /**
     * The access_checker callback is called after the header_parser
     * stage, and gives the module an opportunity to allow or deny
     * a request based upon whichever criteria it wishes.
     * mod_access uses the access_checker hook in order to allow or deny
     * requests based upon various criteria such as IP address.
     *
     * @param pRequest
     *
     * @return
     */
    int access_checker(ApacheRequestRec *pRequest);
private:

    /**
     * This returns a well formed ULR from the apache request
     * 
     * @param inputString
     * @param realmName
     * @param rUrlString
     * 
     * @return 
     */
    OsStatus getWellFormedURL (
        const UtlString& inputString,
        const UtlString& realmName,
        UtlString& rUrlString ) const;

    /**
     *
     * @param logFilePath
     */
    OsStatus getLogFilePath ( UtlString& logFilePath ) const;

    /**
     *
     * @param loginID
     * @param contact
     */
    void writeToLog( 
        const UtlString& methodName,
        const UtlString& contents, 
        const OsSysLogPriority logLevel = PRI_DEBUG ) const;

    /**
     * Using the login identity, search the aliases IMDB table
     * for a credential identity (there must only be one matching row)
     * If the aliases table has no match the loginId can be used
     *
     * @param loginID
     * @param contact
     *
     * @return
     */
    UtlBoolean getContactUri (
        const UtlString& loginID,
        Url& contact ) const;

    /**
     * Determine the kind of authentication required for this request
     *
     * @param requestUrl
     * @param realmName
     * @param rAuthResource
     *                   returns MAILBOX, CGI or OTHER (the latter being unauthorized
     *                   and resulting in an OS_FAILED return code
     *
     * @return OS_SUCCESS if the url pattern matches either a mailbox or a
     *         sipxchange cgi directory structure
     */
    OsStatus getAuthResource(
        const UtlString& requestUrl,
        AuthResource&   rAuthResource ) const;

    /**
     * Put in lazy IMDB Initializer to work around apache
     * root permission identity security for shared memory ownership
     *
     * @param pRequest
     */
    void initializeHandler( ApacheRequestRec *pRequest );
};

// Make sure apache does not see the mangled name
extern "C" { extern AP_MODULE_DECLARE_DATA cpp_factory_t mod_sipxauth; }

#endif // SIPXAUTHHANDLER_H

