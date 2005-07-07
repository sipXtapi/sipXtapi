// $Id: //depot/OPENDEV/sipXphone/src/pinger/SipConfigDeviceAgent.cpp#3 $
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
#include <stdio.h>
#ifdef _WIN32
#   include <io.h>
#elif defined(__pingtel_on_posix__)
#   include <stdlib.h>
#   include <unistd.h>
#   define O_BINARY 0 // There is no notion of a "not binary" file under POSIX,
                      // so we just set O_BINARY used below to no bits in the mask.
#endif
#include <sys/stat.h>
#include <fcntl.h> 

// APPLICATION INCLUDES
#include <pinger/SipConfigDeviceAgent.h>
#include <pinger/Pinger.h>
#include <pinger/PingerConfigEncryption.h>
#include <net/SipUserAgent.h>
#include <net/SipRefreshMgr.h>
#include <net/SipMessageEvent.h>
#include <net/NameValueTokenizer.h>
#include <net/NetAttributeTokenizer.h>
#include <net/Url.h>
#include <cp/CallManager.h>
#include <os/OsConnectionSocket.h>
#include <os/OsDateTime.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define CONFIG_AGENT_ENABLED 1
#define DEFAULT_EXPIRES 60*60*24 //24 hrs

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipConfigDeviceAgent::SipConfigDeviceAgent(const char* model,
                                            const char* version,
                                            const char* macAddress,
                                            const char* serialNum,
                                            SipUserAgent* userAgent,
                                            SipRefreshMgr* Refreshmgr,
                                           const char* fileSystemRoot,
                                           const char* explicitHostAddress,
                                           int explicitHostPort,
                                           int restartCount) 
: OsServerTask("SipCfgDvcAgnt-%d")
{
   mMidDiscovery = FALSE;
   mConfigServerDiscoveryMethod = NO_DISCOVERY;
   mEnrollmentResponse = 0;
   mpEnrollmentSubscribe = NULL;
   mLocalCSeq = 0;
   mRemoteCSeq = 0;
   mRestartCount = restartCount;

   mVendor = "SIPFoundry";
   mModel = model ? model : "sipXphone";
   mVersion = version ? version : "1.0.0";
   mMac = macAddress ? macAddress : "";
   mSerial = serialNum ? serialNum : "1001";


   mConfigConfigFileName = "config-config";
   mProfileFileNameMap.set("x-xpressa-device", "pinger-config");
   mProfileFileNameMap.set("x-xpressa-user", "user-config");
   mProfileFileNameMap.set("x-xpressa-apps", "app-config");
   mProfileFileNameMap.set("x-xpressa-install", "");

   if(fileSystemRoot) mFileSystemRoot = fileSystemRoot;

   if(userAgent) 
   {
       mpSipUserAgent = userAgent;
       mpSipUserAgent->addMessageObserver(*(this->getMessageQueue()),
           SIP_NOTIFY_METHOD, 
           TRUE, // do want to get requests
           FALSE, // do not want responses
           TRUE, // Incoming messages
           FALSE); // Don't want to see out going messages

       mpSipUserAgent->addMessageObserver(*(this->getMessageQueue()),
           SIP_SUBSCRIBE_METHOD, 
           FALSE, // do not want to get requests
           TRUE, // do want responses
           TRUE, // Incoming messages
           FALSE); // Don't want to see out going messages
   }
   if(Refreshmgr)
   {
       mpRefreshMgr = Refreshmgr;
   }

   if(!mFileSystemRoot.isNull() &&
       !mConfigConfigFileName.isNull())
   {
       UtlString configFileFullPath(mFileSystemRoot);
       configFileFullPath.append('/');
       configFileFullPath.append(mConfigConfigFileName);

       // Read the current config agent configuration.
       mProfileUrls.loadFromFile(configFileFullPath.data());
   }

   setServer(explicitHostAddress, explicitHostPort);

   if (mConfigServerDiscoveryMethod == NO_DISCOVERY)
   {
       mProfileUrls.get(CONFIG_ENROLLMENT_HOST, mConfigServerAddress);
       mProfileUrls.get(CONFIG_ENROLLMENT_PORT, mConfigServerPort);
   }
}

// Copy constructor
SipConfigDeviceAgent::SipConfigDeviceAgent(const SipConfigDeviceAgent& rSipConfigDeviceAgent)
{
}

