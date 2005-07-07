// $Id: //depot/OPENDEV/sipXphone/src/pinger/PingerConfigEncryption.cpp#4 $
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
#include <ctype.h>
#include <time.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#include <netdb.h>
#endif

// APPLICATION INCLUDES
#include "os/OsUtil.h"
#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "os/OsFS.h"
#include "os/OsPathBase.h"
#include "os/OsExcept.h"
#include "os/OsConfigDb.h"
#include "os/OsEncryption.h"
#include "os/OsConfigEncryption.h"
#include "pinger/PingerConfigEncryption.h"
#include "pinger/Pinger.h"
#include "pinger/PingerInfo.h"
#include "pingerjni/JXAPI.h"
#include "net/NetMd5Codec.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define ENCRYPTED_PROFILE_COUNT 3

// Header on any config file to test if it's encrypted. Can be anything.
static const unsigned char gEncryptedHeader[] = { 
  'P', 'I', 'N', 'G',
  '_', 'B', 'I', 'N', 
  'A', 'R', 'Y', '_',
  'F', 'I', 'L', 'E'
};

// ORDER IS IMPORTANT HERE: if new configuration requires config files to be written 
// out as encrypted but they've already been written out any other profiles we'd
// have to recycle thru AND profile may have been written out unencrypted if only
// for a brief second
#define PCE_DEVICE_CONFIG 0
#define PCE_USER_CONFIG   1
#define PCE_APP_CONFIG    2
const PingerConfigEncryptionEntry gEncryptedProfiles[ENCRYPTED_PROFILE_COUNT] = 
{
    { "pinger-config", "PHONESET_" },
    { "user-config", "USER_DEFAULT_OUTBOUND_LINE" },
    { "app-config", "USER," }
};

// PERFORMANCE: Global reference and not global pointer to allocate memory in global memory heap
// at the cost of always having this on the global stack.
PingerConfigEncryption gPingerConfigInstance;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

PingerConfigEncryption::PingerConfigEncryption()
   : mWriteEncryptedEnabled(FALSE)
{
    memset(mKey, 0, sizeof(mKey));
    memset(mUser, 0, sizeof(mUser));
    memset(mPassword, 0, sizeof(mPassword));
    mUserCanceledPassword = FALSE;
}

PingerConfigEncryption::~PingerConfigEncryption()
{
}

void PingerConfigEncryption::initLocalConfig(OsConfigDb *cfg)
{
    if (cfg != NULL)
    {
        cfg->get("SERIAL_NUMBER", mSerialNum);
    }
    else
    {
        OsSysLog::add(FAC_AUTH, PRI_ERR, "Cannot get serial number from configuration information");
    }
}

void PingerConfigEncryption::initConfig(OsConfigDb *cfg)
{
    initWriteEnabled(cfg);

    if (cfg != NULL)
    {
        UtlString user;
        if (OS_SUCCESS == cfg->get("PHONESET_DEFAULT_USER", user))
        {
            strncpy(mUser, user.data(), sizeof(mUser));
        }
    }
}

void PingerConfigEncryption::initWriteEnabled(OsConfigDb *cfg)
{
    // NOTE: Originally this could be determined dynamically by calling
    // Pinger.getPingerTask().getLocalConfig().get("ENCRYPT_PROFILES") 
    // however saving profiles as encrypted needs to be determined before
    // call to getPingerTask is valid (is results in a deadlock)
    mWriteEncryptedEnabled = FALSE;
    if (cfg != NULL)
    {
        UtlString value;
        cfg->get("ENCRYPT_PROFILES", value);
        mWriteEncryptedEnabled = (value.compareTo("ENABLE", UtlString::ignoreCase) == 0) ;
        OsSysLog::add(FAC_AUTH, PRI_INFO, "encryption of local profile enabled = %d\n", 
            mWriteEncryptedEnabled);
    }
}

// Logic for encryption rules of OsConfigDb for the phone
UtlBoolean PingerConfigEncryption::isEncrypted(OsConfigDb *cfg, const char *buffer,
        int buffLen)
{
    return isEncrypted(cfg->getIdentityLabel(), buffer, buffLen);
}


