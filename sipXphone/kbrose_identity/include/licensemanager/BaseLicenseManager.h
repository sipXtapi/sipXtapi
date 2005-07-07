// $Id: //depot/OPENDEV/sipXphone/include/licensemanager/BaseLicenseManager.h#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _BaseLicenseManager_h_
#define _BaseLicenseManager_h_

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsConfigDb.h"
#include "net/NetMd5Codec.h"

// DEFINES

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsTime;

//:Base class for all License Managers.  Each platform should derive from this
//:and implement the policy for retrieving and verifying the correct license
//:info.
class BaseLicenseManager
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

   BaseLicenseManager();
     //:Default constructor

   virtual
   ~BaseLicenseManager();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean setSerialNumber(const UtlString& rSerialNumber);
     //: Sets the serial number for the license if supported on the
     //: platform.

   virtual void PopulateFeatures();
     //:Defines the features that are common to all license managers.

/* ============================ ACCESSORS ================================= */
   static BaseLicenseManager* getInstance();
   //: Retrieves the current instance of the license manager.  This method 
   //  is *NOT* thread safe.  If you believe this can be called from 
   //  multiple thread contexts, please add the appropriate locks.

   virtual UtlBoolean getFeatureValue(const UtlString& rFeaturePrefix,
                                      UtlString& rFeatureValue,
                                      int nFeatureIndex = 0);
     //:Retrieves one (specified by index) of the current values specified
     //:by the feature string prefix. (EG. "gips-" with index of 1, may
     //:return "gips-g711, then index 2 returns "gips-ipcm".

   virtual void getExpirationDate(OsTime& rExpireTime);
     //:Returns the expiration data (seconds since epoch).
     //:May also return infinite.

   static int getFailureCode() {return mFailureCode;}
     //:Returns the specific code for the failure case
     //:See derived class for specifics

   void getFeatures(UtlString& rFeatures);
     //:Returns a feature string.  This string may contain more than the actual
     //:key-config file.  The key-config might only contain commercial, or personal.

   virtual void getSerialNumber(UtlString& rSerialNumber) = 0 ;

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

/* ============================ INQUIRY =================================== */
   virtual UtlBoolean isLicensed(const UtlString& rFeatureStr);
     //:Returns TRUE if this platform is licensed for a specific feature

   UtlBoolean isValid();
     //:Returns TRUE if the loaded license was valid
     //:You should call this before calling any other methods on this class

/* //////////////////////////// PROTECTED ///////////////////////////////// */   

protected:

	void digest(const char* pText, UtlString& rEncodedText);

    OsConfigDb *mpFeatures;
      //:This is the place to store each feature.
      //:Key is the prefix.  If you were looking for gips

    OsTime mExpireDateInSecs;
      //:This is the place to store date (in secs) of license timeout.

    UtlBoolean mIsValid;
      //:TRUE if the license loaded is valid

    static int mFailureCode;
      //:If mIsValid is false, this will contain the failure code.
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    static BaseLicenseManager *mpLicenseManager;
      //:This stores the instance of the license manager the user is using for the platform.
      //:It it set when the platform creates and instance of their LicenseManager, then
      //:call setInstance.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _BaseLicenseManager_h_

