// $Id: //depot/OPENDEV/sipXphone/src/pinger/PingerInfo.cpp#5 $
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
#include "cp/CallManager.h"
#include "cp/CpCall.h"
#include "pinger/PingerInfo.h"
#include "net/NetMd5Codec.h"
#include "os/OsConfigDb.h"
#include "os/OsDateTime.h"
#include "os/OsDefs.h"
#include "os/OsUtil.h"
#include "os/OsFS.h"
#include "pinger/Pinger.h"
#include "pinger/version.h"
#include "licensemanager/BaseLicenseManager.h"
#include "os/OsSysLog.h"
#include "config.h"

#ifdef WIN32
#include <direct.h>
#include <winsock.h>
#endif

#ifdef __pingtel_on_posix__
#include <unistd.h>
#include <stdlib.h>
#endif

// EXTERNAL FUNCTIONS


// EXTERNAL VARIABLES


// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

// OsStatus   PingerInfo::mNetworkStatus = OS_SUCCESS;
// OsStatus   PingerInfo::mDnsServiceStatus = OS_SUCCESS;
UtlBoolean PingerInfo::mContactedTimeServer = FALSE;
PingerInfo::HandsetEQSetting PingerInfo::mHandsetEQSetting = HANDSET_EQ_UNKNOWN;
PingerInfo::HookSwitchControlSetting PingerInfo::mHookSwitchControlSetting = HOOKSWITCH_CONTROL_UNKNOWN;
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PingerInfo::PingerInfo()
{

}

// Destructor
PingerInfo::~PingerInfo()
{
}

/* ============================ MANIPULATORS ============================== */

void PingerInfo::digest(const char* text, UtlString& encodedText)
{
   NetMd5Codec::encode(text, encodedText);
}


void PingerInfo::setupGetLogicalIdUrl(UtlString& url,
                                      const char* adminDomain,
                                      const char* adminId,
                                      const char* adminPwd,
                                      const char* secret,
                                      const char* logicalId)
{
   UtlString text;
   UtlString encoded;

   encoded.remove(0);
   text = secret + UtlString(":") + UtlString(adminId) + UtlString(":") + UtlString(adminPwd);
   NetMd5Codec::encode(text.data(), encoded);

   url = CONFIG_GET_LOGICALID + UtlString("?") +
         "ADMIN_DOMAIN=" + adminDomain + UtlString("&") +
         "ADMIN_ID=" + adminId + UtlString("&") +
         "ADMIN_PWD=" + encoded + UtlString("&");

   UtlString macAddress;
   getMacAddress(macAddress);
   text = secret + UtlString(":") + UtlString(macAddress) ;
   encoded.remove(0);
   NetMd5Codec::encode(text.data(), encoded);

   UtlString kernelVersion;
   getVersion(kernelVersion);
   HttpMessage::escape(kernelVersion);

   url += "PHYSICAL_ID=" + macAddress + UtlString("&") +
          "PHYSICAL_PWD=" + encoded + UtlString("&") +
          "KERNEL_VERSION=" + UtlString(kernelVersion);
   if (logicalId)
      url += UtlString("&LOGICAL_ID=") + UtlString(logicalId);

   osPrintf("PingerInfo::setupGetLogicalIdUrl: %s\n", url.data());
}

void PingerInfo::setupGetConfigUrl(UtlString& url,
                  const char* adminDomain,
                  const char* logicalId,
                  const char* secret)
{
   UtlString text;
   UtlString encoded;
   encoded.remove(0);

   UtlString macAddress;
   getMacAddress(macAddress);

   text = secret + UtlString(":") + UtlString(macAddress) +
          UtlString(":") + UtlString(logicalId);
   NetMd5Codec::encode(text.data(), encoded);

   UtlString kernelVersion;
   getVersion(kernelVersion);
   HttpMessage::escape(kernelVersion);

   url = CONFIG_GET_CONFIGFILE + UtlString("?") +
            "ADMIN_DOMAIN=" + adminDomain + UtlString("&") +
            "LOGICAL_ID=" + logicalId + UtlString("&") +
            "LOGICAL_PWD=" + encoded + UtlString("&") +
            "PHYSICAL_ID=" + macAddress + UtlString("&") +
            "KERNEL_VERSION=" + kernelVersion;
   osPrintf("PingerInfo::setupGetConfigUrl: %s\n", url.data());

}

