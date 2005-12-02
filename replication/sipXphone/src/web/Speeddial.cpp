// $Id: //depot/OPENDEV/sipXphone/src/web/Speeddial.cpp#3 $
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
#include "web/Speeddial.h"
#include "pinger/Pinger.h"
#include <os/iostream>

#include <assert.h>

#ifdef _WIN32
#   include <io.h>
#elif defined(__pingtel_on_posix__)
#   include <malloc.h>
#endif
#include <time.h>

// APPLICATION INCLUDES
#include "pingerjni/SpeedDialWebDSP.h"
#include "pingerjni/JXAPI.h"
#include "web/Webui.h"
#include "web/Webutil.h"

#ifdef _NONJAVA
#include "pingerjni/JNIStubs.h"
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS


// STATIC VARIABLE INITIALIZATIONS
Speeddial* Speeddial::spInstance = 0;
OsBSem  Speeddial::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Return a pointer to the Speeddial task, creating it if necessary
Speeddial* Speeddial::getSpeeddialTask(void)
{

   // If the task object already exists, and the corresponding low-level task
   // has been started, then use it
   if (spInstance != NULL)
      return spInstance;

   // If the task does not yet exist or hasn't been started, then acquire
   // the lock to ensure that only one instance of the task is started
   sLock.acquire();
   if (spInstance == NULL)
       spInstance = new Speeddial();
   sLock.release();

   return spInstance;
}

// Destructor
Speeddial::~Speeddial()
{
	delete spInstance ;
}


/* ============================ ACCESSORS ================================= */


/**
 * Add a speeddial entry
 * 
 * strMethod, type of entry
 *    ADD : are adding a new entry
 *    EDIT: are editing an entry
 * strId, ID of speeddial
 * strLabel, Label 
 * strNumber Phone number - digits
 * strUrl SIP URL 
 * iNext 0 - use strId, 1 - use next free ID
 * Returns:
 * strMessage status message
 * int 0=OK, -1=Error
 */

int Speeddial::add(UtlString strMethod, UtlString strId, UtlString strLabel, UtlString strNumber, 
	UtlString strUrl, UtlString strMessage, int iNext=0) 
{
	// Check whether user wants to use next value as speeddial ID
	int iSpeeddialId = 0;							// Number of speeddial entries already assigned
	char* arrSpeedDialId[MAX_SPEEDDIAL_ENTRIES];	// Array of speeddial IDs
	int i ;
	int iError = 0 ;
	for (i=0; i<MAX_SPEEDDIAL_ENTRIES; i++) {	// Set initial array values
		arrSpeedDialId[i] = (char*) malloc(MAX_SPEEDDIAL_LENGTH);
		memset(arrSpeedDialId[i], 0, MAX_SPEEDDIAL_LENGTH);
    }


	JNI_getSpeeddialEntries(SPEED_DIAL_SCHEMA_ID, MAX_SPEEDDIAL_ENTRIES, arrSpeedDialId, iSpeeddialId) ;

	// validate parameters for speecdial entry 
	for (int j = 0; j<MAX_SPEEDDIAL_ENTRIES;j++) {
		if (strMethod.compareTo(METHOD_EDIT)!=0 && strId.length()!=0 && strId.compareTo(arrSpeedDialId[j])==0) {
			strMessage.append("This speed dial number already exists") ;
			iError = 1;
		}
	}

	if(iError == 0)
	{
		if(strId.length()==0)
		{
			strMessage.append("Must specify a phone number or URL value") ;
			iError = 2 ; 
		}
		else if(strNumber.length()==0 && strUrl.length()==4 && strLabel.length()==0)
		{
			strMessage.append("Must specify a phone number or URL value") ;
			iError = 3 ; 
		}
		else if (strNumber.length()==0 && strUrl.length()==4) {
			strMessage.append("Must specify a phone number or URL value") ;
			iError = 4 ; 
		} 
		else if (strLabel.length()==0) {
			strMessage.append("Must specify a Label") ;
			iError = 5 ;
		}
	}

	// enter speeddial number
	if (iError==0) {
#ifdef DEBUG_WEB_UI
	osPrintf("Webui::no error found %s\n", strMessage.data());
#endif

	/* Removed by Harippriya on Aug 9, 2001. -- users can now select their own speed dial id.

		if (iNext==1) {
			// Find next free Speeddial ID
			i = 1 ;
			int iCompare = 0 ;
			while (i<MAX_SPEEDDIAL_ENTRIES && iFound!=0) {
				for (int j=0; j<iSpeeddialId;j++) {
					sscanf( arrSpeedDialId[j], "%d", &iCompare );
					if (i == iCompare) {
						iFound = 1 ;
					}
				}
				if (iFound !=1) {
					iFound = 0 ;
					iNextId = i ;
				} else {
					iFound = -1 ;
				}
				i++ ;
			}
			char buf[4] ;
			sprintf(buf, "%d", iNextId) ;
			strId.remove(0) ;
			strId.append(buf) ;
		}
		*/

		if (strNumber.length()>0) {
			JNI_addSpeeddialEntry((const char*)strId.data(), 
							  (const char*)strLabel.data(), 
							  (const char*)strNumber.data(),
							  (const char*) "") ;
		} else {
			JNI_addSpeeddialEntry((const char*)strId.data(), 
							  (const char*)strLabel.data(), 
							  (const char*) "", 
							  (const char*)strUrl.data()) ;
		}

		strMessage.append("SpeedDial Entry Successful") ;
		return 0 ;
	} else {
		return iError ;
	}

}


