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

// APPLICATION INCLUDES
#include <cp/CpGatewayManager.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpGatewayManager::CpGatewayManager()
{
}

// Copy constructor
CpGatewayManager::CpGatewayManager(const CpGatewayManager& rCpGatewayManager)
{
}

// Destructor
CpGatewayManager::~CpGatewayManager()
{
}

/* ============================ MANIPULATORS ============================== */

CpGatewayManager* CpGatewayManager::getCpGatewayManager()
{
        return NULL;
}

OsStatus CpGatewayManager::setGatewayInterface(const char* terminalName,
       PtGatewayInterface* pInterface)
{
        return OS_UNSPECIFIED;
}

void CpGatewayManager::getNewCallId(UtlString& callId)
{
}

OsStatus CpGatewayManager::connect(const char* callId,
                    const char* fromAddressUrl,
                    const char* fromTerminalName,
                    const char* toAddressUrl)
{
        return OS_UNSPECIFIED;
}

OsStatus CpGatewayManager::answer(const char* callId, const char* terminalName)
{
        return OS_UNSPECIFIED;
}


OsStatus CpGatewayManager::disconnectConnection(const char* callId,
                                 const char* connectionAddressUrl)
{
        return OS_UNSPECIFIED;
}

// Assignment operator
CpGatewayManager&
CpGatewayManager::operator=(const CpGatewayManager& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */

UtlBoolean CpGatewayManager::gatewayInterfaceExists(const char* terminalName)
{
        return FALSE;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
