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

// Force 32-bit file offsets, because that's what Apache uses when laying out
// its interface to check_user_id.
#undef _FILE_OFFSET_BITS

// SYSTEM INCLUDES
#if defined(__pingtel_on_posix__)
#include <pwd.h>
#endif
#include <signal.h>
#include <apr_lib.h>
#include <apr_strings.h>



// APPLICATION INCLUDES
#include "utl/UtlRegex.h"
#include "os/OsFS.h"
#include "os/OsSysLog.h"
#include "net/HttpMessage.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/SIPXAuthHelper.h"
#include "../mailboxmgr/MailboxManager.h"
#include "SIPXAuthHandler.h"

// DEFINES
#define WEB_LOG_DIR         "/var/log"
#define WEB_LOG_FILENAME    "apacheauth.log"
#define WEB_DEFAULT_LOG_DIR SIPX_LOGDIR
#ifndef  SIPXCHANGE_USERNAME
# define SIPXCHANGE_USERNAME "sipx"
#endif
#define LOG_FACILITY        FAC_APACHE_AUTH

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef void (*sighandler_t)(int);

// FUNCTIONS
extern "C" {
    void  sigHandler( int sig_num );
    sighandler_t pt_signal( int sig_num, sighandler_t handler );

}

// FORWARD DECLARATIONS
// GLOBAL VARIABLES
OsMutex*     gpLockMutex = new OsMutex(OsMutex::Q_FIFO);
UtlBoolean   gShutdownFlag = FALSE;
UtlBoolean   gHandlerInitialized = FALSE;

/** helper method to determine whether we're running as sipxchange or root */
void
getCurrentUser( UtlString& user )
{
    user.remove(0);
#if defined(__pingtel_on_posix__)
    // don't use getlogin as it is not reliable
    passwd* pUserInfo = getpwuid( geteuid() );
    if ( (pUserInfo != NULL) && (pUserInfo->pw_name != NULL) )
    {
        user = pUserInfo->pw_name;
    }
#else
    user = SIPXCHANGE_USERNAME;
#endif
}

/**
 * Description:
 * closes any open connections to the IMDB safely using a mutex lock
 */
void
closeIMDBConnections ()
{

    OsSysLog::add( LOG_FACILITY, PRI_DEBUG, "closeIMDBConnections: >>>>> Entering closeIMDBconnections" );

    UtlString currentUserName;
    getCurrentUser( currentUserName );

    // can only call close on the IMDB if we're the right userid
    if ( currentUserName.compareTo(SIPXCHANGE_USERNAME) == 0 )
    {
        // Critical process lock  here
        OsLock lock( *gpLockMutex );

        // now deregister this process's database references from the IMDB
        SIPDBManager::getInstance()->releaseAllDatabase();

        delete SIPXAuthHelper::getInstance();
    }
}

/**
 * Description:
 * This is a replacement for signal() which registers a signal handler but sets
 * a flag causing system calls ( namely read() or getchar() ) not to bail out
 * upon recepit of that signal. We need this behavior, so we must call
 * sigaction() manually.
 */
sighandler_t
pt_signal( int sig_num, sighandler_t handler)
{
#if defined(__pingtel_on_posix__)
    struct sigaction action[2];
    action[0].sa_handler = handler;
    sigemptyset(&action[0].sa_mask);
    action[0].sa_flags = 0;
    sigaction ( sig_num, &action[0], &action[1] );
    return action[1].sa_handler;
#else
    return signal( sig_num, handler );
#endif
}

/**
 * Description:
 * This is the signal handler, When called this sets the
 * global gShutdownFlag allowing the main processing
 * loop to exit cleanly.
 */
void
sigHandler( int sig_num )
{
    // set a global shutdown flag
    gShutdownFlag = TRUE;

    // Unregister interest in the signal to prevent recursive callbacks
    pt_signal( sig_num, SIG_DFL );

    // Minimize the chance that we loose log data
    if (SIGTERM == sig_num)
    {
       OsSysLog::add( LOG_FACILITY, PRI_INFO, "sigHandler: terminate signal received.");
    }
    else
    {
       OsSysLog::add( LOG_FACILITY, PRI_CRIT, "sigHandler: caught signal: %d", sig_num );
    }
    OsSysLog::flush();
}