void PingerInfo::setupGetConfigMyPingtelUrl(UtlString& url,
                  const char* adminDomain,
                  const char* logicalId,
                  const char* secret)
{
   UtlString text;
   UtlString encoded;
   encoded.remove(0);

   UtlString macAddress;
   getMacAddress(macAddress);

   text = secret + UtlString(":") + UtlString(macAddress) +
          UtlString(":") + UtlString(logicalId);
   NetMd5Codec::encode(text.data(), encoded);

   UtlString kernelVersion;
   getVersion(kernelVersion);
   HttpMessage::escape(kernelVersion);

   url = CONFIG_GET_CONFIGFILE + UtlString("?") +
            "DEVICE_ID=" + logicalId + UtlString("&") +
            "ADMIN_NAME=" + adminDomain + UtlString("&") +
            "DEVICE_PWD=" + encoded + UtlString("&") +
            "MAC_ADDRESS=" + macAddress + UtlString("&") +
            "KERNEL_VERSION=" + kernelVersion;
   osPrintf("PingerInfo::setupConfigGetMyPingtelUrl: %s\n", url.data());

}

void PingerInfo::setContactedTimeServer(UtlBoolean bContactedTimeServer)
{
    mContactedTimeServer = bContactedTimeServer;
}




/* ============================ ACCESSORS ================================= */

void PingerInfo::getCurrentIPAddress(UtlString &ipAddress, UtlBoolean& isDHCP)
{
   OsSocket::getHostIp(&ipAddress) ;
   isDHCP = TRUE ;
}


void PingerInfo::getFlashFileSystemLocation(UtlString& location)
{
#ifdef WIN32
   location = CONFIG_PREFIX_USER_DATA ;
#else
   location = getenv("HOME") ; 
   location.append("/.sipxphone") ;
#endif
}

OsStatus PingerInfo::getHttpServerStatus()
{
   OsStatus retval = OS_UNSPECIFIED;
   Pinger *pPinger = NULL;

   pPinger = Pinger::getPingerTask();

   if (pPinger)
      pPinger->getHttpStatus();

   return retval;
}

// get SIP protocol version
void PingerInfo::getSipVersion(UtlString& version)
{
    version.remove(0);
    version.append(SIP_PROTOCOL_VERSION);
}

void PingerInfo::getVersion(UtlString& version)
{
    version.remove(0) ;
    version.append(KERNEL_VERSION) ;
}

void PingerInfo::getVersionComment(UtlString& comment)
{
    comment.remove(0) ;
    comment.append(KERNEL_VERSION_COMMENT) ;   
}

void PingerInfo::getPlatformType(UtlString& platformType)
{
#if defined(WIN32)
    platformType = "ixpressa_x86_win32" ;
#elif defined(__linux__)
    platformType = "ixpressa_x86_linux" ;
#elif defined(sun)
    platformType = "ixpressa_sparc_solaris" ;
#else
    platformType = "unknown" ;
#endif
}


// Get the time since boot in the format: "x day(s) hh:mm:ss"
#define MIN_IN_SECS  (60)
#define HOUR_IN_SECS ((MIN_IN_SECS)*60)
#define DAY_IN_SECS  ((HOUR_IN_SECS)*24)
void PingerInfo::getUptime(UtlString& uptime)
{
   OsTime timeSinceBoot ;
   OsDateTime::getCurTimeSinceBoot(timeSinceBoot) ;

   // Calculate days, hours, minutes, and seconds
   long lSeconds = timeSinceBoot.seconds() ;

   long lDays = lSeconds / DAY_IN_SECS ;
   lSeconds %= DAY_IN_SECS ;
   long lHours = lSeconds / HOUR_IN_SECS ;
   lSeconds %= HOUR_IN_SECS ;
   long lMinutes = lSeconds / MIN_IN_SECS ;
   lSeconds %= MIN_IN_SECS ;

   // Build up result string
   char szTmpString[128] ;
   sprintf(szTmpString, "%ld day%s %02ld:%02ld:%02ld",
         lDays,
         (lDays == 1) ? "" : "s",
         lHours,
         lMinutes,
         lSeconds) ;

   uptime = szTmpString ;
}


