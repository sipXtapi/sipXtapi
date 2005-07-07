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
#include <iostream>
#include "utl/UtlInt.h"
#include "utl/UtlSortedList.h"
#include "os/OsMsgQ.h"
#include "os/OsProcessIterator.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/ResultSet.h"
#include "sipdb/AliasDB.h"
#include "sipdb/HuntgroupDB.h"
#include "sipdb/ExtensionDB.h"
#include "sipdb/PermissionDB.h"
#include "sipdb/RegistrationDB.h"
#include "sipdb/SubscriptionDB.h"
#include "sipdb/DialByNameDB.h"
#include "sipdb/CredentialDB.h"
#include "sipdb/RegistrationDB.h"
#include "sipdb/AuthexceptionDB.h"
#include "sipdb/SIPXAuthHelper.h"
#include "IMDBTaskMonitor.h"
#include "DisplayTask.h"

extern UtlString uriKey;
extern UtlString extensionKey;
extern UtlString callidKey;
extern UtlString contactKey;
extern UtlString realmKey;
extern UtlString useridKey;
extern UtlString passtokenKey;
extern UtlString authtypeKey;
extern UtlString identityKey;
extern UtlString userKey;
extern UtlString permissionKey;
extern UtlString qvalueKey;
extern UtlString expiresKey;
extern UtlString timenowKey;
extern UtlString subscribecseqKey;
extern UtlString eventtypeKey;
extern UtlString toKey;
extern UtlString cseqKey;
extern UtlString fromKey;
extern UtlString fileKey;
extern UtlString keyKey;
extern UtlString recordrouteKey;
extern UtlString notifycseqKey;
extern UtlString np_identityKey;
extern UtlString np_contactKey;
extern UtlString np_digitsKey;

using namespace std ;

DisplayTask::DisplayTask ( 
    const UtlString& rArgument, OsMsgQ& rMsgQ, OsEvent& rCommandEvent) : 
    IMDBWorkerTask( rArgument, rMsgQ, rCommandEvent )
{}

DisplayTask::~DisplayTask()
{}

int 
DisplayTask::run( void* runArg )
{
    osPrintf ("Starting Thread\n");
    OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "Starting Thread\n");
    // Indicate that we're finished, the monitor thread
    // reads this flag and if it's still set
    setBusy (TRUE);

    showTableRows ( mArgument );

    UtlString databaseInfo;
    getDatabaseInfo ( databaseInfo );
    osPrintf ( "%s", databaseInfo.data() );
    OsSysLog::add(LOG_FACILITY, PRI_DEBUG, databaseInfo.data());

    setBusy (FALSE);

    // send a success message to the sleeping monitor
    notifyMonitor( USER_DISPLAY_SUCCESS_EVENT );

    cleanupIMDBResources();
    osPrintf ("Stopping Thread");
    OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "Stopping Thread\n");
    return( TRUE );
}

