// $Id: //depot/OPENDEV/sipXphone/include/licensemanager/opensource/OSLicenseManager.h#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _OSLicenseManager_h_
#define _OSLicenseManager_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsTime.h"
#include "os/OsConfigDb.h"
#include "licensemanager/BaseLicenseManager.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


//:Open Source version of the license manager
class OSLicenseManager : public BaseLicenseManager
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OSLicenseManager();
     //:Default constructor

   virtual
   ~OSLicenseManager();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean setSerialNumber(const UtlString& rSerialNum) ;
     //:Sets the serial number string to the license

/* ============================ ACCESSORS ================================= */

   virtual void getExpirationDate(OsTime& rExpireTime);
     //:Returns the expiration data (seconds since epoch).
     //:May also return infinite.

   virtual UtlBoolean getExpireDateAsStr(UtlString &rExpDate);
     //:Returns the expire date from the key as a string

   virtual UtlBoolean getAdminDomain(UtlString& rDomain);
     //:Gets the AdminDomain (in this case, the admin username)

   virtual UtlBoolean getHostName(UtlString& rHost);
     //:Gets the hostname.

   virtual UtlBoolean getKey(UtlString& rKey);
     //:Gets the encrypted key from the license

   virtual UtlBoolean getLicenseType(UtlString& rLicenseType);
     //:Get the license type.

   virtual int getAppLimit();
     //Returns the maximum number of application that can be loaded on the softphone
     //Returns:
     //  -1 if no applimit or value read from field

   virtual void getSerialNumber(UtlString& rSerialNumber) ;

/* ============================ INQUIRY =================================== */   

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    UtlString mSerialNum;
      //:Serial Number stored in key

    virtual void PopulateFeatures();
    //:Defines what features are available to instant xpressa.    
};

/* ============================ INLINE METHODS ============================ */

#endif  // _OSLicenseManager_h_

