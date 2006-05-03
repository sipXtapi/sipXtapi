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
// APPLICATION INCLUDES
#include "stdwx.h"
#include "sipXezPhoneSettings.h"
#include "states/PhoneStateMachine.h"
#include "utl/UtlDListIterator.h"
#include "os/OsConfigEncryption.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
sipXezPhoneSettings* sipXezPhoneSettings::spSipXezPhoneSettings = NULL;

// Constructor - private, this is a singleton
sipXezPhoneSettings::sipXezPhoneSettings() :
   mIdentity(""),
   mUsername(""),
   mPassword(""),
   mRealm(""),
   mSpeakerVolume(VOLUME_DEFAULT),
   mRingerVolume(VOLUME_DEFAULT),
   mMicGain(GAIN_DEFAULT),
   RecentNumbersListSize(10),
   mTestMode(false),
   mbEnableAEC(false),
   mbEnableOutOfBandDTMF(true),
   mbEnableSRTP(false),
   mCodecPreference(AUDIO_CODEC_BW_NORMAL),
   mVideoCodecPreference(VIDEO_CODEC_BW_NORMAL),
   mVideoQuality(3),
   mVideoBitrate(300),
   mVideoFramerate(30),
   mVideoCPU(50),
   mbSecurityEnable(false),
   mSecurityLevel(0),
   mSipPort(5060),
   mRtpPort(9000),
   mDbLocation("."),
   mCertDbPassword(""),
   mCertNickname(""),
   mPhoneState(0),
   mXPos(-1),
   mYPos(-1),
   mRed(193),
   mGreen(194),
   mBlue(245),
   mAutoAnswer(false),
   mLocationHeader(""),
   mbLimitBandwidth(false),
   mSelectedAudioCodecs(""),
   mSelectedVideoCodecs(""),
   mIdleTimeout(5),
   mbShortNames(false)
{
    PhoneStateMachine::getInstance().addObserver(this);    
}

// destructor - private
sipXezPhoneSettings::~sipXezPhoneSettings()
{
}

// static accessor for the singleton instance
sipXezPhoneSettings& sipXezPhoneSettings::getInstance()
{
   if (sipXezPhoneSettings::spSipXezPhoneSettings == NULL)
   {
      sipXezPhoneSettings::spSipXezPhoneSettings = new sipXezPhoneSettings();
   }
   return *(sipXezPhoneSettings::spSipXezPhoneSettings);
}