void
getPID (UtlString& rPidStr)
{
#ifdef WIN32
    int pid = GetCurrentProcessId();
#else
    pid_t pid = getpid();
#endif
    char temp[20];
    sprintf( temp, "%d", pid );
    rPidStr = temp;
}

SIPXAuthHandler::SIPXAuthHandler() : ApacheHandler()
{
    // do NOT Initialize the IMDB Singleton - Note this is called
    // running as user root so delay until running as user apache
    // as the shared memory semaphores will have permission
    // problems
}

SIPXAuthHandler::~SIPXAuthHandler()
{
    writeToLog ("~SIPXAuthHandler", "destructor", PRI_DEBUG);
    closeIMDBConnections();
}

void
SIPXAuthHandler::initializeHandler( ApacheRequestRec *pRequest )
{
    // Register Signal handlers to close IMDB, note that these
    // must be done as user "sipxchange and not as user root"
/*
    pt_signal(SIGINT,   sigHandler);    // Trap Ctrl-C on NT
    pt_signal(SIGILL,   sigHandler);
    pt_signal(SIGABRT,  sigHandler);    // Abort signal 6
    pt_signal(SIGFPE,   sigHandler);    // Floading Point Exception
*/
/*
    pt_signal(SIGTERM,  sigHandler);    // Trap kill -15 on UNIX */
#if defined(__pingtel_on_posix__)
/*
    // @JC Should not deal with a signal from this one as I do not want to disconnect
    // from the IMDB when this happens
    pt_signal(SIGHUP,   sigHandler);    // Hangup - this one is critical here for the logrotate
*/
/*
    pt_signal(SIGQUIT,  sigHandler);
    pt_signal(SIGPIPE,  sigHandler);    // Handle TCP Failure
    pt_signal(SIGBUS,   sigHandler);
    pt_signal(SIGSYS,   sigHandler);
    pt_signal(SIGXCPU,  sigHandler);
    pt_signal(SIGXFSZ,  sigHandler);
    pt_signal(SIGUSR1,  sigHandler);
    pt_signal(SIGUSR2,  sigHandler);
*/
#endif

    UtlString currentUser;
    getCurrentUser( currentUser );
    if ( currentUser.compareTo(SIPXCHANGE_USERNAME) == 0 )
    {
        // firstly get the logfile location
        UtlString logFilePath;
        if ( getLogFilePath ( logFilePath ) == OS_SUCCESS )
        {
            if ( !logFilePath.isNull() )
            {
                // Initialize the logger.
                    UtlString pid;
                    getPID( pid );
                    UtlString processname("mod_sipxauth");
                    processname.append("-");
                    processname.append(pid);
                OsStatus retval = OsSysLog::initialize(0, processname.data()) ;
                if ( retval == OS_SUCCESS )
                {
                    // Tell only auth module to dump the world
                    // syslog is typically only interested in errors
                    OsSysLog::setOutputFile( 0, logFilePath ) ;

                    // The logging priority for other facilities are set to DEBUG
                    // for debugging purpose. They should be set to PRI_ERR for
                    // production.
                    OsSysLog::setLoggingPriority( PRI_DEBUG );

                    writeToLog( "initializeHandler", (UtlString)"SIPXAuthHandler::initializeHandler", PRI_DEBUG);
                    pt_signal(SIGSEGV,  sigHandler);    // Address access violations signal 11

                    int processCount;
                    SIPDBManager::getInstance()->
                        getProcessCount( processCount );

                    char temp[20];
                    sprintf( temp, "%d", processCount );
                    UtlString processes = temp;

                    writeToLog( "initializeHandler", (UtlString)"Initialized Syslog in pid: " + pid, PRI_DEBUG);
                } else
                {
                    fprintf ( stderr, "ERROR: Could not initialize SysLog!\n");
                }
            } else
            {
                fprintf (stderr, "ERROR: Could not initialize SysLog, path not found!\n");
            }
        } else
        {
            fprintf (stderr, "ERROR: Could not initialize SysLog, path not found!\n");
        }
    } else
    {
        fprintf (stderr, "ERROR: cannot initialize the authentication module, not runnign as user '"
                 SIPXCHANGE_USERNAME "'!\n");
    }
}

