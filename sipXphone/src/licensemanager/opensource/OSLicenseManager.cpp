// $Id: //depot/OPENDEV/sipXphone/src/licensemanager/opensource/OSLicenseManager.cpp#2 $
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
#include <assert.h>
#include <string.h>
#include <stdlib.h>


// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsTime.h"
#include "os/OsDateTime.h"
#include "licensemanager/opensource/OSLicenseManager.h"
#include "pinger/PingerInfo.h"
#ifdef _WIN32
#include "os/wnt/WindowsAdapterInfo.h"
#elif defined(__linux__)
#include "os/linux/host_address.h"
#else
#error Unsupported target platform.
#endif

// EXTERNAL FUNCTIONS

// EXTERNAL VARIABLES
#ifdef _WIN32
 extern 	AdapterInfoRec adapters[MAX_ADAPTERS];				//used to store all the adapters it finds
#endif
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OSLicenseManager::OSLicenseManager()
{   
#ifdef _WIN32
   if (getAdaptersInfo() > 0)
       mSerialNum = (char *)adapters[0].MacAddress;
   else
	   OsSocket::getHostName(&mSerialNum);
#elif defined(__linux__)
    char hwaddr[18];
    getEthernetHWAddrLinux(hwaddr, sizeof(hwaddr));
    mSerialNum = hwaddr;
#else
#  error Unsupported target platform.
#endif

   PopulateFeatures();

   mIsValid = TRUE ;
}

// Destructor
OSLicenseManager::~OSLicenseManager()
{
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean OSLicenseManager::setSerialNumber(const UtlString& rSerialNum)
{
	mSerialNum = rSerialNum ;

	return TRUE ;
}

/* ============================ ACCESSORS ================================= */


//:Returns the expiration data (seconds since epoch).
//:May also return infinite.
void OSLicenseManager::getExpirationDate(OsTime& rExpireTime)
{
   rExpireTime = OsTime::OS_INFINITY;
}

UtlBoolean OSLicenseManager::getAdminDomain(UtlString& rDomain)
{
   return FALSE ;
}

UtlBoolean OSLicenseManager::getHostName(UtlString& rHost)
{
   return FALSE ;
}

UtlBoolean OSLicenseManager::getKey(UtlString& rKey)
{
   return FALSE ;   
}

void OSLicenseManager::getSerialNumber(UtlString& rSerialNumber) 
{
   rSerialNumber = mSerialNum;
}

UtlBoolean OSLicenseManager::getLicenseType(UtlString& rLicenseType)
{
   return FALSE ;
}


UtlBoolean OSLicenseManager::getExpireDateAsStr(UtlString &rExpDate)
{
   return FALSE ;   
}


//returns the maximum number of application that can be loaded on the softphone
// returns:
//     -1 if no applimit
//   or value read from field
int OSLicenseManager::getAppLimit()
{
   return -1 ;
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void OSLicenseManager::PopulateFeatures()
{
   BaseLicenseManager::PopulateFeatures();

   // Assume this code is license for everything -- if the codes are
   // not physically available, the codec factory should do the right 
   // thing.
   mpFeatures->set("COMMERCIAL","");
   mpFeatures->set("GIPS-G711","");
   mpFeatures->set("GIPS-IPCM","");
   mpFeatures->set("APPLIMIT-NONE","");
   mpFeatures->set("EG711U","");
   mpFeatures->set("IPCMU","");
   mpFeatures->set("260","");
   mpFeatures->set("EG711A","");
   mpFeatures->set("IPCMA","");
   mpFeatures->set("259","");
   mpFeatures->set("G729","");
   mpFeatures->set("G729A","");
   mpFeatures->set("G729B","");
   mpFeatures->set("G729AB","");
   mpFeatures->set("G729A-FOR-CISCO-7960","");
   mpFeatures->set("ILBC","");
}

/* ============================ FUNCTIONS ================================= */


