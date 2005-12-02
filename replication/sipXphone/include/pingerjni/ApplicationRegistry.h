// $Id: //depot/OPENDEV/sipXphone/include/pingerjni/ApplicationRegistry.h#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _APPLICATION_REGISTRY_H
#define _APPLICATION_REGISTRY_H

//DEFINES
//return codes for installing applications
#define CGI_INSTALL_APPLICATION_OK                             0
#define CGI_INSTALL_APPLICATION_ERROR_URL                      1
#define CGI_INSTALL_APPLICATION_ERROR_APP_ALREADY_INSTALLED    2
#define CGI_INSTALL_APPLICATION_ERROR_EXCEEDED_MAXIMUM_ALLOWED 3
#define CGI_INSTALL_APPLICATION_ERROR_TIMEOUT 				   4
#define CGI_INSTALL_APPLICATION_ERROR_VERSIONFAILED 		   5
#define CGI_INSTALL_APPLICATION_ERROR                         -1

//return codes for uninstalling applications
#define CGI_UNINSTALL_APPLICATION_OK                           0
#define CGI_UNINSTALL_APPLICATION_ERROR_URL                    1
#define CGI_UNINSTALL_APPLICATION_ERROR                       -1


#define MAX_TITLE_LENGTH	128
#define MAX_APPS 15
#define MAX_URL_LENGTH 256

//struct to contain app title and app url
typedef struct tagAPPINFO {
	char szUrl[MAX_URL_LENGTH] ;
	char szTitle[MAX_TITLE_LENGTH] ;
} APPINFO ;


/**
 * CGI / JNI method of installing a user application.  All applications are 
 * assumed to be of type USER.  Another method will be exposed for registering
 * CORE components.
 *
 * @param strClassnameOrURL the class name or URL of the application to 
 *        install
 *
 * @return int error code
 */
int JNI_installJavaApplication(const char *szClassnameOrURL) ;


/**
 * CGI / JNI method of uninstalling a user application.  All applications are
 * assumed to be of type USER.  Another method will be exposed for removing
 * CORE components.
 *
 * @param strClassnameOrURL the class name or URL of the application to 
 *        uninstall
 *
 * @return int error code
 */
int JNI_uninstallJavaApplication(const char *szClassnameOrURL) ;


/**
 * CGI / JNI method of querying for user applications.
 *
 * @param iMaxItems max number of items/apps we should fetch
 * @param szClassnameOrURL array of classname or urls (must have iMaxItems 
 *        strings of some arbitarily long lenth)
 * @param iActualItems the actual number of items loaded and populated into
 *        the szClassnameOrURL array.
 *
 * @return int error code
 */
void JNI_queryInstalledJavaApplications(int iMaxItems, 
		char *szClassnameOrURL[], int& iActualItems) ;


/**
 * CGI / JNI method of querying for title and urls of user applications.
 *
 * @param iMaxItems max number of items/apps we should fetch
 * @param pAppInfo array of array( that consists of title and url)(must have iMaxItems 
 *        strings of some arbitarily long lenth)
 * @param iActualItems the actual number of items loaded and populated into
 *        the szClassnameOrURL array.
 *
 * @return int error code
 */

void JNI_queryInstalledJavaApplications
	(int iMaxItems, APPINFO* pAppInfo, int& iActualItems);

/**
 * CGI / JNI method of querying loaded applications i.e. applications that can currently run
 *
 * @param iMaxItems max number of items/apps we should fetch
 * @param szClassnameOrURL array of classname or urls (must have iMaxItems 
 *        strings of some arbitarily long lenth)
 * @param iActualItems the actual number of items loaded and populated into
 *        the szClassnameOrURL array.
 *
 * @return int error code
 */
void JNI_queryLoadedApplications(int iMaxItems, char *szClassnameOrURL[], int& iActualItems) ;

/*
 *  This gives you a list of all USER apps and the apps that have title
 *  and  icons.
 * @param iMaxItems max number of items/apps we should fetch
 * @param szTitles array of titles(must have iMaxItems 
 *        strings of some arbitarily long lenth)
 * @param iActualItems the actual number of items loaded and populated into
 *        the szTitles array.
 */
void JNI_queryAllInstalledApplications(int iMaxItems, char *szTitles[], int& iActualItems);


#endif


