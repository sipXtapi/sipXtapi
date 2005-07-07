// $Id: //depot/OPENDEV/sipXphone/include/web/WebuiLineMgrUtil.h#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WEBUILINEMGRUTIL_H__0DE37037_D2A8_464D_916A_30D6FA4302D5__INCLUDED_)
#define AFX_WEBUILINEMGRUTIL_H__0DE37037_D2A8_464D_916A_30D6FA4302D5__INCLUDED_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "pinger/Pinger.h"


// DEFINES
// MACROS                     
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

//:Webutil class
// This class is used as a utility calls by Web Ui to manage the Lines.
class Url;

class WebuiLineMgrUtil  
{
public:
	WebuiLineMgrUtil();
	virtual ~WebuiLineMgrUtil();

   //VIEW LINES
   static void saveOutboundLine(UtlString& newOutBoundLine,UtlString& strErrorMsg);
	
   static void viewAllLines(UtlString& addToHtmlString);

   //ADD LINES
   static void saveAddLines(UtlString& strLineUrl, 
						UtlString& strLineCallHandling, 
						UtlString& strLineState, 
						UtlString& ReqLineUser, 
						UtlString& statusMessage,
						UtlBoolean& redirectToViewLine,
						UtlBoolean& isErrorAndStayOnSamePage);
   
   static void deleteAddLines(UtlString& sipUrl, UtlString& statusMessage);
   
   static void updateAddLines(UtlString& sipUrl,
                                      UtlString& previousSipUrl,
                                      UtlString& strLineCallHandling,
                                      UtlString& strLineState,
                                      UtlBoolean& isErrorAndStayOnSamePage,
                                      UtlBoolean& redirectToViewLine,
                                      UtlString& statusMessage);
   static void editAddLines(UtlString& userName,
                                    UtlString& ReqSipUrl,
                                    UtlString& addToHtml,
                                    UtlBoolean& isAuthorized,
                                    UtlBoolean& redirectToViewLine,
                                    UtlString& statusMessage);

   //ADD CREDENTIALS
   static void saveAddCredentials(UtlString& sipUrl,
                                          UtlString& strUserId,
                                          UtlString& strRealm,
                                          UtlString& strPassword,
                                          UtlString& strConfirmPassword,
                                          UtlString& strErrorMsg,
                                          UtlString& statusMessage,
                                          UtlBoolean& isCredentialAdded,
                                          UtlBoolean& redirectToAddLine);
   
   static void deleteAddCredentials(UtlString& sipUrl,
                                    UtlString& strRealm,
                                    UtlString& statusMessage);
               

   static UtlBoolean updateAddCredentials( UtlString& sipUrl,
                                             UtlString& sipRealm,
                                             UtlString& sipUserId,
                                             UtlString& strrealm,
                                             UtlString& strUserId,
                                             UtlString& strPassword,
                                             UtlString& strConfirmPassword,
                                             UtlString& statusMessage,
                                             UtlBoolean& redirectToAddLine );
protected:

   static UtlBoolean isRegistringToItself(Url sipUri);
   static void getUriForUrl(Url &userEnteredUrl , Url &uri);
   static void getCanonicalUrl(Url &userEnteredUrl , Url &uri , Url &canonicalUrl/*out*/);


private:

   static void makeStringsHack(UtlString*& rpRealm, UtlString*& rpType,
      UtlString*& rpUserId, UtlString*& rpPassToken, int noOfCredentials);
     //:Hack to work around internal compiler error in g++ 2.9 with -O2

};

#endif // !defined(AFX_WEBUILINEMGRUTIL_H__0DE37037_D2A8_464D_916A_30D6FA4302D5__INCLUDED_)
