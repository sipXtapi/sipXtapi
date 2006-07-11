// $Id: //depot/OPENDEV/sipXphone/src/web/AppsList.cpp#3 $
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
#include "web/AppsList.h"
#include <os/iostream>
#include <assert.h>

#ifdef _WIN32
#   include <io.h>
#elif defined(__pingtel_on_posix__)
#   include <stdlib.h>
#   include <unistd.h>
#endif

// APPLICATION INCLUDES
#include "net/HttpServer.h"
#include "net/HttpMessage.h"
#include "os/OsConnectionSocket.h"
//#include "rw/regexp.h"
//#include "pingerjni/SpeedDialWebDSP.h"
//#include "pingerjni/JXAPI.h"
//#include "web/MyXpressaHtml.h"
//#include "web/Webui.h"
//#include "web/Webutil.h"

#ifdef _NONJAVA
#include "pingerjni/JNIStubs.h"
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define APPLICATIONS_HOST		"my.pingtel.com"
//#define APPLICATIONS_HOST		"" 
// TBD: for testing only
#define APPLICATIONS_PORT		"80"
//#define APPLICATIONS_PORT		"90" 
// TBD: for testing only
#define APPLICATIONS_URL		"/AppList"


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
AppsList::AppsList()
{


}

// Copy constructor
AppsList::AppsList(const AppsList& rAppsList)
{


}


// Destructor
AppsList::~AppsList()
{
	int i ;
	for (i=0; i<MAX_ALL_APPS; i++) {
        free(szPotentialApps[i]) ;
	}

	for (i=0; i<MAX_ALL_APPS; i++) {
        free(szPotentialApps[i]) ;
    }
	
	for (i=0; i<MAX_ALL_APPS; i++) {
        free(szPotentialUrls[i]);
    }
}

