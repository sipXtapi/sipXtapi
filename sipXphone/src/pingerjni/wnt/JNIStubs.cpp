// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/wnt/JNIStubs.cpp#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

//
// Stubs for building non-Java wnt apps. to use, add this file to the wnt project.
// Also, #define _NONJAVA in the wnt project and include the JNIStubs.h file as following:
//
// #ifdef _NONJAVA
// #include "pingerjni/JNIStubs.h"
// #endif
//
//

/*****************************************************************************
 */
int JNI_installJavaApplication(const char *szClassnameOrURL)
{
	return 0 ;
}


/*****************************************************************************
 */
int JNI_uninstallJavaApplication(const char *szClassnameOrURL)
{
	return 0 ;
}


/*****************************************************************************
 */
void JNI_queryInstalledJavaApplications(int iMaxItems, char *szClassnameOrURL[], int& iActualItems)
{
	return;
}


/*****************************************************************************
 */
void JNI_queryLoadedApplications(int iMaxItems, char *szClassnameOrURL[], int& iActualItems) 
{
	return;
}

/*****************************************************************************
 */
void JNI_dumpInstalledJavaApplications()
{
	return;
}

/*****************************************************************************
 */
int tcasInDisplayMode4x20()
{
	return 0;
}

/*****************************************************************************
 */
int JPingerStart()
{
	return 0 ;
}



/*****************************************************************************
 */
extern "C" void JXAPI_DisplayStatus(const char* szStatus) 
{ 
    return ;
}

/*****************************************************************************
 */
extern "C" void JXAPI_ClearStatus()
{ 
    return ;
}


/*****************************************************************************
 */
extern "C" void JXAPI_doGarbageCollect()
{ 
    return ;
}


/*****************************************************************************
 */
extern "C" void JXAPI_SimpleDial(const char* szSIPURL, char* szCallID, int nCallID)
{ 
    return ;
}


/*****************************************************************************
 */
extern "C" void JXAPI_Transfer(const char* szSIPURL, char* szCallID)
{ 
    return ;
}


/*****************************************************************************
 */
extern "C" void JXAPI_DropCall(const char* szCallId)
{ 
    return ;
}

 
/*****************************************************************************
 */
extern "C" int JXAPI_MessageBox(MBT enumType, char* szTitle, char* szText, UtlBoolean bBlocking)
{ 
    return -1 ;
}

