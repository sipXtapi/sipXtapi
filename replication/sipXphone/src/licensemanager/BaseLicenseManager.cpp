// $Id: //depot/OPENDEV/sipXphone/src/licensemanager/BaseLicenseManager.cpp#5 $
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

// APPLICATION INCLUDES
#include "licensemanager/BaseLicenseManager.h"
#ifdef PINGTEL_COMMERICAL
#  include "licensemanager/ixpressa/IxLicenseManager.h"
#  include "licensemanager/xpressa/XpressaLicenseManager.h"
#else
#  include "licensemanager/opensource/OSLicenseManager.h"
#endif


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
BaseLicenseManager* BaseLicenseManager::mpLicenseManager = NULL;
int BaseLicenseManager::mFailureCode = 0;

 
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
BaseLicenseManager::BaseLicenseManager()
{
    mExpireDateInSecs = OsTime::OS_INFINITY;
    mIsValid = FALSE;
    mpFeatures = new OsConfigDb(); 
}


// Destructor
BaseLicenseManager::~BaseLicenseManager()
{
    if (mpFeatures)
        delete mpFeatures;
}


// Retrieves the current instance of the license manager.  This method 
// is *NOT* thread safe.  If you believe this can be called from 
// multiple thread contexts, please add the appropriate locks.
BaseLicenseManager * BaseLicenseManager::getInstance()
{    
    if (mpLicenseManager == NULL)
    {
#ifdef PINGTEL_COMMERICAL
		mpLicenseManager = new IxLicenseManager();
#else
        mpLicenseManager = new OSLicenseManager();
#endif
	}

    return mpLicenseManager;
}


/* ============================ MANIPULATORS ============================== */


// Sets the serial number for the license if supported on the platform.
UtlBoolean BaseLicenseManager::setSerialNumber(const UtlString& rSerialNumber)
{
	return FALSE ;
}


void BaseLicenseManager::PopulateFeatures()
{
    
   // All codecs listed below will be supported on all platforms.
   // Meaning, add codecs that are common to ALL platforms here.  
   // If you have a feature tied to a specific platform, at it
   // to that LicenseManager-- not this.
   mpFeatures->set("PCMU","");
   mpFeatures->set("G711U","");
   mpFeatures->set("PCMA","");           
   mpFeatures->set("G711A","");          
   mpFeatures->set("GIPS-G711U","");     
   mpFeatures->set("GIPS-G711A","");     
   mpFeatures->set("TELEPHONE-EVENT","");
   mpFeatures->set("0","");
   mpFeatures->set("8","");
   mpFeatures->set("258","");
   mpFeatures->set("257","");
   mpFeatures->set("128","");
}

/* ============================ ACCESSORS ================================= */
  //:Retrieves one (specified by index) of the current values specified
  //:by the feature string prefix. (EG. "gips-" with index of 1, may
  //:return "gips-g711, then index 2 returns "gips-ipcm".
UtlBoolean BaseLicenseManager::getFeatureValue(const UtlString& rFeaturePrefix,
                                              UtlString& rFeatureValue,
                                              int nFeatureIndex)
{
    int foundCount = 0;
    UtlString keyLast;
    UtlString keyNext;
    UtlString valueNext;
    UtlString featurePrefix(rFeaturePrefix);

    featurePrefix.toUpper();
    keyLast = "";
    
    while (mpFeatures->getNext(keyLast, keyNext, valueNext) == OS_SUCCESS)
    {
        if (keyNext.index(featurePrefix) == 0)
        {
            foundCount++;
            if (foundCount == nFeatureIndex+1)
                break;
        }

        keyLast = keyNext ;
    }

    if (foundCount == nFeatureIndex+1)
    {
        rFeatureValue = keyNext;
    }
    
    return (foundCount == nFeatureIndex+1);
}

//:Returns the expiration data (seconds since epoch).
//:May also return infinite.
void BaseLicenseManager::getExpirationDate(OsTime& rExpireTime)
{
    rExpireTime = mExpireDateInSecs;
}

void BaseLicenseManager::getFeatures(UtlString& rFeatures)
{
    UtlString keyLast = "";
    UtlString keyNext;
    UtlString valueNext;

    while (mpFeatures->getNext(keyLast, keyNext, valueNext) == OS_SUCCESS)
    {
        if (rFeatures.length())
            rFeatures.append(" ");
        rFeatures.append(keyNext);

        keyLast = keyNext ;
    }
}

// Returns the expire date from the key as a string
UtlBoolean BaseLicenseManager::getExpireDateAsStr(UtlString &rExpDate)
{
   rExpDate = "Never" ;
   return FALSE ;
}
     

// Gets the AdminDomain (in this case, the admin username)
UtlBoolean BaseLicenseManager::getAdminDomain(UtlString& rDomain)
{
   rDomain = "" ;
   return FALSE ;
}

// Gets the hostname.
UtlBoolean BaseLicenseManager::getHostName(UtlString& rHost)
{
   rHost = "" ;
   return FALSE ;
}
     

// Gets the encrypted key from the license
UtlBoolean BaseLicenseManager::getKey(UtlString& rKey)
{
   rKey = "" ;
   return FALSE ;
}
     
    
// Returns the maximum number of application that can be loaded on the softphone
int BaseLicenseManager::getAppLimit()
{
    return -1 ;
}

// Get the license type.
UtlBoolean BaseLicenseManager::getLicenseType(UtlString& rLicenseType)
{
   rLicenseType = "" ;
   return FALSE ;
}

 
/* ============================ INQUIRY =================================== */
 //:Returns TRUE if this platform is licensed for a specific feature
UtlBoolean BaseLicenseManager::isLicensed(const UtlString& rFeatureStr)
{
    UtlString tmpVal;
    UtlString tmpKey(rFeatureStr);

    tmpKey.toUpper();

    return (mpFeatures->get(tmpKey,tmpVal) == OS_SUCCESS);
}

//:Returns TRUE if license is valid
UtlBoolean BaseLicenseManager::isValid()
{
    return mIsValid;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void BaseLicenseManager::digest(const char* text, UtlString& encodedText)
{
   NetMd5Codec::encode(text, encodedText);
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */


/* ============================ FUNCTIONS ================================= */


