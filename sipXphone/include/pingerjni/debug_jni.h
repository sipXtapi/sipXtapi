// $Id: //depot/OPENDEV/sipXphone/include/pingerjni/debug_jni.h#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////


/**
 * debug_jni.h - Debug flags for JNI
 * 
 */

#ifndef _DEBUG_JNI_H
#define _DEBUG_JNI_H

#include "os/OsTimeLog.h"


// Cannot use TRUE/FALSE for the following switches
// as TRUE/FALSE are defined as true/false for some flavors of
// Rogue Wave.  This causes cpp to not be happy with #if constructs 
#define DEBUG_FIRE_CALL_LISTENERS       0    // Report all call events?
#define DEBUG_FIRE_COMPONENT_LISTENERS  0   // Report all component events?

#define DEBUG_METHOD_CALLS              0   // Report ALL method calls
#define DEBUG_PROFILING                 0   // Include Profiling support?
#define DEBUG_VALID_STRINGS             0   // Should we validate C-Strings?
#define DEBUG_FINALIZE                  0   // Report when objects are finialized
#define DEBUG_GET_VM                    0   // Report when obtaining/releasing JVMs
#define DEBUG_INCL_WINDVIEW             0   // Should we fire wind view events?

#define DEBUG_REALTIME_TOGGLE           0    // Should debugging output be enabled on start up?
#define DEBUG_REALTIME_WINDVIEW         0    // Should we default to debugging on?
#define JNI_REALTIME_PROFILING          0    // Should profiling be enabled on start up?

#define DEBUG_DEFAULT_LEVEL             2 // Default level picked via EnableJNIDebugging()


#define DEBUG_LEVEL_LOW                 1
#define DEBUG_LEVEL_HIGH                2

#ifdef _WIN32
// Disable "empty controlled statement found; is this the intent?" caused by
// the following macro
#pragma warning(disable : 4390) 
#endif

#define _get_jni_nesting_padding(level)\
    (level > 9) ? g_szNestingPadding : g_szNestingPadding+((10-level)*2)

#if DEBUG_METHOD_CALLS
    #if DEBUG_PROFILING
        #define JNI_BEGIN_METHOD(XXX)  \
            UtlString _jni_method_name(XXX) ; \
            bool     _is_callback = false ; \
            if (IsJNIDebuggingEnabled()) \
                osPrintf("%sBEGIN %s\n", _get_jni_nesting_padding(g_iNestingDepth), _jni_method_name.data()) ; \
            OsTimeLog log ; \
            if (IsJNIProfilingEnabled()) \
                log.addEvent(_jni_method_name.data()) ;

        #define JNI_BEGIN_CALLBACK(XXX, YYY)  \
            UtlString _jni_method_name(XXX) ; \
            bool     _is_callback = true ; \
            _jni_method_name.append(" method=") ; \
            _jni_method_name.append(YYY) ; \
            if (IsJNIDebuggingEnabled()) \
                osPrintf("%sBEGIN %s\n", _get_jni_nesting_padding(g_iNestingDepth), _jni_method_name.data()) ; \
            OsTimeLog log ; \
            if (IsJNIProfilingEnabled()) \
                log.addEvent(_jni_method_name.data()) ; \
            g_iNestingDepth++ ;


        #define JNI_END_METHOD()  \
            if ((g_iNestingDepth > 0) && _is_callback) \
                g_iNestingDepth-- ; \
            if (IsJNIDebuggingEnabled()) \
                osPrintf("%sEND   %s\n", _get_jni_nesting_padding(g_iNestingDepth), _jni_method_name.data()) ; \
            if (IsJNIProfilingEnabled()) { \
                log.addEvent(_jni_method_name.data()) ;    \
                log.dumpLog() ; \
            } 
    #else

        #define JNI_BEGIN_METHOD(XXX)  \
            UtlString _jni_method_name(XXX) ; \
            bool     _is_callback = false ; \
            if (IsJNIDebuggingEnabled()) \
                osPrintf("%sBEGIN %s\n", _get_jni_nesting_padding(g_iNestingDepth), _jni_method_name.data()) ;

        #define JNI_BEGIN_CALLBACK(XXX, YYY)  \
            UtlString _jni_method_name(XXX) ; \
            bool     _is_callback = true ; \
            _jni_method_name.append(" method=") ; \
            _jni_method_name.append(YYY) ; \
            if (IsJNIDebuggingEnabled()) \
                osPrintf("%sBEGIN %s\n", _get_jni_nesting_padding(g_iNestingDepth), _jni_method_name.data()) ; \
            g_iNestingDepth++ ;


        #define JNI_END_METHOD()  \
            if ((g_iNestingDepth > 0) && _is_callback) \
                g_iNestingDepth-- ; \
            if (IsJNIDebuggingEnabled()) \
                osPrintf("%sEND   %s\n", _get_jni_nesting_padding(g_iNestingDepth), _jni_method_name.data()) ; 
    #endif



    #define JNI_DUMP_CALL_CALLID(pCall) \
        if (IsJNIDebuggingEnabled() && (GetJNIDebugLevel() >= DEBUG_LEVEL_HIGH)) \
        { \
            char _jni_call_id[80] ; \
            pCall->getCallId(_jni_call_id, sizeof(_jni_call_id)-1) ; \
            osPrintf("%s- Call-Id: %s\n", _get_jni_nesting_padding(g_iNestingDepth), _jni_call_id) ; \
        }
    
    #define JNI_DUMP_CONNECTION_ADDRESS(pConnection) \
        if (IsJNIDebuggingEnabled() && (GetJNIDebugLevel() >= DEBUG_LEVEL_HIGH)) \
        { \
            PtAddress _jni_address ; \
            pConnection->getAddress(_jni_address) ; \
            char _jni_szAddress[80] ; \
            _jni_address.getName(_jni_szAddress, sizeof(_jni_szAddress)) ; \
            osPrintf("%s- Address: %s\n", _get_jni_nesting_padding(g_iNestingDepth), _jni_szAddress) ; \
        }

    #define JNI_DUMP_CAUSECODE(cause) \
        if (IsJNIDebuggingEnabled() && (GetJNIDebugLevel() >= DEBUG_LEVEL_HIGH)) \
        { \
            osPrintf("%s- Cause Code: %d\n", _get_jni_nesting_padding(g_iNestingDepth), cause) ; \
        }