void
SIPXAuthHandler::writeToLog(
    const UtlString& methodName,
    const UtlString& contents,
    const OsSysLogPriority logLevel ) const
{
    UtlString logContents = methodName + " : " + contents;
    OsSysLog::add( LOG_FACILITY, logLevel, "%s", logContents.data() );
    OsSysLog::flush();
    return;
}

int
SIPXAuthHandler::check_user_id( ApacheRequestRec *pRequest )
{
    const char *password;

    UtlString errorLog;

    // Delayed Initialization of IMDB, the shared
    // memory semaphores and mutexes take on the role
    // of the User/Groupid settings in httpd.conf.  If the
    // iniaizlize code was placed in the constructor
    // the semaphores would be owned by user Root
    // as the constructor is initialized when the library
    // is preloaded by the root initialization code

    // Critical Section here
    OsLock lock( *gpLockMutex );

    if ( !gHandlerInitialized ) {
        initializeHandler( pRequest );
        gHandlerInitialized = TRUE;
    }

    UtlString pid;
    getPID (pid);

    int processCount;
    SIPDBManager::getInstance()->
        getProcessCount( processCount );

    char temp[20];
    sprintf( temp, "%d", processCount );
    UtlString processes = temp;
    UtlString reqUrlTemp = pRequest->uri();

    writeToLog ("check_user_id", "entering pid: " + pid +
                " processcount: " + processes +
                " URI: " + reqUrlTemp);

    // Assume Unauthoriuzed until proven otherwise!
    int result = HTTP_UNAUTHORIZED;

    // Extract the password from the 'Authorization' or
    // 'Proxy-Authorization' http headers.  Calling this method
    // implicitly sets the request->user field (as it is decoded
    // in this method call)
    if ( pRequest->get_basic_auth_pw( &password ) == OK )
    {
        // fetch the apache request object (used for realm
        // and other portable runtime library calls)
        request_rec *request = pRequest->get_request_rec();

        // Get the userid
        UtlString loginString = pRequest->user();

        // this is the realm name specified in the Directory Spec
        UtlString realmName = ap_auth_name( request );

        // userid is escaped
        // to support the @ separator in the login userid
        HttpMessage::unescape ( loginString );

        // Determine which folder we are trying to access
        // the challenge rules are different for each
        // cgi-bin, userui, adminui or mailboxes folder
        AuthResource authResource = UNKNOWN;

        UtlString requestUrl = pRequest->uri();

        OsSysLog::add( LOG_FACILITY, PRI_DEBUG, "SIPXAuthHandler::check_user_id login %s realm %s request url %s",
                        loginString.data(), realmName.data(), requestUrl.data() );

        // Using the pattern specified in the request
        // Url determine the type of resource that we're looking for
        if ( getAuthResource( requestUrl, authResource ) == OS_SUCCESS )
        {
            switch ( authResource )
            {
            ///////////////
            case MAILBOXES:
            ///////////////
                {
                    // the request url pattern must contain the /mailstore/ string
                    RegEx urlPattern( (const UtlString)"/" + MAILBOX_DIR + (const UtlString)"/" );
                    if (urlPattern.Search(requestUrl.data()))
                    {
                        UtlString loginPassword = password;
                        UtlString errorLog, contactUserID, contactDomain;

                        if ( SIPXAuthHelper::getInstance()->isAuthorizedUser (
                                loginString,    // this may include @domain
                                loginPassword,
                                realmName,
                                "",             // Blank domain name
                                TRUE,           // Check for permissions also
                                contactUserID,
                                contactDomain,
                                errorLog ) )
                        {
                            // now that we have the real userid from the IMDB ensure
                            // that it is also part of the domain, this should really
                            // protect the mailboxes from unauthorozed access
                            if ( requestUrl.index( (const UtlString)"/" + MAILBOX_DIR + (const UtlString)"/" +
                                 contactUserID ) != UTL_NOT_FOUND )
                                result = OK;
                        } else
                        {
                            // write a detailed error log
                            writeToLog ("isAuthorizedUser", errorLog, PRI_DEBUG );
                            writeToLog ("isAuthorizedUser",
                                (const UtlString) "User " + loginString +
                                " failed authentication for realm: " + realmName, PRI_DEBUG );
                        }
                    } else
                    {
                        writeToLog ("check_user_id",
                            (const UtlString)"Url: " + requestUrl +
                            " invalid for mailbox access", PRI_DEBUG );
                    }
                }
                break;

            ///////////////
            case PROCESS_CGI:
            case REPLICATION_CGI:
            case MEDIASERVER_CGI:
            ///////////////
                {
                        UtlString loginPassword = password;
                        UtlString errorLog, contactUserID, contactDomain;

                        // note that the contactDomain must match the realmName
                        if ( SIPXAuthHelper::getInstance()->isAuthorizedUser (
                                loginString,    // this may include @domain
                                loginPassword,
                                realmName,
                                "",             // Blank domain name
                                FALSE,           // Check for permissions also
                                contactUserID,
                                contactDomain,
                                errorLog ) )
                        {
                                result = OK;
                        } else
                        {
                            // write a detailed error log
                            writeToLog ("isAuthorizedUser", errorLog, PRI_DEBUG );
                            writeToLog ("isAuthorizedUser",
                                (const UtlString) "User " + loginString +
                                " failed authentication for realm: " + realmName, PRI_DEBUG );
                        }
                }
                break;

            ///////////////
            case UNKNOWN:
            ///////////////
                // result remains HTTP_UNAUTHORIZED.
                break;
            }
        }
    } else
    {
        writeToLog ("check_user_id", "no authorization headers set sending 401", PRI_DEBUG );
    }

    if ( result != OK )
    {
        // Reset the headers forcing a rechallenge
        ap_note_basic_auth_failure ( pRequest->get_request_rec() );
        result = HTTP_UNAUTHORIZED;
    }

    // ensure that the process count is the same as above
    SIPDBManager::getInstance()->
        getProcessCount( processCount );

    sprintf( temp, "%d", processCount );
    processes = temp;

    SIPDBManager::getInstance()->
        getProcessCount( processCount );
    sprintf( temp, "%d", processCount );
    processes = temp;

    // this should decrement the process count to 0
    closeIMDBConnections();

    writeToLog ("<<<<## SIPXAuthHandler::check_user_id", "<<<<< Exiting  pid: " + pid +
                " processcount: " + processes +
                " URI: " + reqUrlTemp);
    return result;
}