// This function displays all rows from the table specified
// using the new cursor/resultset interface
int
DisplayTask::showTableRows ( const UtlString& rTableName ) const
{
    int exitCode  = EXIT_SUCCESS;
    ResultSet resultSet;
    if ( rTableName.compareTo("processinfo" , UtlString::ignoreCase)==0 )
    {
        UtlString processInfo;
        getProcessInfo(TRUE, processInfo);
        cout << processInfo.data() << endl;
    } else if ( rTableName.compareTo("credential" , UtlString::ignoreCase)==0 )
    {
        CredentialDB::getInstance()->getAllRows (resultSet);
        for (int i=0; i< resultSet.getSize(); i++)
        {
            UtlHashMap record;
            resultSet.getIndex( i, record );
            UtlString uri = *((UtlString*)record.findValue(&uriKey));
            UtlString realm = *((UtlString*)record.findValue(&realmKey));
            UtlString userid = *((UtlString*)record.findValue(&useridKey));
            UtlString passtoken = *((UtlString*)record.findValue(&passtokenKey));
            UtlString authtype = *((UtlString*)record.findValue(&authtypeKey));
            cout << "Credential Row" << endl;
            cout << "==============" << endl;
            cout << "uri:\t\t"     << uri.data()       << endl \
                 << "realm:\t\t"   << realm.data()     << endl \
                 << "userid:\t\t"  << userid.data()    << endl \
                 << "passtoken:\t" << passtoken.data() << endl \
                 << "authtype:\t"  << authtype.data()  << endl << endl;
        }
    } else if ( rTableName.compareTo("huntgroup" , UtlString::ignoreCase)==0 ) 
    {
        HuntgroupDB::getInstance()->getAllRows( resultSet );
        for (int i=0; i< resultSet.getSize(); i++)
        {
            UtlHashMap record;
            resultSet.getIndex( i, record );
            UtlString identity = *((UtlString*)record.findValue(&identityKey));
            cout << "Huntgroup Row" << endl \
                 << "=============" << endl \
                 << "identity:\t" << identity.data() << endl << endl;
        }
    } else if ( rTableName.compareTo("authexception" , UtlString::ignoreCase)==0 ) 
    {
        AuthexceptionDB::getInstance()->getAllRows( resultSet );
        for (int i=0; i< resultSet.getSize(); i++)
        {
            UtlHashMap record;
            resultSet.getIndex( i, record );
            UtlString user = *((UtlString*)record.findValue(&userKey));
            cout << "Authexception Row" << endl \
                 << "=================" << endl \
                 << "user:\t" << user.data() << endl << endl;
        }
    } else if ( rTableName.compareTo("registration" , UtlString::ignoreCase)==0  )
    {
        // Find all unexpired contacts
        RegistrationDB::getInstance()->getAllRows ( resultSet );
        for (int i=0; i< resultSet.getSize(); i++)
        {
            UtlHashMap record;
            resultSet.getIndex( i, record );
            UtlString uri    = *((UtlString*)record.findValue(&uriKey));
            UtlString callid = *((UtlString*)record.findValue(&callidKey));
            UtlString contact= *((UtlString*)record.findValue(&contactKey));
            UtlString qvalue = *((UtlString*)record.findValue(&qvalueKey));
            int cseq        = ((UtlInt*)record.findValue(&cseqKey))->getValue();
            int expires     = ((UtlInt*)record.findValue(&expiresKey))->getValue();

            cout << "Registration Row" << endl \
                 << "================" << endl \
                 << "uri:\t\t"    << uri.data()     << endl \
                 << "callid:\t\t" << callid.data()  << endl \
                 << "contact:\t"  << contact.data() << endl \
                 << "qvalue:\t\t" << qvalue.data()  << endl \
                 << "cseq:\t\t"   << cseq           << endl \
                 << "expires:\t"  << expires        << endl << endl;
        }
    } else if ( rTableName.compareTo("alias" , UtlString::ignoreCase)==0 )
    {
        AliasDB::getInstance()->getAllRows( resultSet );
        for (int i=0; i< resultSet.getSize(); i++)
        {
            UtlHashMap record;
            resultSet.getIndex( i, record );
            UtlString aliasIdentity = *((UtlString*)record.findValue(&identityKey));
            UtlString contact = *((UtlString*)record.findValue(&contactKey));
            cout << "Alias Row" << endl \
                 << "=========" << endl \
                 << "aliasIdentity:\t" << aliasIdentity.data() << endl \
                 << "contact:\t" << contact.data() << endl << endl;
        }
    } else if ( rTableName.compareTo("extension" , UtlString::ignoreCase)==0 )
    {
        ExtensionDB::getInstance()->getAllRows( resultSet );

        for (int i=0; i< resultSet.getSize(); i++)
        {
            UtlHashMap record;
            resultSet.getIndex( i, record );
            UtlString uri = *((UtlString*)record.findValue(&uriKey));
            UtlString extension = *((UtlString*)record.findValue(&extensionKey));
            cout << "Extension Row" << endl \
                 << "=============" << endl \
                 << "uri:\t\t"      << uri.data()       << endl \
                 << "extension:\t"  << extension.data() << endl << endl;
        }
    } else if (  rTableName.compareTo("permission" , UtlString::ignoreCase)==0  )
    {
        // Find all unexpired contacts
        PermissionDB::getInstance()->getAllRows ( resultSet );

        UtlString identity, permission;
        for (int i=0; i< resultSet.getSize(); i++)
        {
            UtlHashMap record;
            resultSet.getIndex( i, record );
            UtlString permission = *((UtlString*)record.findValue(&permissionKey));
            UtlString identity = *((UtlString*)record.findValue(&identityKey));

            cout << "Permission Row"   << endl \
                 << "=============="   << endl \
                 << "identity:\t\t"    << identity.data()   << endl \
                 << "permission:\t\t"  << permission.data() << endl << endl;
        }
    } else if ( rTableName.compareTo("dialbyname" , UtlString::ignoreCase)==0 )
    {
        // Find all unexpired contacts
        DialByNameDB::getInstance()->getAllRows ( resultSet );

        for (int i=0; i<resultSet.getSize(); i++)
        {
            UtlHashMap record;
            resultSet.getIndex( i, record );
            UtlString np_identity = *((UtlString*)record.findValue(&np_identityKey));
            UtlString np_contact = *((UtlString*)record.findValue(&np_contactKey));
            UtlString np_digits = *((UtlString*)record.findValue(&np_digitsKey));

            cout << "DialByName Row" << endl \
                 << "==============" << endl \
                 << "np_identity:\t" << np_identity.data() << endl \
                 << "np_contact:\t"  << np_contact.data()  << endl  \
                 << "np_digits:\t"   << np_digits.data()   << endl << endl;
        }
    } else if ( rTableName.compareTo("registration" , UtlString::ignoreCase)==0 )
    {
        // Find all unexpired contacts
        RegistrationDB::getInstance()->getAllRows ( resultSet );

        for (int i=0; i< resultSet.getSize(); i++)
        {
            UtlHashMap record;
            resultSet.getIndex( i, record );
            UtlString uri = *((UtlString*)record.findValue(&uriKey));
            UtlString callid = *((UtlString*)record.findValue(&callidKey));
            UtlString contact = *((UtlString*)record.findValue(&contactKey));
            UtlString qvalue = *((UtlString*)record.findValue(&qvalueKey));
            int expires = ((UtlInt*)record.findValue(&expiresKey))->getValue();
            int cseq    = ((UtlInt*)record.findValue(&cseqKey))->getValue();

            cout << "Registration Row" << endl \
                 << "================" << endl \
                 << "uri:\t"     << uri.data()     << endl \
                 << "callid:\t"  << callid.data()  << endl \
                 << "contact:\t" << contact.data() << endl \
                 << "qvalue:\t"  << qvalue.data()  << endl \
                 << "expires:\t" << expires        << endl \
                 << "cseq:\t"    << cseq           << endl << endl;
        }
    } else if ( rTableName.compareTo("subscription" , UtlString::ignoreCase)==0  ) 
    {
        SubscriptionDB::getInstance()->getAllRows ( resultSet );

        for (int i=0; i< resultSet.getSize(); i++)
        {
            UtlHashMap record;
            resultSet.getIndex( i, record );
            UtlString uri        = *((UtlString*)record.findValue(&uriKey));
            UtlString callid     = *((UtlString*)record.findValue(&callidKey));
            UtlString contact    = *((UtlString*)record.findValue(&contactKey));
            int expires          = ((UtlInt*)record.findValue(&expiresKey))->getValue();
                                   ((UtlInt*)record.findValue(&subscribecseqKey))->getValue();
            UtlString eventtype  = *((UtlString*)record.findValue(&eventtypeKey));
            UtlString to         = *((UtlString*)record.findValue(&toKey));
            UtlString from       = *((UtlString*)record.findValue(&fromKey));
            UtlString file       = *((UtlString*)record.findValue(&fileKey));
            UtlString key        = *((UtlString*)record.findValue(&keyKey));
            UtlString recordroute= *((UtlString*)record.findValue(&recordrouteKey));
            int notifycseq       = ((UtlInt*)record.findValue(&notifycseqKey))->getValue();

            cout << "Subscription Row" << endl \
                 << "================" << endl \
                 << "uri:\t\t"      << uri.data()        << endl \
                 << "callid:\t\t"   << callid.data()     << endl \
                 << "contact:\t"    << contact.data()    << endl \
                 << "expires:\t"    << expires           << endl \
                 << "eventtype:\t"  << eventtype.data()  << endl \
                 << "to:\t\t"       << to.data()         << endl \
                 << "from:\t\t"     << from.data()       << endl \
                 << "file:\t\t"     << file.data()       << endl \
                 << "key:\t\t"      << key.data()        << endl \
                 << "recordroute:\t"<< recordroute.data()<< endl \
                 << "notifycseq:\t" << notifycseq        << endl << endl;
        }
    } else
    {
        exitCode = EXIT_BADSYNTAX;
    }
    return exitCode;
}


