// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/javaJNIArgInit.cpp#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#include <jni.h>
#include <string.h>

#include "os/OsConfigDb.h"
#include "os/OsSocket.h"
#include "os/OsDefs.h"
#include "config.h"

#ifdef __pingtel_on_posix__
#include <malloc.h>
#endif

// Location of Configuration File
#define JVM_DEVICE_CONFIG_FILE CONFIG_PREFIX_USER_DATA "pinger-config"
#define JVM_USER_CONFIG_FILE CONFIG_PREFIX_USER_DATA "user-config"

// Defaults if config file is not found
#define JVM_DEFAULT_NATIVE_STACK    (16384)         // Native Stack size
#define JVM_DEFAULT_JAVA_STACK      (32768)         // Java Stack Size
#define JVM_DEFAULT_MIN_HEAP        (5*1024*1024)   // Min Heap Setting
#define JVM_DEFAULT_MAX_HEAP        (5*1024*1024)   // Max Heap Settinsg

// Globals
static bool     g_bInitializedParams    = FALSE ;
static UtlString g_strClassPath ;
static int      g_iAsyncGC              = TRUE ;
static int      g_iVerboseGC            = FALSE ;
static int      g_iVerifyMode           = 0 ;
static int      g_iNativeStackSize      = JVM_DEFAULT_NATIVE_STACK ;
static int      g_iJavaStackSize        = JVM_DEFAULT_JAVA_STACK ;
static int      g_iMinHeap              = JVM_DEFAULT_MIN_HEAP ;
static int      g_iMaxHeap              = JVM_DEFAULT_MAX_HEAP ;

// Local Prototypes
void loadSettings(const char* szClassPath) ;
bool initConfigFile(OsConfigDb* config) ;


// Externs
extern "C" void javaClassPathSet(const char* classPath) ;
extern "C" const char* javaClassPathGet() ;


/**
 * Called when starting up the VM or attaching to VM for the purpose of
 * tweaking params.
 */
extern "C" void javaJNIArgInit(void *pArgs)
{   
    bool bReportSettings = false ;

    JDK1_1InitArgs* pJniArgs = (JDK1_1InitArgs*) pArgs ;

    // Turn off class GC (problems with JNI)
    pJniArgs->enableClassGC = FALSE ;


    // Load params if needed
    if (g_bInitializedParams == FALSE) {
        loadSettings(pJniArgs->classpath) ;
        g_bInitializedParams = TRUE ;
        bReportSettings = true ;
    }

    // Update VM Params
    pJniArgs->enableVerboseGC = g_iVerboseGC ;
    pJniArgs->verifyMode = g_iVerifyMode ;
    pJniArgs->disableAsyncGC = !g_iAsyncGC ;
    pJniArgs->nativeStackSize = g_iNativeStackSize ;
    pJniArgs->javaStackSize = g_iJavaStackSize ;
    pJniArgs->minHeapSize = g_iMinHeap ;
    pJniArgs->maxHeapSize = g_iMaxHeap ;

    // Reset classpath only if needed
    if (strcmp(g_strClassPath.data(), pJniArgs->classpath) != 0) {
        pJniArgs->classpath = (char *) malloc(strlen(g_strClassPath.data())+1) ;
        strcpy(pJniArgs->classpath, g_strClassPath.data()) ;
    }

    // Dump Results
    if (bReportSettings)
    {   
        osPrintf("Initializing JVM Settings\n") ;
        osPrintf("     classpath: %s\n", pJniArgs->classpath) ;
        osPrintf("      class gc: %d\n", pJniArgs->enableClassGC) ; 
        osPrintf("    verbose gc: %d\n", pJniArgs->enableVerboseGC) ;
        osPrintf("        verify: %d\n", pJniArgs->verifyMode) ;    // 0 -- none, 1 -- remotely loaded code, 2 -- all code.
        osPrintf("  native stack: 0x%08X (%d)\n", pJniArgs->nativeStackSize, pJniArgs->nativeStackSize) ;
       osPrintf("    java stack: 0x%08X (%d)\n", pJniArgs->javaStackSize, pJniArgs->javaStackSize) ;
        osPrintf("      min heap: 0x%08X (%d)\n", pJniArgs->minHeapSize, pJniArgs->minHeapSize) ;
        osPrintf("      max heap: 0x%08X (%d)\n", pJniArgs->maxHeapSize, pJniArgs->maxHeapSize) ;
    }
}


