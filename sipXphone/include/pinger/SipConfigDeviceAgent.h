// $Id: //depot/OPENDEV/sipXphone/include/pinger/SipConfigDeviceAgent.h#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipConfigDeviceAgent_h_
#define _SipConfigDeviceAgent_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsServerTask.h>
#include <os/OsConfigDb.h>

// DEFINES
#define DEFAULT_SIP_CONFIG_SERVER "sipuaconfig"
#define CONFIG_ENROLLMENT_HOST "CONFIG_ENROLLMENT_HOST"
#define CONFIG_ENROLLMENT_PORT "CONFIG_ENROLLMENT_PORT"
#define RESTART_REASON_LEN 64
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;
class SipMessage;
class SipRefreshMgr;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipConfigDeviceAgent : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    enum ConfigDiscoveryMethods
    {
        NO_DISCOVERY = 0,
        DHCP_DISCOVERY,
        DNS_SRV_DISCOVERY,
        DNS_A_DISCOVERY,
        MULTICAST_DISCOVERY,
        MANUAL_DISCOVERY
    };

/* ============================ CREATORS ================================== */

   SipConfigDeviceAgent(const char* model = NULL,
                        const char* version = NULL,
                        const char* macAddress = NULL,
                        const char* serialNum = NULL,
                        SipUserAgent* userAgent = NULL, 
						SipRefreshMgr* refrshMgr = NULL,
                        const char* fileSystemRoot = NULL,
                        const char* explicitHostAddress = NULL,
                        int explicitHostPort = 0,
                        int restartCount = 1);
     //:Default constructor

   virtual
   ~SipConfigDeviceAgent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

    UtlBoolean setServer(const char* serverAddress, int serverPort);
    //: Manually set the config. server

    UtlBoolean discover();
    //: Automatically discover the config. server

    UtlBoolean enroll();
    //: Signup for configuration management with the server


   virtual UtlBoolean handleMessage(OsMsg& eventMessage);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

   UtlBoolean isConfigServerDiscovered();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipConfigDeviceAgent(const SipConfigDeviceAgent& rSipConfigDeviceAgent);
     //:Copy constructor disabled

   SipConfigDeviceAgent& operator=(const SipConfigDeviceAgent& rhs);
     //:Assignment operator disabled

   //UtlBoolean checkForConfigChanges();
   void updateConfigUrls(const char* urlListContents);
   void parseUrlSequence(const char* profileSequenceUrl, UtlString& url, 
                        int& sequence);

   UtlBoolean loadProfile(const char* profileName, const char* profileUrl, 
                         UtlBoolean& bIncoming, UtlBoolean& bRestartNow);

   UtlBoolean saveProfile(UtlString& profileFilePath, const char *profileUrl, 
                        UtlString& bodyContent, int len);

   SipUserAgent* mpSipUserAgent;
   SipRefreshMgr* mpRefreshMgr;

   UtlBoolean mMidDiscovery;
   UtlString mConfigServerAddress;
   int mConfigServerPort;
   enum ConfigDiscoveryMethods mConfigServerDiscoveryMethod;

   UtlString mVendor;
   UtlString mModel;
   UtlString mVersion;
   UtlString mMac;
   UtlString mSerial;

   UtlString mFileSystemRoot;
   UtlString mConfigConfigFileName; // The place config. config. info is cached
   OsConfigDb mProfileUrls; // Hash of NOTIFY body contents
   OsConfigDb mProfileFileNameMap; // maps a profile name into the filename that is used to store it on the phone

   int mLocalCSeq;
   int mRemoteCSeq;
   int mEnrollmentResponse;
   UtlString mCallId;
   SipMessage* mpEnrollmentSubscribe;
   int mRestartCount;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipConfigDeviceAgent_h_