// returned the number of call tasks currently being tracked by 
// the lower layer
int GetCallTaskCount()
{
   return CpCall::getCallTrackingListCount() ;
}

void PingerInfo::killAFile(UtlString dir, const char* name)
{
   UtlString killFile(dir);
   killFile.append(name);
   if (remove(killFile.data()) == -1)
   {
         // Add to syslog, and flush everything we can before proceeding.
       OsSysLog::add(FAC_KERNEL, PRI_ERR, "Unable to delete file: %s from directory %s",
                name,dir.data());
       OsSysLog::flush();
   }
}

UtlBoolean PingerInfo::restoreDefaults()
{
   UtlBoolean bRC = true ;
   UtlString  killFile ;
   UtlString  flashLocation ;

   // Remove files.  This is only reached if we we aren't formatting the
   // files system or if the format failed.

   PingerInfo::getFlashFileSystemLocation(flashLocation);
   flashLocation.append(OsPath::separator);

   killAFile(flashLocation, "cache.ser") ;
   killAFile(flashLocation, "config.html") ;
   killAFile(flashLocation, "kernel.aif.install") ;

   // DWW bug 757 says it should not do this
   // killAFile(flashLocation, "key-config") ;

   killAFile(flashLocation, "pinger-config") ;
   killAFile(flashLocation, "app-config") ;
   killAFile(flashLocation, "config-config") ;
   killAFile(flashLocation, "pingtel.jar.install") ;
   killAFile(flashLocation, "tmpSoundFile.raw") ;
   killAFile(flashLocation, "user-config") ;
   killAFile(flashLocation, "pinger-config_incoming");
   killAFile(flashLocation, "user-config_incoming");
   killAFile(flashLocation, "app-config_incoming");

   //delete the local-config file which stores the rstart count
   killAFile(flashLocation, "local-config") ;

   // Delete known distinctive rings files
   killAFile(flashLocation, "dr_rings/ring1.raw") ;
   killAFile(flashLocation, "dr_rings/ring2.raw") ;
   killAFile(flashLocation, "dr_rings/ring3.raw") ;
   killAFile(flashLocation, "dr_rings/ring4.raw") ;
   killAFile(flashLocation, "dr_rings/ring5.raw") ;

   killFile = flashLocation ;
   killFile.append("dr_rings") ;
#ifdef WIN32
   _rmdir((char*)killFile.data()) ;
#else
   rmdir((char*)killFile.data()) ;
#endif

   //next block of code will kill upgrade-log if it is over 4k
   UtlString logFilename = flashLocation;
   logFilename.append("upgrade-log");
   OsFile file(logFilename);

   if (file.exists())
   {
       OsFileInfo info;
       file.getFileInfo(info);
       unsigned long size;
       info.getSize(size);
       
       if (size > 4096)
            killAFile(flashLocation, "upgrade-log") ;

   }


   return bRC ;
}


void PingerInfo::getMacAddress(UtlString& macAddress)
{
	getSerialNum(macAddress) ;
}


void PingerInfo::getSerialNum(UtlString& serial)
{
   BaseLicenseManager* pLM = BaseLicenseManager::getInstance() ;
   assert(pLM != NULL) ;
   if (pLM != NULL)
   {
	   pLM->getSerialNumber(serial) ;	
   }
}


OsStatus PingerInfo::getDnsStatus()
{
   return OS_SUCCESS ;
}


void PingerInfo::getRootDirectory(UtlString& rootDirectory)
{
   rootDirectory = CONFIG_PREFIX_USER_DATA ;
}

// Get the path and filename of the device configuration file
/*void PingerInfo::getDeviceConfigFileNamePath(UtlString& configFileNamePath)
{
   getRootDirectory(configFileNamePath);
   configFileNamePath.append("/");
   configFileNamePath.append(DEVICE_CONFIGDB_NAME);
}

// Get the path and filename of the user configuration file
void PingerInfo::getUserConfigFileNamePath(UtlString& configFileNamePath)
{
   getRootDirectory(configFileNamePath);
   configFileNamePath.append("/");
   configFileNamePath.append(USER_CONFIGDB_NAME);
}*/