/**
 * loads settings from the config db file
 */
void loadSettings(const char* strClassPath)
{
    int         iValue ;
    UtlString    strValue ;
    OsConfigDb  config ;

    // Load up user defined settings
    if (initConfigFile(&config)) {
        if (config.get("JVM_VERBOSE_GC", iValue) != OS_NOT_FOUND) {
            g_iVerboseGC = iValue ;
        }
        if (config.get("JVM_VERIFY_MODE", iValue) != OS_NOT_FOUND) {
            g_iVerifyMode = iValue ;
        }
        if (config.get("JVM_NATIVE_STACK", iValue) != OS_NOT_FOUND) {
            g_iNativeStackSize = iValue ;
        }
        if (config.get("JVM_JAVA_STACK", iValue) != OS_NOT_FOUND) {
            g_iJavaStackSize = iValue ;
        }
        if (config.get("JVM_MIN_HEAP", iValue) != OS_NOT_FOUND) {
            g_iMinHeap = iValue ;
        }
        if (config.get("JVM_MAX_HEAP", iValue) != OS_NOT_FOUND) {
            g_iMaxHeap = iValue ;
        }
        if (config.get("JVM_CLASS_PATH", strValue) != OS_NOT_FOUND) {
            strValue.append(":") ;
            strValue.append(strClassPath) ;

            g_strClassPath = strValue ;
        } else {
            g_strClassPath = strClassPath ;
        }
    }
}


bool initConfigFile(OsConfigDb* config)
{
    bool     bRC = false ;
    UtlString configFileName ;

    // Device Config
    configFileName.insert(0, JVM_DEVICE_CONFIG_FILE) ;
    OsStatus retval =  config->loadFromFile((char *)configFileName.data());

    // User-config
    configFileName.remove(0);
    configFileName.insert(0, JVM_USER_CONFIG_FILE);
    config->loadFromFile((char *)configFileName.data()) ;

    // Ignore user-config, if server config or device config
    if(retval == OS_SUCCESS)
    {
        bRC = true ;        
    }

    return bRC ;
}



extern "C" void InitializeJVMSettings(JDK1_1InitArgs* pJNIArgs)
{
    // Turn off class GC (problems with JNI)
    pJNIArgs->enableClassGC = FALSE ;

    // Load params if needed
    loadSettings(pJNIArgs->classpath) ;

    // Update VM Params
    pJNIArgs->enableVerboseGC = g_iVerboseGC ;
    pJNIArgs->verifyMode = g_iVerifyMode ;
    pJNIArgs->nativeStackSize = g_iNativeStackSize ;
    pJNIArgs->javaStackSize = g_iJavaStackSize ;
    pJNIArgs->minHeapSize = g_iMinHeap ;
    pJNIArgs->maxHeapSize = g_iMaxHeap ;
    pJNIArgs->classpath = (char *) g_strClassPath.data() ;  


    osPrintf("Initializing JVM Settings\n") ;
    osPrintf("     classpath: %s\n", pJNIArgs->classpath) ;
    osPrintf("      class gc: %d\n", pJNIArgs->enableClassGC) ; 
    osPrintf("    verbose gc: %d\n", pJNIArgs->enableVerboseGC) ;
    osPrintf("        verify: %d\n", pJNIArgs->verifyMode) ;    // 0 -- none, 1 -- remotely loaded code, 2 -- all code.
    osPrintf("  native stack: 0x%08X (%d)\n", pJNIArgs->nativeStackSize, pJNIArgs->nativeStackSize) ;
    osPrintf("    java stack: 0x%08X (%d)\n", pJNIArgs->javaStackSize, pJNIArgs->javaStackSize) ;
    osPrintf("      min heap: 0x%08X (%d)\n", pJNIArgs->minHeapSize, pJNIArgs->minHeapSize) ;
    osPrintf("      max heap: 0x%08X (%d)\n", pJNIArgs->maxHeapSize, pJNIArgs->maxHeapSize) ;
}
