// $Id: //depot/OPENDEV/sipXphone/include/web/Speeddial.h#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _Speeddial_h_
#define _Speeddial_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "pinger/Pinger.h"
#include "os/OsDefs.h"
#include "os/OsBSem.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS                     
// EXTERNAL VARIABLES
// CONSTANTS
#define SPEEDDIAL_ADD	"ADD"
#define SPEEDDIAL_EDIT	"EDIT"
#define SPEEDDIAL_ID	"ID"
#define SPEEDDIAL_LABEL	"LABEL"
#define SPEEDDIAL_NUMBER "NUMBER"
#define SPEEDDIAL_URL	 "URL"

// Speeddial data
#define MAX_SPEEDDIAL_ENTRIES 100
#define MAX_SPEEDDIAL_LENGTH 256
#define SPEED_DIAL_SCHEMA_ID "id"
#define SPEED_DIAL_SCHEMA_LABEL "label"
#define SPEED_DIAL_SCHEMA_PHONE_NUMBER "phone_number"
#define SPEED_DIAL_SCHEMA_URL "url"
#define METHOD_ADD	"ADD"
#define METHOD_EDIT "EDIT"
#define NUM_SPEEDIAL_NEW_ENTRIES 5

#define SPEED_DIAL_FORM_ID "id"
#define SPEED_DIAL_FORM_LABEL "label"
#define SPEED_DIAL_FORM_NUMBER "number"
#define SPEED_DIAL_FORM_PHONENUMBER "phonenumber"
#define SPEED_DIAL_FORM_URL "url"
#define SPEED_DIAL_FORM_DELETE "delete"


// Application data
#define MAX_APPS 15



// STRUCTS
//struct to contain speeddialid, label, addresss, isDeleteChecked
typedef struct tagSPEED_DIAL_INFO {
	char szID[MAX_SPEEDDIAL_LENGTH] ;
	char szLabel[MAX_SPEEDDIAL_LENGTH] ;
	char szAddress[MAX_SPEEDDIAL_LENGTH];
	char szIsDeleteChecked[MAX_SPEEDDIAL_LENGTH]; //off is false, on is true
} SPEED_DIAL_INFO ;

// TYPEDEFS

// FORWARD DECLARATIONS

//:Speeddial class
class Speeddial
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

   static Speeddial* getSpeeddialTask(void);
     //:Return a pointer to the Speeddial task, creating it if necessary

   virtual
   ~Speeddial();
     //:Destructor

   void initSpeeddial(HttpServer* pHttpServer);

/* ============================ MANIPULATORS ============================== */

	static int add(UtlString strMethod, 
		UtlString strId, UtlString strLabel, UtlString strNumber, 
		UtlString strUrl, UtlString strMessage, int iNext) ;

    static int Speeddial::deleteEntry(UtlString strId) ;

	static int Speeddial::getAll(char *arrSpeedDialId[], char *arrSpeedDialLabel[], 
					  char *arrSpeedDialPhoneNumber[], char *arrSpeedDialURL[], int &iNumEntries) ;
 

/* ============================ ACCESSORS ================================= */



/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   Speeddial();
     //:Constructor (called only indirectly via getSpeeddialTask())
     // We identify this as a protected (rather than a private) method so
     // that gcc doesn't complain that the class only defines a private
     // constructor and has no friends.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
  

   HttpServer*       mpHttpServer;       // Http Server

   // Static data members used to enforce Singleton behavior
   static Speeddial*    spInstance;    // pointer to the single instance of
                                    //  the Speeddial class
   static OsBSem     sLock;         // semaphore used to ensure that there
                                    //  is only one instance of this class   

private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _Speeddial_h_