OsStatus PingerInfo::setHandsetEQSetting(UtlString &rStrHandsetEQSetting)
{
    OsStatus retval = OS_INVALID;

    if (rStrHandsetEQSetting == "REVB")
        mHandsetEQSetting = HANDSET_EQ_REV_B;
    else
    if (rStrHandsetEQSetting == "REVC")
        mHandsetEQSetting = HANDSET_EQ_REV_C;
    else
    if (rStrHandsetEQSetting == "HEADSET")
        mHandsetEQSetting = HANDSET_EQ_HEADSET;
    else
    {
        osPrintf("\n\nUnknown handset EQ setting specified!\n\n");
        mHandsetEQSetting = HANDSET_EQ_UNKNOWN; 
        retval = OS_INVALID; //unknown type
    }

    return retval;
}

/* ============================ INQUIRY =================================== */

int PingerInfo::getTotalOutgoingCalls()
{
    int nNumCalls = 0;
    
    Pinger *pPinger = Pinger::getPingerTask();
    
    if (pPinger)
    {
        CallManager *pCallManager = pPinger->getCallManager();
        
        if (pCallManager)
        {
            nNumCalls = pCallManager->getTotalNumberOutgoingCalls();
        }   
    }

    return nNumCalls;
}

int PingerInfo::getTotalIncomingCalls()
{
    int nNumCalls = 0;

    Pinger *pPinger = Pinger::getPingerTask();
    
    if (pPinger)
    {
        CallManager *pCallManager = pPinger->getCallManager();
        
    
        if (pCallManager)
        {
            nNumCalls = pCallManager->getTotalNumberIncomingCalls();
        }
    }

    return nNumCalls;

}


// 
// Identifies what controls the handset/hook switch state (on hook or off 
// hook).  Options include "NORMAL" and "HEADSET_BUTTON".  The default value 
// (not defined or incorrectly defined) is NORMAL.

// HOOKSWITCH_CONTROL_NORMAL - The hook switch operates as expected.   Only the physical hook 
// switch toggles the handset state.
// HOOKSWITCH_CONTROL_HEADSET - Both the hook switch and the HEADSET button may be used to 
// manipulate the handset state.  Pressing the HEADSET button will toggle the 
// hook switch state (ignoring the physical hook switch).

PingerInfo::HookSwitchControlSetting PingerInfo::getHookSwitchControl()
{
    HookSwitchControlSetting hookSwitchSetting = HOOKSWITCH_CONTROL_NORMAL;
    
    //if not set, then we need to look up the setting
    if (mHookSwitchControlSetting == HOOKSWITCH_CONTROL_UNKNOWN)
    {
        Pinger *pPinger = Pinger::getPingerTask() ;
        if (pPinger)
        {
            UtlString strHookSwitchControlSetting;
            OsConfigDb *pConfig = pPinger->getConfigDb();
            if (pConfig)
            {
                if (pConfig->get("PHONESET_HOOKSWITCH_CONTROL", strHookSwitchControlSetting) == OS_SUCCESS)
                {
                    strHookSwitchControlSetting.toUpper();
                    if (strHookSwitchControlSetting == "HEADSET_BUTTON")
                        hookSwitchSetting = HOOKSWITCH_CONTROL_HEADSET;
                }
            }
        }

        mHookSwitchControlSetting = hookSwitchSetting;
    }

    hookSwitchSetting = mHookSwitchControlSetting;

    return hookSwitchSetting;
}

PingerInfo::HandsetEQSetting PingerInfo::getHandsetEQSetting()
{
    HandsetEQSetting handsetSetting = HANDSET_EQ_UNKNOWN;
    
    //if not set, then we need to look at the mac address.
    //if it is a new mac address then set the handset to be REV C
    if (mHandsetEQSetting == HANDSET_EQ_UNKNOWN)
    {
      
        //must be IX, so default to headset
        handsetSetting = HANDSET_EQ_HEADSET;

        //now save off the value so we don't keep
        //calculating for each call
        mHandsetEQSetting = handsetSetting;
    }
    else
        handsetSetting = mHandsetEQSetting;


    return handsetSetting;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
