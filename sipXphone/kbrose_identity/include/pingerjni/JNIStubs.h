// $Id: //depot/OPENDEV/sipXphone/include/pingerjni/JNIStubs.h#2 $
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
// stubs for building non-Java wnt apps. to use, #define _NONJAVA in the wnt project 
// and include this file as following:
//
// #ifdef _NONJAVA
// #include "pingerjni/JNIStubs.h"
// #endif
//
// The file src/pingerjni/wnt/JNIStubs.cpp should also be included in the wnt project.
//


#ifndef _JNIStubs_h_
#define _JNIStubs_h_

int		JNI_installJavaApplication(const char *szClassnameOrURL);
int		JNI_uninstallJavaApplication(const char *szClassnameOrURL);
void	JNI_queryInstalledJavaApplications(int iMaxItems, char *szClassnameOrURL[], int& iActualItems);
void    JNI_queryLoadedApplications(int iMaxItems, char *szClassnameOrURL[], int& iActualItems) ;
void	JNI_dumpInstalledJavaApplications();

int		tcasInDisplayMode4x20();
int		JPingerStart();

#endif /* _JNIStubs_h_ */