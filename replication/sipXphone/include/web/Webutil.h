// $Id: //depot/OPENDEV/sipXphone/include/web/Webutil.h#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _Webutil_h_
#define _Webutil_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "pinger/Pinger.h"
#include "net/HttpServer.h"


// DEFINES
// MACROS                     
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

//:Webutil class
// This is the main Webutil task. It is a singleton task and is responsible
// for initializing the Webutil device and starting up any other tasks that
// are needed.
class Webutil
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   Webutil();
     //:Constructor
   virtual
   ~Webutil();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   static int validPassword(UtlString password) ;

   static int validNumber(UtlString password) ;
   
   static void escapeChars(UtlString& strNumber) ;

	static void unescapeChars(UtlString& strNumber);
 
   static int validPhoneNumber(UtlString password) ;

	static UtlBoolean isValidSipUrl(const UtlString& sipUrl);
   
   static void onlyDigits(UtlString& strNumber) ;
	
	static void replaceAll(const UtlString& originalString ,
								 UtlString &modifiedString ,
								 const UtlString& replaceWhat,
								 const UtlString& replaceWith);

/* ============================ ACCESSORS ================================= */



/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

	

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
  
                                    //  is only one instance of this class   
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _Webui_h_