#define JNI_DUMP_TC_NAME(pTC) \
        if (IsJNIDebuggingEnabled() && (GetJNIDebugLevel() >= DEBUG_LEVEL_HIGH)) \
        { \
            UtlString  jni_terminal_name ; \
            UtlBoolean jni_is_Local ; \
            pTC->getTerminalName(jni_terminal_name) ; \
            pTC->isLocal(jni_is_Local) ; \
            osPrintf("%s- Name: %s (%s)\n", _get_jni_nesting_padding(g_iNestingDepth), jni_terminal_name.data(), jni_is_Local ? "local" : "remote") ; \
        }


    //#define JNI_DUMP_ADDRESS(xxx)
    //#define JNI_DUMP_CAUSE(xxx)
#else
    #define JNI_BEGIN_METHOD(name)              // NOP
    #define JNI_BEGIN_CALLBACK(name, method)    // NOP
    #define JNI_END_METHOD()                    // NOP
    #define JNI_DUMP_CALL_CALLID(xxx)           // NOP
    #define JNI_DUMP_CONNECTION_ADDRESS(xxx)    // NOP
    #define JNI_DUMP_CAUSECODE(xxx)             // NOP
    #define JNI_DUMP_TC_NAME(xxx)               // NOP
#endif

extern int  g_iNestingDepth ;
extern const char* g_szNestingPadding ;

extern "C" void EnableJNIDebugging(int iLevel) ;
extern "C" void DisableJNIDebugging() ;
extern "C" bool IsJNIDebuggingEnabled() ;
extern "C" int  SetJNIDebugLevel(int iLevel) ;
extern "C" int  GetJNIDebugLevel() ;
extern "C" void DisplayJNIDebugSettings() ;

extern "C" void EnableJNIProfiling() ;
extern "C" void DisableJNIProfiling() ;
extern "C" bool IsJNIProfilingEnabled() ;

extern "C" void EnableWVDebugging() ;
extern "C" void DisableWVDebugging() ;
extern "C" bool IsWVDebuggingEnabled() ;

#endif
