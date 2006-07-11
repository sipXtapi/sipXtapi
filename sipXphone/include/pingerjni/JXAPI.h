// $Id: //depot/OPENDEV/sipXphone/include/pingerjni/JXAPI.h#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _JXAPI_h_
#define _JXAPI_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
#define MBT_INFORMATIONAL   0
#define MBT_WARNING         1
#define MBT_ERROR           2

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

/* ============================ FUNCTIONS ================================= */

extern "C" void JXAPI_DisplayStatus(const char* szStatus) ;
    //:Displays a blurb of status text on the phone's LCD display.  The user
    // is blocked until the status is removed using JXAPI_ClearStatus.

extern "C" void JXAPI_ClearStatus() ;
    //:Clear any status displayed by calling JXAPI_DisplayStatus.

extern "C" void JXAPI_doGarbageCollect() ;
    //:Request Java Garbage Collection

extern "C" void JXAPI_onConfigurationChanged() ;
    //:Tells Java that the configuration has been changed

extern "C" void JXAPI_SimpleDial(const char* szSIPURL, char* szCallID, int nCallID) ;
    //:Dials the specified sip URL and returns the call id in the szCallID 
    //!parameter.  The nCallID should reflect the size of the szCallID buffer.

extern "C" void JXAPI_Transfer(const char* szSIPURL, const char* szCallID) ;
    //:Transfers the designated call (identified by szCallID) to the 
    //!designated SIP URL.

extern "C" void JXAPI_Answer(const char* szCallID) ;
    //: answer the call with the given callID

extern "C" void JXAPI_Hold(const char* szCallID, bool bEnable) ;
    //: holds/unholds the call with the given callID if bEnable is true/false
   

extern "C" void JXAPI_DropCall(const char* szCallId) ;
    //:Drops the called identified the the designated szCallID.

extern "C" void JXAPI_AddParty(const char* szSIPURL, const char* szCallID) ;
    //:adds the party with the address szSIPURL to the call (identified by szCallID) 

extern "C" void JXAPI_RemoveParty(const char* szSIPURL, const char* szCallID) ;
    //:Removes the party with the address szSIPURL fromthe call (identified by szCallID) 



extern "C" void JXAPI_setUploadingFile(UtlBoolean bUploadingFile) ;
    //:toggles ability to receive calls
 
extern "C" int JXAPI_AuthorizeProfiles(int passwordAttempts,
                                       char* username, 
                                       int maxUsernameLen,
                                       char* password, 
                                       int maxPasswordLen);
    //:User name and password prompt to decrypt profiles sent from
    //config server

extern "C" int JXAPI_MessageBox(    int         iType, 
                                    const char* szTitle, 
                                    const char* szText, 
                                    UtlBoolean   bBlocking) ;
    //:Displays either a block of non-blocking message box.  The szTitle and 
    //!szText cannot be NULL.
extern "C" UtlBoolean JXAPI_GetCoreAppVersion(char *versionString) ;
extern "C" UtlBoolean JXAPI_GetJVMFreeMemory(unsigned long& ulFreeMemory) ;
extern "C" UtlBoolean JXAPI_GetJVMTotalMemory(unsigned long& ulTotalMemory) ;


extern "C" UtlBoolean JXAPI_GetXpressaResource(const char* szResource, char** pData, int& iLength) ;
    //:Pulls at name file/resource out of the pingel.jar file and returns it 
    // as a pile of bits.  This method assumes that lib/resources is the
    // root and all gets must be relative to that.
    //!param szResource (IN) - The resource that will be fetched.  Name and 
    //       path must be relative to lib/resources
    //!param pData (IN/OUT) - Pointer to pointer of data.  This method will
    //       allocate the data using new.  The caller is expected to delete
    //       the data when done.
    //!param iLength (OUT) - The length of data pointed to by pData.


extern "C" void JXAPI_FlushCache() ;
    //:Used to flush cache.ser in java before reboot

extern "C" UtlBoolean JXAPI_RequestRestart(int iSeconds, const char* szReason);
    //:Requests a restart from the upper layers within iSeconds

extern "C" UtlBoolean JXAPI_IsPhoneBusy();
    //:Determines if the phone is 'busy', where busy is presently assumed
    //:to be having active or held calls.  
    // In the future, this concept could expand to include active 
    // applications, measures of idleness, etc.


#endif // #ifndef _JXAPI_h_
