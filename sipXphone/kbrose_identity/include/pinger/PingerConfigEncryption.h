// $Id: //depot/OPENDEV/sipXphone/include/pinger/PingerConfigEncryption.h#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _PingerConfigEncryption_h_
#define _PingerConfigEncryption_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsFS.h"
#include "os/OsConfigEncryption.h"
#include "os/OsEncryption.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define PCE_INCOMING_PROFILE_SUFFIX "_incoming"
#define PCE_MAX_USER_LEN  128
#define PCE_MAX_PASSWORD_LEN  128

// Leave room for MD5 padding and ":" separator
#define PCE_MAX_KEY_LEN  PCE_MAX_USER_LEN + PCE_MAX_PASSWORD_LEN + 64

#define PCE_MAX_PROFILE_NAME_LEN  32

#define PCE_MAX_PROFILE_SCAN_LEN  64


// STRUCTS
struct PingerConfigEncryptionEntry
{
    char profileId[PCE_MAX_PROFILE_NAME_LEN];
    char encryptedContentScanTest[PCE_MAX_PROFILE_SCAN_LEN];
};

// TYPEDEFS
// FORWARD DECLARATIONS

/*! Encryption of rules of profiles read/written on phone, what
  profiles get encrypted, and when, what the key is.
 */
class PingerConfigEncryption : public OsConfigEncryption
{
 public:
    PingerConfigEncryption();
    virtual ~PingerConfigEncryption() ;
    
    //! called after PingerConfig has loaded config files
    void initWriteEnabled(OsConfigDb *cfg);

    //! called after PingerConfig has loaded config files
    void initConfig(OsConfigDb *cfg);

    //! called after PingerConfig has loaded config files
    void initLocalConfig(OsConfigDb *cfg);

    //! tests a buffer if its been encrypted
    UtlBoolean isEncrypted(const char *profile, const char *buffer, int buffLen);

    //! tests a buffer if its been encrypted
    UtlBoolean isEncrypted(OsConfigDb *cfg, const char *buffer, int buffLen);

    //! tests a buffer if its been encrypted
    UtlBoolean isEncryptedByConfigServer(const char *profile, const char *buffer, int buffLen);

    //! is this profile typically encrypted , pinger-config yes, local-config no
    UtlBoolean isNormallyEncrypted(OsConfigDb *cfg);

    //! encrypt whats in this buffer, setting the proper encryption key
    OsStatus encrypt(OsConfigDb *cfg, OsEncryption *e, char *buffer, int buffLen);

    //! decrypt whats in this buffer, setting the proper decryption key
    OsStatus decrypt(OsConfigDb *cfg, OsEncryption *e, char *buffer, int buffLen);
    
    //! @return Should profiles be written out as encrypted?
    UtlBoolean isWriteEncryptedEnabled();

    //! @return bytes in initial N bytes of the encrypted file
    // const unsigned char *PingerConfigEncryption::getFileHeader(int& headerLen);

    /*! SipConfigDeviveAgent may have left some incoming profiles to be
     decrypted and subsequently used as replacement for current
     profiles. Check for this now and prompt user if nec. for
     password to decrypt profiles consequently this can only be
     called after Java layer has been started.  
     
     returns TRUE if one or more profiles were handled and system
     should restart
    */
    UtlBoolean handleIncomingProfiles();
    
 private:

    //! Launch java GUI for info
    UtlBoolean promptUserForKey(int badPasswordAttempts);

    void buildKey();

    void saveKey();

    const PingerConfigEncryptionEntry *getProfileEntry(const char *idLabel);

    void setEncryptionSettings(OsEncryption *e, char *buff, int buffLen);

    OsStatus replaceIncomingProfile(const char *profile, OsFile &incomingFile, int profileNdx);

    char mKey[PCE_MAX_KEY_LEN];

    UtlString mSerialNum;

    char mUser[PCE_MAX_USER_LEN];

    char mPassword[PCE_MAX_PASSWORD_LEN];

    UtlBoolean mUserCanceledPassword;

    UtlBoolean mWriteEncryptedEnabled;
};

#endif // _PingerConfigEncryption_h_
