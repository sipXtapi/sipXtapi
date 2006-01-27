//
// Copyright (C) 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// System includes
#include <getopt.h>

#include "os/OsDefs.h"
#if defined(_WIN32)
#   include <windows.h>
#elif defined(__pingtel_on_posix__)
#   include <unistd.h>
#endif

// APPLICATION INCLUDES
#include <utl/UtlInt.h>
#include <utl/UtlHashMapIterator.h>
#include <net/XmlRpcRequest.h>
#include <net/XmlRpcDispatch.h>
#include <os/OsSysLog.h>
#include <os/OsTask.h>
#include <os/OsConfigDb.h>

#include "configrpc/ConfigRPC.h"

// CONSTANTS
#define HTTP_PORT               8200    // Default HTTP port
#define TEST_DATASET            "test.conf"
#define TEST_FILENAME           "configTest.db"
#define TEST_VERSION            "0.1"
#define TEST_LOGFILE            "xmlrpcServer.log"


class test_Callback : public ConfigRPC_Callback
{
   void modified()
   {
   }
};

void initLogger(char* argv[])
{
    UtlString LogFile = TEST_LOGFILE;
    OsSysLog::initialize(0, // do not cache any log messages in memory
                         argv[0]); // name for messages from this program
    OsSysLog::setOutputFile(0, // no cache period
                            LogFile); // log file name

    OsSysLog::setLoggingPriority(PRI_DEBUG);
}

int main(int argc, char *argv[])
{
    OsConfigDb configDb;
    UtlString configDbFile = TEST_FILENAME;

    if (configDb.loadFromFile(configDbFile) != OS_SUCCESS)
    {
        configDb.set("TestItem1", "Value1");
        configDb.set("TestItem2", "Value2");
        configDb.set("TestItem3", "Value3");
        configDb.set("TestItem3", "Value4");        
 
        configDb.storeToFile(configDbFile);
    }
    
    ConfigRPC_Callback* confCallbacks;
    ConfigRPC*          configRPC;
    XmlRpcDispatch*     rpc;
 
    // start a simple XmlRpc test server
    rpc = new XmlRpcDispatch(HTTP_PORT, false);

    confCallbacks = new test_Callback();
    configRPC     = new ConfigRPC( TEST_DATASET,
                                   TEST_VERSION,
                                   configDbFile,
                                   confCallbacks);
    // enter the connector RPC methods in the XmlRpcDispatch table
    ConfigRPC::registerMethods(*rpc);
    
    while (1)
    {
        OsTask::delay(500);
    }
    
    exit(0);
}
