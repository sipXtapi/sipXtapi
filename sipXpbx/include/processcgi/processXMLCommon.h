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
#ifndef _processXMLCommon_
#define _processXMLCommon_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <signal.h>


// APPLICATION INCLUDES
#include "xmlparser/tinyxml.h"
#include "os/OsNameDb.h"
#include "os/OsDefs.h"
#include "os/OsProcess.h"
#include "os/OsProcessMgr.h"
#include "os/OsProcessIterator.h"
#include "processcgi/DependencyList.h"

// DEFINES
#define ACTION_START    "start"
#define ACTION_STOP     "stop"
#define ACTION_RESTART  "restart"
#define ACTION_STATUS   "status"
#define ACTION_VERIFY   "verify"

OsStatus initProcessXMLLayer(UtlString &rProcessXMLPath, TiXmlDocument &rProcessXMLDoc, UtlString &rStrErrorMsg);
OsStatus startstopProcessTree(TiXmlDocument &rProcessXMLDoc, UtlString &rProcessAlias, UtlString &rActionVerb);
OsStatus WriteProcessXML(TiXmlDocument &doc, UtlString &buffer);
OsStatus VerifyProcess(UtlString &rAlias);

#endif //_processXMLCommon__