// Destructor
SipConfigDeviceAgent::~SipConfigDeviceAgent()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipConfigDeviceAgent& 
SipConfigDeviceAgent::operator=(const SipConfigDeviceAgent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean SipConfigDeviceAgent::handleMessage(OsMsg& eventMessage)
{
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();

    // if this is a SIP message
    if(msgType == OsMsg::PHONE_APP &&
        msgSubType == CallManager::CP_SIP_MESSAGE)
    {
        const SipMessage* sipMessage = ((SipMessageEvent&)eventMessage).getMessage();
        int messageType = ((SipMessageEvent&)eventMessage).getMessageStatus();
        osPrintf("SipConfigDeviceAgent::messageType: %d\n", messageType);
        UtlString method;

        // This is a request which failed to get sent 
        if(messageType == SipMessageEvent::TRANSPORT_ERROR)
        {
            sipMessage->getRequestMethod(&method);
            osPrintf("SipConfigDeviceAgent:: Processing message transport error method: %s\n",
                sipMessage->isResponse() ? method.data() : "response");

            if(sipMessage->isResponse())
            {
                int seqNum;
                sipMessage->getCSeqField(&seqNum, &method);
                // SUBSCIBE (enrollment) response
                if(method.compareTo(SIP_SUBSCRIBE_METHOD))
                {
                    // We are sad our config. server is not there
                    mEnrollmentResponse = SIP_NOT_FOUND_CODE;
                    osPrintf("SipConfigDeviceAgent::handleMessage enroll FAILURE: no response\n");
                }

            }
        }

        else if(messageType == SipMessageEvent::AUTHENTICATION_RETRY)
        {
        }

        // If this is a response
        else if(sipMessage->isResponse())
        {
            int seqNum;
            sipMessage->getCSeqField(&seqNum, &method);
            // SUBSCIBE (enrollment) response
            if(method.compareTo(SIP_SUBSCRIBE_METHOD))
            {
                int responseCode = sipMessage->getResponseStatusCode();
                if(responseCode < SIP_2XX_CLASS_CODE)
                {
                    // Provisional response ignore it
                }
                else if(responseCode >= SIP_2XX_CLASS_CODE &&
                    responseCode < SIP_3XX_CLASS_CODE)
                {
                    // We are happy our config. server is there
                    mMidDiscovery = FALSE;
                    mEnrollmentResponse = SIP_OK_CODE;
                    osPrintf("SipConfigDeviceAgent::handleMessage enroll SUCCESS: %d\n",
                        responseCode);
                }
                else
                {
                    // We are sad our config. server does not like us
                    mEnrollmentResponse = responseCode;
                    osPrintf("SipConfigDeviceAgent::handleMessage enroll FAILURE: %d\n",
                        responseCode);
                }
            }

        }

        // This is a request
        else
        {
            sipMessage->getRequestMethod(&method);
            UtlString eventType;
            sipMessage->getEventField(eventType);
            eventType.toLower();
            UtlString toField;
            sipMessage->getToField(&toField);
            Url toUrl(toField);
            UtlString macAddress;
            toUrl.getFieldParameter("Mac", macAddress);
            UtlString callId;
            sipMessage->getCallIdField(&callId);
            // NOTIFY (config change) request
            if(method.compareTo(SIP_NOTIFY_METHOD) == 0 &&
                eventType.index(SIP_EVENT_CONFIG, 0, UtlString::ignoreCase) == 0)
            {
               SipMessage response;
               // accept message only if callid is same and the mach addres is its own address
               if( macAddress.compareTo(mMac) == 0 && mCallId.compareTo(callId) == 0 )
               {
                   // Always send an ok for now
                   // Eventually we may want to send a trying here
                   //  and then send a 200 OK after the profiles
                   // have successfully been loaded
                   response.setOkResponseData(sipMessage);
                   int sentOk;
                   if(mpSipUserAgent) 
                       sentOk = mpSipUserAgent->send(response);

                   const HttpBody* body = sipMessage->getBody();
                
                   if(body)
                   {
                       UtlString bodyBytes;
                       int bodyLen;
                       body->getBytes(&bodyBytes, &bodyLen);

                       updateConfigUrls(bodyBytes.data());
                   }
               }
               else
               {
                  //send a 481 response
                    response.setResponseData(sipMessage, SIP_BAD_TRANSACTION_CODE, 
                    SIP_BAD_TRANSACTION_TEXT);
                  if(mpSipUserAgent) 
                     mpSipUserAgent->send(response);
               }
            }
        }
    }

    return(TRUE);
}

UtlBoolean SipConfigDeviceAgent::setServer(const char* serverAddress,
                                          int serverPort)
{
    UtlBoolean hostFound = FALSE;

    if(serverAddress && *serverAddress)
    {
        UtlString configServerIpAddress;

        // Handle either a host name or an IP address
        OsSocket::getHostIpByName(serverAddress, &configServerIpAddress);

        if(!configServerIpAddress.isNull() &&
            configServerIpAddress.compareTo("0.0.0.0"))
        {
            hostFound = TRUE;
            mConfigServerAddress = serverAddress;
            // We appear to have a legitimate IP address
            mProfileUrls.set(CONFIG_ENROLLMENT_HOST, serverAddress);
            char portChar[20];
            mConfigServerPort = serverPort;
            sprintf(portChar, "%d", serverPort);
            mProfileUrls.set(CONFIG_ENROLLMENT_PORT, portChar);
            mConfigServerDiscoveryMethod = MANUAL_DISCOVERY;
        }
        else
        {
          // Our IP address was bad
          mConfigServerDiscoveryMethod = NO_DISCOVERY;
        }
    }

    return(hostFound);
}

UtlBoolean SipConfigDeviceAgent::discover()
{
    UtlBoolean foundServer = FALSE;
    if(!mMidDiscovery)
    {
        mConfigServerDiscoveryMethod = NO_DISCOVERY;
        mMidDiscovery = TRUE;
    }
    UtlString serverAddress;


    // Check DHCP options

    // Check DNS SRV

    // Check DNS A record
    // This is temporary.  Eventually the dicover method should
    // ignore the fact that the address is already set and just
    // redo the discovery process
#ifdef CONFIG_AGENT_ENABLED
    if(mConfigServerAddress.isNull())
    {
        //see if we have a domain name
        UtlString domainName;
        UtlString fullServerConfigAddress;
        OsSocket::getDomainName(domainName);
        if (domainName.length())
        {
            osPrintf("Domain Name: %s\n",domainName.data());
            fullServerConfigAddress = DEFAULT_SIP_CONFIG_SERVER;
            fullServerConfigAddress += ".";
            fullServerConfigAddress += domainName;

            OsSocket::getHostIpByName(fullServerConfigAddress, &serverAddress);

            if(!serverAddress.isNull() && serverAddress.compareTo("0.0.0.0"))
            {
                mConfigServerPort = 0;
                foundServer = TRUE;
                char portString[20];
                sprintf(portString, "%d", mConfigServerPort);
                mConfigServerDiscoveryMethod = DNS_A_DISCOVERY;
                mConfigServerAddress = fullServerConfigAddress;
                mProfileUrls.set(CONFIG_ENROLLMENT_HOST, fullServerConfigAddress);
                mProfileUrls.set(CONFIG_ENROLLMENT_PORT, portString);
            }
            
            osPrintf("SipConfigDeviceAgent Discovery Results:\n") ;
            osPrintf("    Method: %d\n", mConfigServerDiscoveryMethod) ;
            osPrintf("        Server Address: %s\n", mConfigServerAddress.data()) ;
            osPrintf("           Server Port: %d=n", mConfigServerPort) ;
        }
        else
        {
            osPrintf("NO DOMAIN NAME! Skipping sipuaconfig lookup.\n");
        }
        
   }
#endif


    return(foundServer);
}

UtlBoolean SipConfigDeviceAgent::enroll()
{
    UtlBoolean sentOk = FALSE;
    if(!mConfigServerAddress.isNull())
    {
        mLocalCSeq++;

        // Get the profile names
        UtlString previousKey;
        UtlString aProfile;
        UtlString profilesField;
        UtlString profileFileName;
        while(OS_SUCCESS == mProfileFileNameMap.getNext(previousKey,
                      aProfile, profileFileName))
        {
            if(!profilesField.isNull())
                profilesField.append(", ");
            profilesField.append(aProfile.data());
            previousKey = aProfile;
        }

        // Build a uri and to field
        Url toUrl;
        toUrl.setUserId(DEFAULT_SIP_CONFIG_SERVER);
        toUrl.setHostAddress(mConfigServerAddress.data());
        toUrl.setHostPort(mConfigServerPort);
        toUrl.setUrlType("sip");
        UtlString toField;
        toUrl.toString(toField);

         int epochTime = OsDateTime::getSecsSinceEpoch();
         char prefix[80];
         sprintf(prefix, "%d", epochTime);

          // Build a from tag
         int fromTagInt = rand();
         char fromTagBuffer[100];
         sprintf(fromTagBuffer, "%dcec%d", fromTagInt, epochTime);

         if( mCallId.isNull() )
         {
            //generate callId
            OsSocket::getHostIp(&mCallId);
            mCallId.insert(0, "@");
            mCallId.insert(0, mMac);
            mCallId.insert(0,"-");
            mCallId.insert(0,fromTagBuffer);
            mCallId.insert(0, "config-");
         }

        // Build a from
        UtlString fromField;
        if(mpSipUserAgent) mpSipUserAgent->getContactUri(&fromField);
        Url fromUrl(fromField.data());
        fromUrl.setFieldParameter("Vendor", mVendor.data());
        fromUrl.setFieldParameter("Model", mModel.data());
        fromUrl.setFieldParameter("Version", mVersion.data());
        fromUrl.setFieldParameter("Serial", mSerial.data());
        fromUrl.setFieldParameter("Mac", mMac.data());
        fromUrl.setFieldParameter("tag",fromTagBuffer);
        fromUrl.toString(fromField);

        UtlString localContact;
        mpSipUserAgent->getContactUri(&localContact);
        SipMessage* enrollRequest = new SipMessage();
        enrollRequest->setEnrollmentData(toField.data(),
                                         fromField.data(),
                                         toField.data(),
                                         mCallId.data(),
                                         mLocalCSeq,
                                         localContact,
                                         "http",
                                         profilesField,
                                         DEFAULT_EXPIRES);


        /*if(mpSipUserAgent) 
            sentOk = mpSipUserAgent->send(*enrollRequest);*/
        if (mpRefreshMgr)
            mpRefreshMgr->newSubscribeMsg(*enrollRequest);

        if(mpEnrollmentSubscribe) delete mpEnrollmentSubscribe;
        mpEnrollmentSubscribe = enrollRequest;
        mEnrollmentResponse = 0;
        if(sentOk) mEnrollmentResponse = -1;
    }

    return(sentOk);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void SipConfigDeviceAgent::updateConfigUrls(const char* urlListContents)
{
    NameValueTokenizer configUrlTokenizer(urlListContents);
    UtlString configProfileName;
    UtlString newUrlSequenceValues;
    UtlString oldUrlSequenceValues;
    UtlString newUrl;
    UtlString oldUrl;
    int newSequence = -1;
    int oldSequence = -1;
    UtlBoolean configFileChanged = FALSE;
    UtlBoolean bRestartNow = FALSE ;       // Should we restart ASAP?
    UtlBoolean bIncoming = FALSE;

    while(configUrlTokenizer.getNextPair(HTTP_NAME_VALUE_DELIMITER,
                &configProfileName, & newUrlSequenceValues))
    {
        configProfileName = configProfileName.strip(UtlString::both) ;
        // Get the previous url and seq. data
        mProfileUrls.get(configProfileName.data(), oldUrlSequenceValues);

        // Parse the values out
        parseUrlSequence(oldUrlSequenceValues.data(), oldUrl, oldSequence);

        // parse the new values out
        parseUrlSequence(newUrlSequenceValues.data(), newUrl, newSequence);

        // The url location or the content has changed (@JC Changed 
        // to load profiles if the sequence numbers are different
        // and not just that the new versino is > the old version
        // since phones that are reassigned will not pick up the sequence info
        if(oldUrl.compareTo(newUrl) || newSequence != oldSequence)
        {
            configFileChanged = TRUE;
            mProfileUrls.set(configProfileName.data(), newUrlSequenceValues);
            loadProfile(configProfileName.data(), newUrl.data(), bIncoming, bRestartNow);
        }
    }

    // Cache the config. agent configuration if it changed
    if(configFileChanged && (!mFileSystemRoot.isNull() &&
       !mConfigConfigFileName.isNull()))
    {
       UtlString configFileFullPath(mFileSystemRoot);
       configFileFullPath.append('/');
       configFileFullPath.append(mConfigConfigFileName);

       // Save the current config agent configuration.
       mProfileUrls.storeToFile(configFileFullPath.data());
    }

    // Aftering Updating the configuration, force a restart.  If only 
    // configuration data changed, then allows the users to delay the restart.
    // However, if a new kernel/pingtel.jar was retrieved, restart immediately
    // without prompting.
    if (configFileChanged)
    {  
       char reason[64];
       strncpy(reason, bIncoming ? RESTART_INCOMING_CONFIG : RESTART_NEW_CONFIG, sizeof(reason));
       if (bRestartNow)
       {
           Pinger::getPingerTask()->restart(FALSE, -1, reason) ;
       }
       else
       {
          int iConfigDelay ;
          OsConfigDb *pConfigDb = Pinger::getPingerTask()->getConfigDb() ;          
          if (pConfigDb->get("PHONESET_CONFIG_RESTART_DELAY", iConfigDelay) != OS_SUCCESS)
             iConfigDelay = 45 ;

          Pinger::getPingerTask()->restart(TRUE, iConfigDelay, reason) ;          
       }
    }
}

void SipConfigDeviceAgent::parseUrlSequence(const char* profileSequenceUrl, 
                                            UtlString& url, 
                                            int& sequence)
{
    NetAttributeTokenizer tokenizer(profileSequenceUrl);
    url.remove(0);
    UtlString name;
    UtlString value;

    while(tokenizer.getNextAttribute(name, value))
    {
        name.toUpper();
        if(name.compareTo("URL") == 0)
        {
            url = value;
        }
        else if(name.compareTo("SEQUENCE") == 0)
        {
            sequence = atoi(value.data());
        }
    }
}

UtlBoolean SipConfigDeviceAgent::loadProfile(const char* profileName, 
                                            const char* profileUrl,
                                            UtlBoolean& bIncomingProfile,
                                            UtlBoolean& bRestartNow)
{
    UtlString profileFileName;
    int foundProfile = mProfileFileNameMap.get(profileName, profileFileName);
    UtlBoolean profileSaved = FALSE;
    bRestartNow = FALSE ;

    // Ignore any unknown profiles
    if(foundProfile == OS_SUCCESS && 
        !profileFileName.isNull())
    {
        Url httpUrl(profileUrl);
        UtlString httpServer;
        int httpPort;
        httpUrl.getHostAddress(httpServer);
        httpPort = httpUrl.getHostPort();
        UtlString httpPath;
        httpUrl.getPath(httpPath);
        if(httpPort <= 0) httpPort = 80;

        HttpMessage profileRequest;
        profileRequest.setRequestFirstHeaderLine("GET", httpPath.data(), "HTTP/1.0");
        OsConnectionSocket httpClientSocket(httpPort, httpServer.data());
        profileRequest.write(&httpClientSocket);
        HttpMessage profileResponse;
        profileResponse.read(&httpClientSocket);

        if(profileResponse.getResponseStatusCode() == HTTP_OK_CODE)
        {
            // We got the profile
            const HttpBody* body = profileResponse.getBody();
            UtlString bodyContent;
            if(body)
            {
                int len;
                body->getBytes(&bodyContent,&len);
                len = bodyContent.length();
                int contentLen = profileResponse.getContentLength();
                
                if(len == contentLen && 
                    !mFileSystemRoot.isNull())
                {
                    UtlString profileFilePath(mFileSystemRoot);
                    profileFilePath.append('/');
                    profileFilePath.append(profileFileName);

                    // Encrypted files do not replace existing files
                    // because if user does not get a chance to
                    // decrypt them before system restarts, C++ layer
                    // will not be in a position to prompt user for
                    // password to decrypt them. So we save them to
                    // save them to a different spot to decrypt at
                    // user's convienece.
                    PingerConfigEncryption *encryptor = Pinger::getPingerTask()->getConfigEncryption();
                    if (encryptor->isEncryptedByConfigServer(profileFilePath.data(), 
                       (const char *)bodyContent.data(), bodyContent.length()))
                    {
                        profileFilePath.append(PCE_INCOMING_PROFILE_SUFFIX);
                        bIncomingProfile = TRUE;
                    }

                    profileSaved = saveProfile(profileFilePath, profileUrl, bodyContent, len);
                }

                else
                {
                    osPrintf("SipConfigDeviceAgent::loadProfile content len: %d bytes: %d differ\n",
                        contentLen, len);
                }
            }
        }
    }
    else
    {
        osPrintf("SipConfigDeviceAgent::loadProfile invalid profile: %s url: %s\n",
            profileName, profileUrl);
    }

    return(profileSaved);
}

UtlBoolean SipConfigDeviceAgent::saveProfile(UtlString& profileFilePath, const char *profileUrl, UtlString& bodyContent, int len)
        {
    UtlBoolean profileSaved = FALSE;

    int fileDesc = open(profileFilePath.data(), 
        O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 
        S_IREAD | S_IWRITE);

    if(fileDesc >= 0)
    {
        int bytesWritten = write(fileDesc, 
            (char*)bodyContent.data(), len);
        
        osPrintf("SipConfigDeviceAgent::loadProfile saved url: %s as %s\n",
                 profileUrl, profileFilePath.data());

        if(bytesWritten == len) 
            profileSaved = TRUE;
        
        close(fileDesc);
    }

    return profileSaved;
}


/* ============================ FUNCTIONS ================================= */