int
SIPXAuthHandler::auth_checker( ApacheRequestRec *pRequest )
{
    // At this stage we should have the userid/passwords
    // if we need to check the parameters before this
    // then we need to override SIPXAuthHandler::access_checker
    // UtlString args (pRequest->args());
    return OK;
}

OsStatus
SIPXAuthHandler::getAuthResource (
    const UtlString& requestUrl,
    AuthResource& rAuthResource ) const
{
    OsStatus result = OS_SUCCESS;

    // Search for a specified alias
    if ( requestUrl.index ("cgi-bin", UtlString::ignoreCase ) != UTL_NOT_FOUND )
    {
        if (requestUrl.index ("mediaserver.cgi", UtlString::ignoreCase ) != UTL_NOT_FOUND)
        {
            rAuthResource = MEDIASERVER_CGI;
        }
        else if (requestUrl.index ("process.cgi", UtlString::ignoreCase ) != UTL_NOT_FOUND)
        {
            rAuthResource = PROCESS_CGI;
        }
        else if (requestUrl.index ("replication.cgi", UtlString::ignoreCase ) != UTL_NOT_FOUND)
        {
            rAuthResource = REPLICATION_CGI;
        }
        else
        {
            rAuthResource = UNKNOWN;
            result = OS_FAILED;
        }
    } else if ( requestUrl.index ( MEDIASERVER_ROOT_ALIAS, UtlString::ignoreCase ) != UTL_NOT_FOUND )
    {
        // if the url contains the MEDIASERVER_ROOT_ALIAS string this is a mailboxes url request
        rAuthResource = MAILBOXES;
    } else
    {
        rAuthResource = UNKNOWN;
        result = OS_FAILED;
    }
    OsSysLog::add( LOG_FACILITY, PRI_DEBUG, "SIPXAuthHandler::getAuthResource %s type %d result %d ", requestUrl.data(), (int)rAuthResource, (int)result);
OsSysLog::flush();
    return result;
}

