//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

// APPLICATION INCLUDES
#include "os/OsSysLogFacilities.h"
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// sFacilityName provides string based representations of the various 
// facilities within the system.
const char* OsSysLog::sFacilityNames[] =
{
   "PERF",
   "KERNEL",
   "AUTH",
   "NET",
   "RTP",
   "PHONESET",
   "HTTP",
   "SIP",
   "CP",
   "MP",
   "TAO",
   "JNI",
   "JAVA",
   "LOG",
   "WATCHDOG",
   "OUTGOING",
   "INCOMING",
   "INCOMING_PARSED",
   "MEDIASERVER_CGI",
   "MEDIASERVER_VXI",
   "ACD",
   "APACHE_AUTH",
   "UPGRADE",
   "LINE_MGR",
   "REFRESH_MGR",
   "PROCESSCGI",
   "STREAMING",
   "REPLICATION_CGI",
   "SIPDB",
   "PROCESSMGR",
   "PROCESS",
   "SIPXTAPI",
   "AUDIO",
   "CONFERENCE",
   "VOICEENGINE",
   "VIDEOENGINE",
   "UNKNOWN",
} ;



/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