UtlBoolean sipXezPhoneSettings::loadSettings()
{
    configDb.clear() ;

    UtlString temps;    

    if (configDb.loadFromFile(SETTINGS_FILENAME) != OS_SUCCESS)
    {
        mStunServer = "stun.sip411.com" ;
        mStunPort = 3478 ;

        mTurnServer = "turn.sip411.com" ;
        mTurnPort = 3500 ;

        mSelectedAudioCodecs = "" ;

        mbEnableIce = true ;
        mUseRport = true ;

        mSipPort = 5060 ;
        mRtpPort = 8000 ;

        mCodecPreference = 4 ;
        mSelectedAudioCodecs = "PCMU PCMA" ;

        mContactType = CONTACT_AUTO ;

        return false;
    }

    configDb.get("USERNAME", mUsername);
    configDb.get("PASSWORD", mPassword);
    configDb.get("REALM", mRealm);
    configDb.get("IDENTITY", mIdentity);
    configDb.get("PROXY_SERVER", mProxyServer);
    configDb.get("STUN_SERVER", mStunServer);
    configDb.get("STUN_PORT", mStunPort);
    configDb.get("LOCATION_HDR", mLocationHeader);
    configDb.get("AUDIO_CODECS", temps);
    unpadCodecString(temps, mSelectedAudioCodecs);
    configDb.get("VIDEO_CODECS", temps);
    unpadCodecString(temps, mSelectedVideoCodecs);
    if (mStunPort <= 0)
    {
        mStunPort = 3478 ;
    }
    configDb.get("TURN_SERVER", mTurnServer);
    configDb.get("TURN_PORT", mTurnPort);
    if (mTurnPort <= 0)
    {
        mTurnPort = 3500 ;
    }
    configDb.get("TURN_USERNAME", mTurnUsername);
    configDb.get("TURN_PASSWORD", mTurnPassword);

    int temp = 0;
    configDb.get("USE_ICE", temp);
    if (temp == 1)
    {
        mbEnableIce = true;
    }
    else
    {
        mbEnableIce = false;
    }

    configDb.get("CERT_NICKNAME", mCertNickname);
    configDb.get("CERT_DB_PASSWORD", mCertDbPassword);

    temp = 0;
    configDb.get("USE_RPORT", temp);
    if (temp == 1)
    {
        mUseRport = true;
    }
    else
    {
        mUseRport = false;
    }
    
    configDb.get("AUTO_ANSWER", temp);
    if (temp == 1)
    {
        mAutoAnswer = true;
    }
    else
    {
        mAutoAnswer = false;
    }
    
    configDb.get("ENABLE_AEC", temp);
    mbEnableAEC = (temp == 1) ? true : false;

    configDb.get("ENABLE_OOB_DTMF", temp);
    mbEnableOutOfBandDTMF = (temp == 1) ? true : false;

    configDb.get("ENABLE_SECURITY", temp);
    mbSecurityEnable = (temp == 1) ? true : false;

    int value = 0;

    configDb.get("SPEAKER_VOLUME", value);
    if (value > 0)
    {
        mSpeakerVolume = value;
    }

    configDb.get("RINGER_VOLUME", value);
    if (value > 0)
    {
        mRingerVolume = value;
    }

    configDb.get("MIC_GAIN", value);
    if (value > 0)
    {
        mMicGain = value;
    }

    configDb.get("CODEC_PREF", value);
    if (value > 0)
    {
        mCodecPreference = value;
    }

    configDb.get("VIDEO_CODEC_PREF", value);
    if (value > 0)
    {
        mVideoCodecPreference = value;
    }

    configDb.get("VIDEO_QUALITY", value);
    if (value > 0)
    {
        mVideoQuality = value;
    }

    configDb.get("VIDEO_BITRATE", value);
    if (value > 0)
    {
        mVideoBitrate = value;
    }

    configDb.get("VIDEO_FRAMERATE", value);
    if (value > 0)
    {
        mVideoFramerate = value;
    }

    configDb.get("VIDEO_CPU", value);
    if (value > 0)
    {
        mVideoCPU = value;
    }


    configDb.get("SECURITY_LEVEL", value);
    if (value > 0)
    {
        mSecurityLevel = value;
    }

    configDb.get("SIP_PORT", value);
    if (value > 0)
    {
        mSipPort = value;
    }

    configDb.get("RTP_PORT", value);
    if (value > 0)
    {
        mRtpPort = value;
    }

    configDb.get("GUI_XPOS", value);
    if (value > 0)
    {
        mXPos = value;
    }

    configDb.get("GUI_YPOS", value);
    if (value > 0)
    {
        mYPos = value;
    }

    configDb.get("GUI_STATE", value);
    if (value > 0)
    {
        mPhoneState = value;
    }

    configDb.get("CONNECTION_TIMEOUT", value);
    if (value > 0)
    {
        mIdleTimeout = value;
    }

    if (configDb.get("GUI_BACKGROUND", value) == OS_SUCCESS)
    {
        mRed = (value&0x000000FF);
        mGreen  = (value&0x0000FF00) >> 8;
        mBlue = (value&0x00FF0000) >> 16;
    }

    configDb.get("USE_SHORTHEADERS", value);
    mbShortNames = (value == 1) ? true : false;

    int i = 0;
    char szKey[256];
    UtlString sTemp("");
    while (true)
    {
        sprintf(szKey, "RECENT_ID_%d", i);
        sTemp = "";
        configDb.get(szKey, sTemp);
        if (sTemp.compareTo("") != 0)
        {
            UtlString* pTemp = new UtlString(sTemp);
            if (!mRecentNumbers.contains(pTemp))
            {
                mRecentNumbers.append(pTemp);
            }
            else
            {
                delete pTemp;
            }
        }
        else
        {
            break;
        }
        i++;
    }

    if (!configDb.get("CONTACT_TYPE", temp))
    {
        temp = CONTACT_AUTO ;
    }
    mContactType = (SIPX_CONTACT_TYPE) temp ;

    mContactNames.destroyAll();
    configDb.loadList("AddressBookNames", mContactNames);
    mContactUrls.destroyAll();
    configDb.loadList("AddressBookUrls",  mContactUrls);
    mContactCerts.destroyAll();
    configDb.loadList("AddressBookCerts", mContactCerts);

    configDb.get("AUDIO_INPUT_DEVICE", mAudioInputDevice) ;
    configDb.get("AUDIO_OUTPUT_DEVICE", mAudioOutputDevice) ;
    configDb.get("VIDEO_CAPTURE_DEVICE", mVideoCaptureDevice) ;
    
/*    configDb.get("SELECTED_AUDIO_CODECS", mSelectedAudioCodecs);
    
    UtlDListIterator itor(mSelectedAudioCodecs);
    UtlString* pCodec;
    
    while (pCodec = (UtlString*)itor())
    {
    }*/

   return true;
}

