//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "stdwx.h"
#include "sipXezPhoneSettings.h"
#include "states/PhoneStateMachine.h"
#include "utl/UtlDListIterator.h"

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
   mCodecPreference(AUDIO_CODEC_BW_NORMAL)
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
    if (configDb.loadFromFile(SETTINGS_FILENAME) != OS_SUCCESS)
    {
        return false;
    }

    configDb.get("USERNAME", mUsername);
    configDb.get("PASSWORD", mPassword);
    configDb.get("REALM", mRealm);
    configDb.get("IDENTITY", mIdentity);
    configDb.get("PROXY_SERVER", mProxyServer);
    configDb.get("STUN_SERVER", mStunServer);

    int temp = 0;
    configDb.get("USE_RPORT", temp);
    if (temp == 1)
    {
        mUseRport = true;
    }
    else
    {
        mUseRport = false;
    }
    configDb.get("ENABLE_AEC", temp);
    mbEnableAEC = (temp == 1) ? true : false;

    configDb.get("ENABLE_OOB_DTMF", temp);
    mbEnableOutOfBandDTMF = (temp == 1) ? true : false;

    configDb.get("ENABLE_SRTP", temp);
    mbEnableSRTP = (temp == 1) ? true : false;

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
    mCodecPreference = value;

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

   return true;
}

UtlBoolean sipXezPhoneSettings::saveSettings()
{
    configDb.set("USERNAME", mUsername);
    configDb.set("PASSWORD", mPassword);
    configDb.set("REALM", mRealm);
    configDb.set("IDENTITY", mIdentity);
    configDb.set("PROXY_SERVER", mProxyServer);
    configDb.set("STUN_SERVER", mStunServer);

    UtlString temp("0");
    if (mUseRport)
    {
        temp = "1";
    }
    configDb.set("USE_RPORT", temp);

    temp = (mbEnableAEC) ? "1" : "0";
    configDb.set("ENABLE_AEC", temp);

    temp = (mbEnableOutOfBandDTMF) ? "1" : "0";
    configDb.set("ENABLE_OOB_DTMF", temp);

    temp = (mbEnableSRTP) ? "1" : "0";
    configDb.set("ENABLE_SRTP", temp);

    char szValue[4];
    char szKey[256];

    sprintf(szValue, "%d", mSpeakerVolume);
    configDb.set("SPEAKER_VOLUME", szValue);
    sprintf(szValue, "%d", mRingerVolume);
    configDb.set("RINGER_VOLUME", szValue);
    sprintf(szValue, "%d", mMicGain);
    configDb.set("MIC_GAIN", szValue);
    sprintf(szValue, "%d", mCodecPreference);
    configDb.set("CODEC_PREF", szValue);


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
    return wxColor(193, 194, 245);
}

void sipXezPhoneSettings::setTestMode(bool bOn)
{
    mTestMode = bOn;
}

bool sipXezPhoneSettings::getTestMode()
{
    return mTestMode;
}
