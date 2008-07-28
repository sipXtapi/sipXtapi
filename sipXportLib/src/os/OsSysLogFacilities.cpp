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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
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
// facilities within the system.  Indexed by enum tagOsSysLogFacility values.
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
   "PARK",
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
   "ODBC",
   "CDR",
   "RLS",
   "VOICEENGINE",
   "VIDEOENGINE",
   "APP",
   "SIP_CUSTOM",
   "SIPXTAPI_EVENT",
   "ARS",
   "NAT",
   "UNKNOWN"
} ;



/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