UtlBoolean sipXezPhoneSettings::saveSettings()
{
    char szValue[256];
    char szKey[256];
    UtlString temp;

    configDb.set("USERNAME", mUsername);
    configDb.set("PASSWORD", mPassword);
    configDb.set("REALM", mRealm);
    configDb.set("IDENTITY", mIdentity);
    configDb.set("PROXY_SERVER", mProxyServer);
    configDb.set("STUN_SERVER", mStunServer);
    sprintf(szValue, "%d", mStunPort);
    configDb.set("STUN_PORT", szValue);
    configDb.set("TURN_SERVER", mTurnServer);
    sprintf(szValue, "%d", mTurnPort);
    configDb.set("TURN_PORT", szValue);
    configDb.set("TURN_USERNAME", mTurnUsername);
    configDb.set("TURN_PASSWORD", mTurnPassword);
    configDb.set("LOCATION_HDR", mLocationHeader);
    padCodecString(mSelectedAudioCodecs, temp);
    configDb.set("AUDIO_CODECS", temp);
    padCodecString(mSelectedVideoCodecs, temp);
    configDb.set("VIDEO_CODECS", temp);
    
    if (mbEnableIce)
    {
        temp = "1" ;
    }
    else
    {
        temp = "0" ;
    }
    configDb.set("USE_ICE", temp);

    configDb.set("CERT_NICKNAME", mCertNickname);
    configDb.set("CERT_DB_PASSWORD", mCertDbPassword);

    if (mUseRport)
    {
        temp = "1";
    }
    else
    {
        temp = "0";
    }
    configDb.set("USE_RPORT", temp);
    
    if (mAutoAnswer)
    {
        temp = "1";
    }
    else
    {
        temp = "0";
    }
    configDb.set("AUTO_ANSWER", temp);

    temp = (mbEnableAEC) ? "1" : "0";
    configDb.set("ENABLE_AEC", temp);

    temp = (mbEnableOutOfBandDTMF) ? "1" : "0";
    configDb.set("ENABLE_OOB_DTMF", temp);

    temp = (mbSecurityEnable) ? "1" : "0";
    configDb.set("ENABLE_SECURITY", temp);

    temp = (mbShortNames) ? "1" : "0";
    configDb.set("USE_SHORTHEADERS", temp);

    sprintf(szValue, "%d", mSpeakerVolume);
    configDb.set("SPEAKER_VOLUME", szValue);
    sprintf(szValue, "%d", mRingerVolume);
    configDb.set("RINGER_VOLUME", szValue);
    sprintf(szValue, "%d", mMicGain);
    configDb.set("MIC_GAIN", szValue);
    sprintf(szValue, "%d", mCodecPreference);
    configDb.set("CODEC_PREF", szValue);
    sprintf(szValue, "%d", mVideoCodecPreference);
    configDb.set("VIDEO_CODEC_PREF", szValue);

    sprintf(szValue, "%d", mVideoQuality);
    configDb.set("VIDEO_QUALITY", szValue);
    sprintf(szValue, "%d", mVideoBitrate);
    configDb.set("VIDEO_BITRATE", szValue);
    sprintf(szValue, "%d", mVideoFramerate);
    configDb.set("VIDEO_FRAMERATE", szValue);
    sprintf(szValue, "%d", mVideoCPU);
    configDb.set("VIDEO_CPU", szValue);
    sprintf(szValue, "%d", mSecurityLevel);
    configDb.set("SECURITY_LEVEL", szValue);

    sprintf(szValue, "%d", mSipPort);
    configDb.set("SIP_PORT", szValue);
    sprintf(szValue, "%d", mRtpPort);
    configDb.set("RTP_PORT", szValue);

    sprintf(szValue, "%d", mXPos);
    configDb.set("GUI_XPOS", szValue);
    sprintf(szValue, "%d", mYPos);
    configDb.set("GUI_YPOS", szValue);
    sprintf(szValue, "%d", mPhoneState);
    configDb.set("GUI_STATE", szValue);
    sprintf(szValue, "%d", mIdleTimeout);
    configDb.set("CONNECTION_TIMEOUT", szValue);

    int tempVal = (mRed) | (mGreen<<8) | (mBlue<<16);
    sprintf(szValue, "%d", tempVal);
    configDb.set("GUI_BACKGROUND", szValue);

   // first, get rid of the old Numbers
   while (mRecentNumbers.entries() > RecentNumbersListSize)
   {
      mRecentNumbers.remove(mRecentNumbers.first());
   }
    UtlDListIterator iterator(mRecentNumbers);
    UtlString* pRecent;

    unsigned i = 0;
    while (pRecent = (UtlString*) iterator())
    {
        if (i >= RecentNumbersListSize)
        {
            break;
        }
        sprintf(szKey, "RECENT_ID_%d", i);
        configDb.set(szKey, *pRecent);
        i++;
    }

    /*if (mSelectedAudioCodecs.entries())
    {
        configDb.addList("SELECTED_AUDIO_CODECS", mSelectedAudioCodecs);
    }*/

    sprintf(szValue, "%d", mContactType);
    configDb.set("CONTACT_TYPE", szValue);


    if (mContactNames.entries())
    {
        configDb.addList("AddressBookNames", mContactNames);
        configDb.addList("AddressBookUrls",  mContactUrls);
        configDb.addList("AddressBookCerts", mContactCerts);
    }
    else
    {
        configDb.removeByPrefix("AddressBook");
    }

    configDb.set("AUDIO_INPUT_DEVICE", mAudioInputDevice) ;
    configDb.set("AUDIO_OUTPUT_DEVICE", mAudioOutputDevice) ;
    configDb.set("VIDEO_CAPTURE_DEVICE", mVideoCaptureDevice) ;


    if (configDb.storeToFile(SETTINGS_FILENAME) != OS_SUCCESS)
    {
      return false;
    }

   return true;
}


