// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
// USA. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _PtDefs_h_
#define _PtDefs_h_

// SYSTEM INCLUDES
#include <string.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"

// DEFINES
//#define PTAPI_TEST

// MACROS

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#ifdef LONG_EVENT_RESPONSE_TIMEOUTS
#  define PT_CONST_EVENT_WAIT_TIMEOUT   2592000   // 30 days in seconds
#else
#  define PT_CONST_EVENT_WAIT_TIMEOUT    40               // time out, seconds
#endif

// STRUCTS
// TYPEDEFS
#undef PTAPI_DEBUG_TRACE

#ifdef PTAPI_DEBUG_TRACE
#define EVENT_TRACE(x) osPrintf(x)
#else
#define EVENT_TRACE(x) //* osPrintf(x) *//
#endif

// FORWARD DECLARATIONS

//:Status codes returned by Pingtel API methods.

enum PtStatus
{
   PT_SUCCESS,
   PT_AUTH_FAILED,
   PT_FAILED,
   PT_EXISTS,
   PT_HOST_NOT_FOUND,
   PT_IN_PROGRESS,
   PT_INVALID_ARGUMENT,
   PT_INVALID_PARTY,
   PT_INVALID_STATE,
   PT_INVALID_IP_ADDRESS,
   PT_INVALID_SIP_DIRECTORY_SERVER,
   PT_INVALID_SIP_URL,
   PT_MORE_DATA,
   PT_NO_MORE_DATA,
   PT_NOT_FOUND,
   PT_PROVIDER_UNAVAILABLE,
   PT_RESOURCE_UNAVAILABLE,
   PT_BUSY
};

class PtTerminalConnection
{
public:
   enum TerminalConnectionState
   {
                IDLE    = 0x60,
                RINGING = 0x61,
                TALKING = 0x62,
                HELD    = 0x63,
                BRIDGED = 0x64,
                IN_USE  = 0x65,
                DROPPED = 0x66,
                UNKNOWN = 0x67
   };
} ;

class PtCall
{
public:
   enum CallState
   {
      IDLE,
      ACTIVE,
      INVALID
   };
};

class PtConnection
{
public:
   enum ConnectionState
   {
      IDLE                = 0x50,
      OFFERED             = 0x51,
      QUEUED              = 0x52,
      ALERTING            = 0x53,
      INITIATED           = 0x54,
      DIALING             = 0x55,
      NETWORK_REACHED     = 0x56,
      NETWORK_ALERTING    = 0x57,
      ESTABLISHED         = 0x58,
      DISCONNECTED        = 0x59,
      FAILED              = 0x5A,
      UNKNOWN             = 0x5B
   };
};

typedef int PtBoolean;

#define PT_CLASS_INFO_MEMBERS static const char* sClassName;
#define PT_NO_PARENT_CLASS
#define PT_IMPLEMENT_CLASS_INFO(CHILD, PARENT) \
const char* CHILD::sClassName = #CHILD; \
PtBoolean isInstanceOf(const char* name); \
\
const char* CHILD::className() { return(sClassName);} \
\
PtBoolean CHILD::isClass(const char* name) { return(strcmp(name, className()) == 0);} \
\
PtBoolean CHILD::isInstanceOf(const char* name) \
{\
    PtBoolean isInstance = isClass(name); \
    if(!isInstance) isInstance = PARENT::isInstanceOf(name); \
    return(isInstance); \
}

/* ============================ INLINE METHODS ============================ */

#endif  // _PtDefs_h_