UtlBoolean PingerConfigEncryption::isEncrypted(const char *profile, const char *buffer, 
        int buffLen)
{
    if (profile != NULL && buffer != NULL && buffLen >= (int) sizeof(gEncryptedHeader))
    {
        return (memcmp(buffer, gEncryptedHeader, sizeof(gEncryptedHeader)) == 0);
    }

    return FALSE;
}

UtlBoolean PingerConfigEncryption::isEncryptedByConfigServer(const char *profile, const char *buffer, 
        int buffLen)
{
    UtlBoolean isEncrypted = FALSE;
    if (profile != NULL && buffer != NULL && buffLen > 0)
    {
        const PingerConfigEncryptionEntry *entry = getProfileEntry(profile);
        if (entry != NULL && buffer != NULL && buffLen > 0)
        {
            isEncrypted = !OsUtil::memscan(buffer, buffLen, entry->encryptedContentScanTest, 
                                           strlen(entry->encryptedContentScanTest));
        }
    }

    return isEncrypted;
}

UtlBoolean PingerConfigEncryption::isWriteEncryptedEnabled()
{
    return mWriteEncryptedEnabled;
}

UtlBoolean PingerConfigEncryption::isNormallyEncrypted(OsConfigDb *cfg)
{
    UtlBoolean isEncrypted = FALSE;
    if (cfg != NULL && cfg->getIdentityLabel() != NULL)
    {
        isEncrypted = (getProfileEntry(cfg->getIdentityLabel()) != NULL);
    }

    return isEncrypted;
}

OsStatus PingerConfigEncryption::encrypt(OsConfigDb *cfg, OsEncryption *e, char *buff, int buffLen)
{
    OsStatus status = OS_FAILED;
    if (cfg != NULL && e != NULL && buff != NULL && buffLen > 0)
    {
        osPrintf("encrypt %s\n", cfg->getIdentityLabel());
        setEncryptionSettings(e, buff, buffLen);
        status = e->encrypt();
    }

    return status;
}

OsStatus PingerConfigEncryption::decrypt(OsConfigDb *cfg, OsEncryption *e, char *buff, int buffLen)
{
    OsStatus status = OS_FAILED;
    if (cfg != NULL && e != NULL && buff != NULL && buffLen > 0)
    {
        osPrintf("decrypt %s\n", cfg->getIdentityLabel());
        setEncryptionSettings(e, buff, buffLen);
        status = e->decrypt();
    }

    return status;
}

//
UtlBoolean PingerConfigEncryption::handleIncomingProfiles()
{
    UtlBoolean profileSaved = FALSE;
    UtlBoolean userCanceled = FALSE;

    // file any number of encrypted file were recieved from config server, then they'd
    // be on the file system with the appropriate filename suffix.  Find them, prompt
    // user for username password to decrypt them, then store them in replace of
    // true profiles.  If the user cannot decrypt them, the true profiles will not
    // be replaced
    int passwordAttempts = 0;
    for (int profileNdx = 0; !userCanceled && profileNdx < ENCRYPTED_PROFILE_COUNT; profileNdx++)
    {
        UtlString profilePath;
        PingerInfo::getFlashFileSystemLocation(profilePath);
        profilePath.append(OsPath::separator);
        profilePath.append(gEncryptedProfiles[profileNdx].profileId);

        UtlString incomingPath = UtlString(profilePath);
        incomingPath.append(PCE_INCOMING_PROFILE_SUFFIX);
        OsFile incomingFile(incomingPath.data());

        if (incomingFile.exists())
        {
            osPrintf("Found encrypted profile %s\n", incomingPath.data());
            if (promptUserForKey(passwordAttempts++))
            {
                OsStatus status = replaceIncomingProfile((const char *)profilePath.data(), 
                    incomingFile, profileNdx);

                if (status == OS_SUCCESS)
                {
                    profileSaved = TRUE;
                    saveKey();
                }
                else if (status == OS_FAILED)
                {
                    // wrong password, do over
                    profileNdx--;
                    memset(mKey, 0, sizeof(mKey));
                }
            }
            else
            {
                userCanceled = TRUE;
            }
        }
    }

    return profileSaved;
}

