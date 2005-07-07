// $Id: //depot/OPENDEV/sipXphone/include/pingerjni/SpeedDialWebDSP.h#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SPEED_DIAL_WEB_DSP_H
#define _SPEED_DIAL_WEB_DSP_H

/*
 * Class:     SpeedDialWebDSP
 * Method:    addSpeeddialEntry
 */
extern "C" 
void JNI_addSpeeddialEntry (const char*  strId, 
						const char*  strLabel,
						const char*  strNumber, 
						const char*  strUrl) ;

/*
 * Class:     SpeedDialWebDSP
 * Method:    getSpeeddialIdEntries
 */
extern "C" 
void JNI_getSpeeddialEntries(const char* strType, int iMaxItems, char *entries[], int& iActualItems) ;


/*
 * Class:     SpeedDialWebDSP
 * Method:    deleteSpeeddialEntry
 */
extern "C" 
void JNI_deleteSpeeddialEntry (const char* strId) ;

/* delete all entries
 * Class:     SpeedDialWebDSP
 * Method:    deleteAllSpeeddialEntries
 */

extern "C" 
void JNI_deleteAllSpeeddialEntries ();


/*
 * Class:     SpeedDialWebDSP
 * Method:    doesIDAlreadyExist
 */
extern "C" 
int JNI_doesIDAlreadyExist (const char* strId) ;

/*
 * Class:     SpeedDialWebDSP
 * Method:    isSpeedDialReadOnly
 */
extern "C" 
int JNI_isSpeedDialReadOnly () ;



/*
 * Class:     SpeedDialWebDSP
 * Method:    getNextAvailableID
 */
extern "C" 
int JNI_getNextAvailableID();

/*
 * Class:     SpeedDialWebDSP
 * Method:    getNumberOfEntries
 */
extern "C" 
int JNI_getNumberOfEntries();



/*
 * Class:     SpeedDialWebDSP
 * Method:    validateEntry
 */
extern "C" 
char* JNI_validateEntry(const char* strID, const char* strLabel, 
					  const char* strPhoneNumber, const char* strURL, 
					  int isPhoneNumber , const char* strNewLine );




#endif // #ifndef _CALL_LISTENER_HELPERS_H