/*============================ MANIPULATORS =============================== */
// Assignment operator
AppsList& 
AppsList::operator=(const AppsList& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */




/* ============================ INQUIRY =================================== */

/**
 * This method returns the application name associated an index
 *   iIndex - position of app in array
 *   returned - application name
 */
char* AppsList::getApplicationName(int iIndex) 
{
	return szPotentialApps[iIndex] ;

}

/**
 * This method returns the application URL associated an index
 *   iIndex - position of app in array
 *   returned - application URL
 */
char* AppsList::getApplicationUrl(int iIndex) 
{
	return szPotentialUrls[iIndex] ;
}

/**
 * This method returns the application ID associated an index
 *   iIndex - position of app in array
 *   returned - application Id
 */
char* AppsList::getApplicationId(int iIndex) 
{
	return szPotentialIds[iIndex] ;
}

/**
 * Return total number of applications on server
 */
int AppsList::getCount() 
{
	return iNumApps ;
}

  
/**
 * This method makes a request to the Pingtel web site to get a list of
 * current applications
 *
 * The returned data from the server is put into the internal data structure
 */
void AppsList::getApplications() 
{
	
#define APP_SEPARATOR ","
#define APP_LINE_SEPARATOR ";"

	int i ;

	for (i=0; i<MAX_ALL_APPS; i++) {
            szPotentialIds[i] = (char*) malloc(MAX_URL_LENGTH);
            memset(szPotentialIds[i], 0, MAX_URL_LENGTH);
    }

	for (i=0; i<MAX_ALL_APPS; i++) {
            szPotentialApps[i] = (char*) malloc(MAX_URL_LENGTH);
            memset(szPotentialApps[i], 0, MAX_URL_LENGTH);
    }
	
	for (i=0; i<MAX_ALL_APPS; i++) {
            szPotentialUrls[i] = (char*) malloc(MAX_URL_LENGTH);
            memset(szPotentialUrls[i], 0, MAX_URL_LENGTH);
    }

	// Create request
		HttpMessage *pRequest = new HttpMessage();
		pRequest->setFirstHeaderLine("GET", APPLICATIONS_URL, HTTP_PROTOCOL_VERSION);
		pRequest->addHeaderField("Accept", "*/*");
		pRequest->addHeaderField("Accept-Language", "en-us;q=0.5");
		pRequest->addHeaderField("User-Agent", "Mozilla/4.0 (compatible)");
		pRequest->addHeaderField("Host", APPLICATIONS_HOST);
		pRequest->addHeaderField("Connection", "Keep-Alive");

		OsConnectionSocket *pSocket;
		int iPort = atoi((const char*) APPLICATIONS_PORT) ;
		pSocket = new OsConnectionSocket(iPort, APPLICATIONS_HOST);

		int charsRead = 0 ;

		if (pSocket && pSocket->isOk())
		{
			if (pRequest->write(pSocket))
			{
					//timeout after 1 min if MyPingtel does not respond. 
					if(pSocket->isReadyToRead(60000))
					{
						// Now that we have started to receive data from MyPingtel,
						// wait for a second before reading data, so as to make sure that
						// we read the entire content.
						#if defined(_WIN32)
							Sleep(1000);
						#else
							sleep(1000);
						#endif
				
						charsRead = pRequest->read(pSocket);
						
					}
			}
			pSocket->close();
			delete pSocket;
		}
		else
		{
			charsRead = -1;	// couldn't talk to the server
			osPrintf("queryListofApplications failed to talk to server\n") ;
			return ;
		}

		int status = pRequest->getResponseStatusCode();
		if (status != HTTP_OK_CODE)
		{
			printf("queryListofApplications ERROR: Response with code %d.\nExiting...\n", status);
			delete pRequest;
			return ;
		}

		osPrintf("queryListofApplications read %d bytes from socket\n", charsRead) ;

		UtlString buffer;
		HttpBody *pBody = (HttpBody *)pRequest->getBody();
		if (pBody && charsRead>=0)
		{
			pBody->getBytes(&buffer, &charsRead);
		}

		/*int contentLength = pRequest->getContentLength();
		if (charsRead != contentLength)
		{
			printf("queryListofApplications ERROR: Content length %d does not match bytes %d received."
			   "\nExiting...\n", contentLength, charsRead);
			delete pRequest;
			return ;
		}*/

		delete pRequest;

		
		//Put response into our return array

		// While not end of file, get next substring up to ,put into array, get next substring up to ;

		UtlString strLine ;
		int iSeparator = 0 ;	//position of separator
		int iCount = 0 ;		//number of responses
		while (buffer.length()>0) {
			if ( (unsigned int) (iSeparator = buffer.index(APP_SEPARATOR))!=UTL_NOT_FOUND) {

				strLine = buffer(0, iSeparator) ;
				strcpy(szPotentialIds[iCount], strLine.data()) ;
				buffer.remove(0, iSeparator+1) ; // remove separator also 

				iSeparator = buffer.index(APP_SEPARATOR) ;
				strLine = buffer(0, iSeparator) ;
				strcpy(szPotentialApps[iCount], strLine.data()) ;
				buffer.remove(0, iSeparator+1) ; // Remove separator also

				osPrintf("szPotentialapp=%s\n", szPotentialApps[iCount]) ;

				iSeparator = buffer.index(APP_LINE_SEPARATOR) ;
				strLine = buffer(0, iSeparator) ;
				strcpy(szPotentialUrls[iCount], strLine.data()) ;
				buffer.remove(0, iSeparator+2) ; // Remove separator also

				osPrintf("szPotentialUrls=%s\n", szPotentialUrls[iCount]) ;
				iCount++ ;
			} else {
				buffer.remove(0,buffer.length()) ;
			}

		}
	
		iNumApps = iCount ;

}

/**
 * Get the Application ID associated with the application name
 * NOTE: this is a temporary approach - there is no way to guarantee
 * the uniqueness of the application name currently.
 * A better approach would be to have a unique ID shared between 
 * client and server. 
 *
 * appName - application name as provided by the list we got from the server
 * Return - ID as UtlString, or empty string
 *
 */
UtlString AppsList::getApplicationById(const char* appName) 
{
	UtlString strReturn ;
	
	for (int i=0; i< iNumApps; i++) {
		if (strcmp(appName, szPotentialApps[i])==0) {
			strReturn.append(szPotentialIds[i]) ;
			return strReturn ;
		}
	}
	
	return strReturn ;

}

/**
 * Get the Application ID associated with the application name
 * NOTE: this is a temporary approach - there is no way to guarantee
 * the uniqueness of the application name currently.
 * A better approach would be to have a unique ID shared between 
 * client and server. 
 *
 * appName - application name as provided by the list we got from the server
 * Return - ID as UtlString, or empty string
 *
 */
UtlString AppsList::getApplicationUrlbyName(const char* appName) 
{
	UtlString strReturn ;
	
	for (int i=0; i< iNumApps; i++) {
		if (strcmp(appName, szPotentialApps[i])==0) {
			strReturn.append(szPotentialUrls[i]) ;
			return strReturn ;
		}
	}
	
	return strReturn ;

}

/** 
 * Get an alphabetically order list of applications
 *
 * vSortedNames - RW vector of names that is returned
 */
void AppsList::getSortedApplicationNames(UtlSortedList *vSortedNames)
{
	for (int i=0 ; i< iNumApps; i++) {
		vSortedNames->insert(new UtlString(szPotentialApps[i])) ;
	}
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */

	char* szPotentialIds[MAX_ALL_APPS] ;  // apps on server 
	char* szPotentialApps[MAX_ALL_APPS] ; // apps on server
	char* szPotentialUrls[MAX_ALL_APPS] ; // apps on server
	int iNumApps ; // Number of apps
