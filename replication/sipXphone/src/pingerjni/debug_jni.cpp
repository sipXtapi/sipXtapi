// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/debug_jni.cpp#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#include "pingerjni/debug_jni.h"
#include "os/OsDefs.h"

bool g_bRealtimeDebug     = DEBUG_REALTIME_TOGGLE ;
bool g_bRealtimeProfiling = JNI_REALTIME_PROFILING ;
bool g_bWVDebug             = DEBUG_REALTIME_WINDVIEW ;
int  g_iDebugLevel        = 1 ;

int   g_iNestingDepth      = 0 ;
const char* g_szNestingPadding = "                    \0" ;
                                        
extern "C" void DisplayJNIDebugSettings()
{
    osPrintf("\n") ;
    osPrintf("JNI Debugging Settings:\n") ;
    osPrintf("\tCall Listeners: %d\n", DEBUG_FIRE_CALL_LISTENERS) ;
    osPrintf("\tComponent Listeners: %d\n", DEBUG_FIRE_COMPONENT_LISTENERS) ;
    osPrintf("\tMethod Calls: %d\n", DEBUG_METHOD_CALLS) ;
    osPrintf("\tFinalize: %d\n", DEBUG_FINALIZE) ;
    osPrintf("\tGetVM: %d\n", DEBUG_GET_VM) ;
    osPrintf("\tInclude WindView: %d\n", DEBUG_INCL_WINDVIEW) ;
    osPrintf("\tProfiling Support: %d\n", JNI_REALTIME_PROFILING) ;
    osPrintf("\tDebug Level: %d\n", g_iDebugLevel) ;
    osPrintf("\n") ;
}


/** 
 * Enable JNI Debugging with a starting debugging level
 */
extern "C" void EnableJNIDebugging(int iLevel)
{
    g_bRealtimeDebug = true ;
    SetJNIDebugLevel((iLevel <= 0) ? DEBUG_DEFAULT_LEVEL : iLevel) ;
}


extern "C" int SetJNIDebugLevel(int iLevel)
{
    int iOldLevel = g_iDebugLevel ;

    g_iDebugLevel = iLevel ;

    DisplayJNIDebugSettings() ;   

    return iOldLevel ;
}


extern "C" int GetJNIDebugLevel()
{
    return g_iDebugLevel ;
}


/** 
 * Disable JNI Debugging
 */
extern "C" void DisableJNIDebugging()
{
    g_bRealtimeDebug = false ;
}


extern "C" bool IsJNIDebuggingEnabled()
{
    return g_bRealtimeDebug ;
}


extern "C" void EnableJNIProfiling()
{
    g_bRealtimeProfiling = true ;
}


extern "C" void DisableJNIProfiling()
{
    g_bRealtimeProfiling = false ;
}

extern "C" bool IsJNIProfilingEnabled()
{
    return g_bRealtimeProfiling ;
}


extern "C" void EnableWVDebugging()
{
    g_bWVDebug = true ;
}


extern "C" void DisableWVDebugging()
{
    g_bWVDebug = false ;
}


extern "C" bool IsWVDebuggingEnabled()
{
    return g_bWVDebug ;
}