/* //////////////////////////////////////// NON-PUBLIC ////////////////////////////////////////// */

UtlBoolean PingerConfigEncryption::promptUserForKey(int passwordAttempts)
{
    UtlBoolean ok = TRUE;
    // mKey is cleared on bad password attempts
    if (strlen(mKey) == 0)
    {
        char password[PCE_MAX_PASSWORD_LEN];
        memset(password, 0, sizeof(password));
        if (JXAPI_AuthorizeProfiles(passwordAttempts, mUser, sizeof(mUser), 
                                    password, sizeof(password)) != 0)
        {
            ok = FALSE;
        }
        else
        {
            strncpy(mPassword, password, sizeof(password));
            buildKey();
        }
    }
    
    return ok;
}

void PingerConfigEncryption::buildKey()
{
    // MUST MATCH EXATLY WHATS IN CONFIG SERVER
    UtlString s;
    s.append(mUser).append(':').append(mPassword);
    UtlString key;
    NetMd5Codec::encode(s.data(), key);
    strncpy(mKey, key.data(), sizeof(mKey));
}

void PingerConfigEncryption::saveKey()
{
    Pinger *pinger = Pinger::getPingerTask();
    if (pinger != NULL)
    {
        OsConfigDb *cfg = pinger->getConfigDb();
        if (cfg != NULL)
        {
            cfg->set("PHONESET_DECRYPT_KEY", UtlString(mKey));
        }
    }
}

const PingerConfigEncryptionEntry *PingerConfigEncryption::getProfileEntry(const char *idLabel)
{
    const PingerConfigEncryptionEntry *profile = NULL;
    if (idLabel != NULL)
    {
        const char *filename = idLabel; 
        UtlString location;
        PingerInfo::getFlashFileSystemLocation(location);
        if (strncmp(idLabel, location.data(), location.length()) == 0)
        {
            filename = idLabel + location.length() + 1; // +1 for path separator
        }

        for (int i = 0; i < ENCRYPTED_PROFILE_COUNT; i++)
        {
           if (strcmp(filename, gEncryptedProfiles[i].profileId) == 0)
           {
               profile = &gEncryptedProfiles[i];
               break;
           }
        }
    }
    
    return profile;
}


OsStatus PingerConfigEncryption::replaceIncomingProfile(const char *profile, OsFile &incomingFile, 
                                                        int profileNdx)
{
    OsStatus status = incomingFile.open(OsFile::READ_ONLY);
    if (status == OS_SUCCESS)
    {
       
		unsigned long fileLen = 0;
        status = incomingFile.getLength(fileLen);
        if (status == OS_SUCCESS && fileLen > 0)
        {
            int buffLen = (int)fileLen;
            char *buff = new char[buffLen + 16]; // HACK: alloc extra bytes or 
			                                     // else crash on win32!
            unsigned long bytesRead;
            incomingFile.read(buff, (unsigned long)fileLen, bytesRead);
            if ((int) bytesRead != buffLen)
            {
                OsSysLog::add(FAC_AUTH, PRI_ERR, "io error reading profile file");
                status = OS_FAILED;
            }
            else
            {
                OsEncryption e;
                e.setKey((const unsigned char *)mKey, strlen(mKey));
                e.setDataPointer((unsigned char *)buff, buffLen);
                status = e.decrypt();
                if (status == OS_SUCCESS)
                {
                    OsConfigDb cfg;
                    cfg.loadFromBuffer((const char *)e.getResults());
                    if (profileNdx == PCE_DEVICE_CONFIG)
                    {
                        initWriteEnabled(&cfg);
                    }
                    status = cfg.storeToFile(profile);
                    incomingFile.remove(true);
                }
            }
            delete [] buff;
        }
        
		incomingFile.close();
    }
    
    return status;
}

void PingerConfigEncryption::setEncryptionSettings(OsEncryption *e, char *buff, int buffLen)
{
    e->setResultsHeader(gEncryptedHeader, sizeof(gEncryptedHeader));
    e->setKey((unsigned char *)mSerialNum.data(), mSerialNum.length());
    e->setDataPointer((unsigned char *)buff, buffLen);
}