/**
 * Delete a speeddial entry
 * 
 * strMethod, strId ID of speeddial
 * int 0=OK, -1=Error
 */
int Speeddial::deleteEntry(UtlString strId) 
{
	if (strId.length()!=0) {
		JNI_deleteSpeeddialEntry((const char*) strId.data()) ;
	}

	return 0 ; //Note: can't fail currently
}

int Speeddial::getAll(char *arrSpeedDialId[], char *arrSpeedDialLabel[], 
					  char *arrSpeedDialPhoneNumber[], char *arrSpeedDialURL[], int &iNumEntries) 
{

	int iSpeeddialId = 0 ;
	int iSpeeddialLabel = 0 ;
	int iSpeeddialPhoneNumber = 0 ;
	int iSpeeddialUrl = 0 ;
	
	int iNumOfEntries = JNI_getNumberOfEntries();
	if( iNumOfEntries > 0 ){
		
		for (int i=0; i<iNumOfEntries; i++) {
            arrSpeedDialId[i] = new char[MAX_SPEEDDIAL_LENGTH];
            memset(arrSpeedDialId[i], 0, MAX_SPEEDDIAL_LENGTH);
            
			arrSpeedDialLabel[i] = new char[MAX_SPEEDDIAL_LENGTH];
            memset(arrSpeedDialLabel[i], 0, MAX_SPEEDDIAL_LENGTH);
			
			arrSpeedDialURL[i] = new char[MAX_SPEEDDIAL_LENGTH];
            memset(arrSpeedDialURL[i], 0, MAX_SPEEDDIAL_LENGTH);
			
			arrSpeedDialPhoneNumber[i] = new char[MAX_SPEEDDIAL_LENGTH];
            memset(arrSpeedDialPhoneNumber[i], 0, MAX_SPEEDDIAL_LENGTH);
		}

		JNI_getSpeeddialEntries(SPEED_DIAL_SCHEMA_ID, iNumOfEntries, arrSpeedDialId, iSpeeddialId) ;
		JNI_getSpeeddialEntries(SPEED_DIAL_SCHEMA_LABEL, iNumOfEntries, arrSpeedDialLabel, iSpeeddialLabel) ;
		JNI_getSpeeddialEntries(SPEED_DIAL_SCHEMA_PHONE_NUMBER, iNumOfEntries, arrSpeedDialPhoneNumber, iSpeeddialPhoneNumber) ;
		JNI_getSpeeddialEntries(SPEED_DIAL_SCHEMA_URL, iNumOfEntries, arrSpeedDialURL, iSpeeddialUrl) ;


	}
	
	if ((iSpeeddialId!=iSpeeddialLabel)||(iSpeeddialPhoneNumber!=iSpeeddialUrl)) {
		return -1 ;
		osPrintf("Speeddial::getAll - incorrrect number of results returned\n") ;
	} else {
		iNumEntries = iSpeeddialId ;
		return 0 ;
	}

	
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Default constructor (called only indirectly via getPingerTask())
Speeddial::Speeddial()
{

}
/* //////////////////////////// PRIVATE /////////////////////////////////// */