PhoneState* sipXezPhoneSettings::OnDial(const wxString phoneNumber)
{
    UtlString* pUtlPhoneNumber = new UtlString(phoneNumber.c_str());
    if (!mRecentNumbers.contains(pUtlPhoneNumber))
    {
        mRecentNumbers.insert(pUtlPhoneNumber);
    }
    return NULL;
}

PhoneState* sipXezPhoneSettings::OnRinging(SIPX_CALL hCall)
{
    char szIncomingNumber[256];
    sipxCallGetRemoteID(hCall, szIncomingNumber, 256);
    wxString incomingNumber(szIncomingNumber);
    UtlString* pUtlIncomingNumber = new UtlString(incomingNumber.c_str());

    if (!mRecentNumbers.contains(pUtlIncomingNumber))
    {
        mRecentNumbers.insert(pUtlIncomingNumber);
    }
    return NULL;
}

UtlDList& sipXezPhoneSettings::getRecentNumberList()
{
    return mRecentNumbers;
}

const bool& sipXezPhoneSettings::getUseRport()
{
    return mUseRport;
}

void sipXezPhoneSettings::setUseRport(const bool useRport)
{
    mUseRport = useRport;
}

wxColor sipXezPhoneSettings::getBackgroundColor() const
{
    //return wxColor(132,169,181);
    return wxColor(mRed, mGreen, mBlue);
}

void sipXezPhoneSettings::setTestMode(bool bOn)
{
    mTestMode = bOn;
}

bool sipXezPhoneSettings::getTestMode()
{
    return mTestMode;
}

bool sipXezPhoneSettings::addContact(const UtlString& name, const UtlString& url, const UtlString& cert )
{
    mContactNames.insert(new UtlString(name));
    mContactUrls.insert(new UtlString(url));
    mContactCerts.insert(new UtlString(cert));
    return true;

}

void sipXezPhoneSettings::removeContact(const UtlString& name, const UtlString& url, const UtlString& cert)
{
    mContactNames.destroy((UtlContainable*)&name);
    mContactUrls.destroy((UtlContainable*)&url);    
    mContactCerts.destroy((UtlContainable*)&cert);
}

void sipXezPhoneSettings::setSelectedAudioCodecs(const UtlDList& listOfCodecs)
{
    /*mSelectedAudioCodecs.destroyAll();
    UtlDListIterator iterator(listOfCodecs);
    
    UtlString* pString;
    while (pString = (UtlString*)iterator())
    {
        mSelectedAudioCodecs.insert(new UtlString(*pString));
    }*/
        
}

const UtlString* sipXezPhoneSettings::lookupCertificate(UtlString sipUrl)
{
    UtlDListIterator urlIter(mContactUrls);
    UtlDListIterator certIter(mContactCerts);
    UtlString* pUrl;
    UtlString* pCert;

    while (pUrl = (UtlString*) urlIter())
    {
        pCert = (UtlString*) certIter();

        if (pUrl->contains(sipUrl.data()) || sipUrl.contains(pUrl->data()))
        {
            return pCert;
        }        
    }
    return NULL;
}

void sipXezPhoneSettings::padCodecString(UtlString& in, UtlString& result)
{
    const char *p = in.data();

    result = "";
    while (*p)
    {
        if (*p == ' ')
        {
            result.append('^');
        }
        else
        {
            result.append(*p);
        }
        ++p;
    }
}

void sipXezPhoneSettings::unpadCodecString(UtlString& in, UtlString& result)
{
    const char *p = in.data();

    result = "";
    while (*p)
    {
        if (*p == '^')
        {
            result.append(' ');
        }
        else
        {
            result.append(*p);
        }
        ++p;
    }
}