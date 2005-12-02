// $Id: //depot/OPENDEV/sipXphone/include/pinger/PingerInfo.h#4 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _PingerInfo_h_
#define _PingerInfo_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "net/SipMessage.h"
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsConfigDb.h"
#include "pinger/secrets.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS

extern "C" int GetCallTaskCount(void);

// EXTERNAL VARIABLES
// CONSTANTS
#define CONFIG_GET_CONFIGFILE     "/getConfig" 
#define CONFIG_GET_LOGICALID      "/getLogicalId" 
#define CONFIG_DEPLOYMENT_SERVER  "pds.pingtel.net" 
#define CONFIG_DEPLOYMENT_PATH    "/deployment"
#define CONFIG_KEY_FILE_NAME      "key-config"
#define MY_PINGTEL_SERVER         "my.pingtel.com"
#define CONFIG_GET_DEVICEID       "/getDeviceId" 

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
//:TODO: condier lock issue to avoid set/get problem. 
class PingerInfo
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum KeyConfigError
   {
      ERR_KEYCONFIG_OK,           // keys match   
      ERR_KEYCONFIG_NOFILE,       // can not find key-config file
      ERR_KEYCONFIG_KEYNOTMATCH,  // key in the key-config file does not match key digest
      ERR_KEYCONFIG_HOSTNOTMATCH, // host name in the key-config file does not match actual host
      ERR_KEYCONFIG_EXPIRED       // date in key-config file expired.
   };

   enum NetworkInterfaceDuplex
   {
       NETWORK_INTERFACE_AUTO_DUPLEX = 0, // Auto sense
       NETWORK_INTERFACE_HALF_DUPLEX = 1, // fixed half duplex
       NETWORK_INTERFACE_FULL_DUPLEX = 2  // fixed full duplex
   };

   enum HandsetEQSetting
   {
       HANDSET_EQ_UNKNOWN   = -1,
       HANDSET_EQ_REV_B     = 0,
       HANDSET_EQ_REV_C     = 1,
       HANDSET_EQ_HEADSET   = 2 
   };

   enum HookSwitchControlSetting
   {
       HOOKSWITCH_CONTROL_UNKNOWN = -1,
       HOOKSWITCH_CONTROL_NORMAL = 0,       // hook switch control is normal
       HOOKSWITCH_CONTROL_HEADSET           // hookswitch is controlled by
                                            // the headset switch
   };
/* ============================ CREATORS ================================== */

   PingerInfo();
     //:Default constructor

   virtual
   ~PingerInfo();
     //:Destructor

/* ============================ MANIPULATORS ============================== */


   static void setContactedTimeServer(UtlBoolean bContactedTimeServer) ;
   //: Set the indicator that we contacted a time server


   UtlBoolean restoreDefaults() ;
   // Formats/flash and clears user settings

   static void digest(const char* text, UtlString& encodedText);

   static void setupGetLogicalIdUrl(UtlString& url,
                                    const char* adminDomain, 
                                    const char* adminId,
                                    const char* adminPwd,
                                    const char* secret = CONFIG_SECRET,
                                    const char* logicalId = NULL);
   // url string returns the url path for getting logical id from deployment server

   void PingerInfo::setupGetDeviceIdMyPingtelUrl(UtlString& url,
                                    const char* adminDomain, 
                                    const char* adminPwd,
                                    const char* secret,
                                    const char* logicalId,
                                    OsConfigDb* config) ;


   void setupGetConfigMyPingtelUrl(UtlString& url,
                                    const char* adminDomain, 
                                    const char* logicalId,
                                    const char* secret = CONFIG_SECRET);
   // url strings returns url path for getting config file from my.pingtel site

   static void setupGetConfigUrl(UtlString& url,
                                    const char* adminDomain, 
                                    const char* logicalId,
                                    const char* secret = CONFIG_SECRET);
   // url string returns the url path for getting config file from deployment server

   static OsStatus setHandsetEQSetting(UtlString &rStrHandsetEQSetting);
   //: set the handset EQ setting
   //  returns OS_INVALID if invalid string specified in pinger-config
   //  returns OS_SUCCESS if handset type set correctly.


/* ============================ ACCESSORS ================================= */

   void getConfigDnsIp(int dnsServerIndex, UtlString& dnsIpAddress);
   // get the config dns ip 


   static void getCurrentIPAddress(UtlString &ipAddress, UtlBoolean& isDHCP);
   //returns ip address and whether on dhcp or not

   static unsigned long getDefaultBindIPAddress();
   // returns the default ip address for the phone (in ulong format)
   // if there is only one adapter in the system it will return IPANY_ADDR
   // if softphone, it will get the config param  and look up the ip address
   // for that device.  NOTE: This may return an empty string, in which case
   // the ip address is unknown. 
   
   void getSipVersion(UtlString& version);
   // get the sip protocol version. 

   static void getRootDirectory(UtlString& rootDirectory);
   // Get the directory in which pinger stuff is stored

   /*static void getDeviceConfigFileNamePath(UtlString& configFileNamePath);
   // Get the path and filename of the device configuration file

   static void getUserConfigFileNamePath(UtlString& configFileNamePath);
   // Get the path and filename of the user configuration file*/

   static void getFlashFileSystemLocation(UtlString& location);

   //returns the current status of the http server
   static OsStatus getHttpServerStatus();

   static OsStatus getDnsStatus(void);
   // Returns OS_SUCCESS: up and running, or OS_DNS_UNAVAILABLE: busted


   static void getVersion(UtlString& version);
   //: Get the kernel version
   //!param: (out) version - The kernel version.

   static void getVersionComment(UtlString& comment);
   //:Get the kernel version comment for this product.   The version comment
   // is often used to identify a beta or pre-release version.
   //!param: (out) comment - The version comment for this product.

   static void getBuildDate(UtlString& buildDate);
   //:Get the date/time that this product was physically compiled/linked.
   //!param: (out) buildDate - The date/time the product was built.

   static void getPlatformType(UtlString& platformType);
   //:Get the platform type for this product.  The platform type is in the 
   // form: <product code>_<processor family>_<operating system).  Example:
   // "xpressa_strongarm_vxworks".
   //!param: (out) platformType - The platform type of this product.

   static void getUptime(UtlString& uptime);
   //: Get the uptime (time since boot)

   static void getMacAddress(UtlString& macAddress);
   // get the Mac address.
   static int getKeyConfig(const char* fileName);

   static void getSerialNum(UtlString& serial);
   // get the serial number for this product

   static HandsetEQSetting getHandsetEQSetting();
   //: Get the handset EQ setting
   //  Possible return values:
   //       HANDSET_EQ_REV_B 
   //       HANDSET_EQ_REV_C 
   //       HANDSET_EQ_HEADSET

   static HookSwitchControlSetting getHookSwitchControl() ;
   //: Determines how the hook switch is controlled.
   //  Can be HOOKSWITCH_CONTROL_NORMAL or HOOKSWITCH_CONTROL_HEADSET
   //  Default is HOOKSWITCH_CONTROL_NORMAL.

/* ============================ INQUIRY =================================== */

   static int getTotalOutgoingCalls();

   static int getTotalIncomingCalls();


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static HandsetEQSetting mHandsetEQSetting;
   static HookSwitchControlSetting mHookSwitchControlSetting ;
   static UtlBoolean mContactedTimeServer; 

   PingerInfo(const PingerInfo& rPingerInfo);
   //:Copy constructor, not implemented yet
   
   PingerInfo& operator=(const PingerInfo& rhs);
   //:Assignment operator, not implemented yet
   
   void killAFile(UtlString dir, const char* name);


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PingerInfo_h_
