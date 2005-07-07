// $Id: //depot/OPENDEV/sipXphone/include/web/AppsList.h#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _AppsList_h_
#define _AppsList_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "pinger/Pinger.h"
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "utl/UtlSortedList.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS                     
// EXTERNAL VARIABLES
// CONSTANTS
#define MAX_ALL_APPS 100
#define MAX_URL_LENGTH 256

// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

//:AppsList class
class AppsList
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */


   AppsList();
     //:Constructor
   ~AppsList();
     //:Destructor

   AppsList(const AppsList& rAppsList);
     //:Copy constructor
   AppsList& operator=(const AppsList& rhs);
	 //:Assignment operator


/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */

   void AppsList::getApplications() ;
     // Get a list of all applications on server

   char* AppsList::getApplicationName(int iIndex) ;
     // Get an application name by an index

   char* AppsList::getApplicationUrl(int iIndex) ;
     // Get an application URL by an index

   char* AppsList::getApplicationId(int iIndex) ;
     // Get an application ID by an index

   int AppsList::getCount() ;
     // Return number of applications in list

   UtlString AppsList::getApplicationById(const char* appName) ;
     // Get application ID

   void AppsList::getSortedApplicationNames(UtlSortedList *vSortedNames) ;
	 // Get List of application names

   UtlString AppsList::getApplicationUrlbyName(const char* appName) ;
     // Get an application URL, given its name

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */


protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
  

private:

	char* szPotentialIds[MAX_ALL_APPS] ;  // apps on server
	char* szPotentialApps[MAX_ALL_APPS] ; // apps on server
	char* szPotentialUrls[MAX_ALL_APPS] ; // apps on server
	int iNumApps ; // Number of apps

};

/* ============================ INLINE METHODS ============================ */

#endif  // _AppsList_h_