/**
 * Experimental access control
 *
 * @param pRequest
 *
 * @return
 */
int
SIPXAuthHandler::access_checker(ApacheRequestRec *pRequest)
{
    return DECLINED;
}

OsStatus
SIPXAuthHandler::getLogFilePath ( UtlString& logFilePath ) const
{
    OsStatus result = OS_SUCCESS;

    // Make sure that the SIPXCHANGE_HOME ends with
    // a trailing file separator character
    char *sipxHomeEnv = getenv ("SIPXCHANGE_HOME");
    OsPath path;
    if ( sipxHomeEnv != NULL )
    {
        UtlString sipxHome = sipxHomeEnv;
        // if the last character is a separator strip it.
        if ( sipxHome ( sipxHome.length() -1 ) == OsPath::separator )
            sipxHome = sipxHome(0, sipxHome.length()-1);

        if ( OsFileSystem::exists( sipxHome + WEB_LOG_DIR) )
        {
            path = sipxHome + WEB_LOG_DIR;
        } else
        {
            // set to current working directory as the dir does not exist
            OsFileSystem::getWorkingDirectory(path);
        }
    } else // Environment Variable is not defined check default location
    {
        if ( OsFileSystem::exists( WEB_DEFAULT_LOG_DIR ) )
        {
            path = WEB_DEFAULT_LOG_DIR;
        } else
        {
            // set to current working directory
            OsFileSystem::getWorkingDirectory(path);
        }
    }

    // now that we have the path to the logfile directory
    // append the name of the log file
    path += OsPath::separator + WEB_LOG_FILENAME;

    // Finally translate path to native path,
    // this will ensure the volume is set correctly
    OsPath nativePath ;
    path.getNativePath(nativePath);
    logFilePath = nativePath ;
    return result;
}

/**
 * Experimental - not used here
 *
 * @param pRequest
 *
 * @return
 */
/*
int
SIPXAuthHandler::handler(ApacheRequestRec *pRequest)
{
    char buf[1024 * 8];
    apr_size_t len_read;

    ap_setup_client_block(pRequest->get_request_rec(), REQUEST_CHUNKED_DECHUNK);

    mHits++;
    pRequest->rprintf("\nThis handler has dealt with %d hits", mHits);
    pRequest->rputs("\nLets Dump The Request!\n");
    pRequest->dump();
    ap_setup_client_block(pRequest->get_request_rec(), REQUEST_CHUNKED_ERROR);
    if( (pRequest->method_number() == M_POST) ||
        (pRequest->method_number() == M_PUT) ) {
        pRequest->rprintf("Content:");
        while((len_read = pRequest->get_client_block(buf, sizeof(buf))) > 0) {
            pRequest->rprintf("%s", apr_pstrndup(pRequest->pool(),
                                                 buf, len_read));
        }
    }
    return OK;
}
*/

/*=====================INSTANCIATORS==========================*/

ApacheHandler *instanciate_auth()
{
    return new SIPXAuthHandler();
}

// Make sure this symbol is exported and also it is crucial
// that the symbol is demangled
cpp_factory_t AP_MODULE_DECLARE_DATA mod_sipxauth = {
    instanciate_auth,
    NULL,
    NULL,
    NULL,
};
