// $Id: //depot/OPENDEV/sipXphone/src/web/Webutil.cpp#4 $
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
#include "web/Webutil.h"
#include "web/Webui.h"
#include "pinger/Pinger.h"
#include <os/iostream>

#include <assert.h>

#ifdef _WIN32
#   include <io.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 

#include <time.h>

// APPLICATION INCLUDES
#include "cp/CallManager.h"
#include "ps/PsPhoneTask.h"

#include "utl/UtlRegex.h"
#include "pinger/PingerMsg.h"
#include "pinger/SipConfigDeviceAgent.h"
#include "net/SipConfigServerAgent.h"
#include "net/MimeBodyPart.h"

#include "mp/MpMisc.h"
#include "mp/MpMediaTask.h"
#include "mp/MpCallFlowGraph.h"
#include "mp/MprFromNet.h"
#include "mp/MprToNet.h"

#include "os/OsEventMsg.h"
#include "os/OsNameDb.h"
#include "os/OsQueuedEvent.h"
#include "os/OsSysTimer.h"
#include "os/OsTimerTask.h"
#include "os/OsTimeLog.h"
#include "pinger/PingerInfo.h"
#include "ps/PsHookswTask.h"
#include "ps/PsPhoneTask.h"
#include "ps/PsButtonTask.h"
#include "ps/PsButtonId.h"
#include "os/OsConfigDb.h"
#include "os/OsUtil.h"
#include "net/SipUserAgent.h"
#include "net/HttpServer.h"
#include "net/HttpRequestContext.h"
#include "net/NameValueTokenizer.h"
#include "net/SipNotifyStateTask.h"

#include "tao/TaoServerTask.h"

#include "ptapi/PtCall.h"
#include "ptapi/PtProvider.h"
#include "ptapi/PtAddress.h"
#include "ptapi/PtSessionDesc.h"
#include "ptapi/PtTerminal.h"
#include "ptapi/PtPhoneButton.h"
#include "ptapi/PtPhoneDisplay.h"
#include "ptapi/PtPhoneGraphicDisplay.h"
#include "ptapi/PtPhoneHookswitch.h"
#include "ptapi/PtPhoneLamp.h"
#include "ptapi/PtPhoneMicrophone.h"
#include "ptapi/PtPhoneRinger.h"
#include "ptapi/PtPhoneSpeaker.h"
#include "ptapi/PtComponentGroup.h"
#include "ptapi/PtConnection.h"
#include "ptapi/PtTerminalConnection.h"
#include "mp/dtmflib.h"

#include "pingerjni/ApplicationRegistry.h"
#include "pingerjni/SpeedDialWebDSP.h"
#include "pingerjni/VoicemailNotifyStateListener.h"
	
#ifdef _NONJAVA
#include "pingerjni/JNIStubs.h"
#endif

// EXTERNAL VARIABLES

// CONSTANTS




/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Default constructor
Webutil::Webutil()
{

}

// Destructor
Webutil::~Webutil()
{
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */

// 
// Utility class to validate that a user has not entered letters in their password
//
int Webutil::validPassword(UtlString password)
		{

	char buf[100] ;
	sprintf(buf, password.data()) ;

	for (unsigned int i=0 ; i<password.length(); i++) {
		switch (buf[i]) {
		case 'a' : case 'b' : case 'c' : case 'd' : case 'e' : 
		case 'f' : case 'g' : case 'h' : case 'i' : case 'j' : 
		case 'k' : case 'l' : case 'm' : case 'n' : case 'o' : 
		case 'p' : case 'q' : case 'r' : case 's' : case 't' : 
		case 'u' : case 'v' : case 'w' : case 'x' : case 'y' : 
		case 'z' : case 'A' : case 'B' : case 'C' : case 'D' : 
		case 'E' : case 'F' : case 'G' : case 'H' : case 'I' : 
		case 'J' : case 'K' : case 'L' : case 'M' : case 'N' : 
		case 'O' : case 'P' : case 'Q' : case 'R' : case 'S' : 
		case 'T' : case 'U' : case 'V' : case 'W' : case 'X' :
		case 'Y' : case 'Z' : case '0' : case '1' : case '2' : 
		case '3' : case '4' : case '5' : case '6' : case '7' : 
		case '8' : case '9' : case '*' : case '#' : 
			break ;
			// do nothing
		default :
			return -1 ;
		}
	}

	return 0 ;
}


// 
// Utility method to validate that a user has entered a valid phone number 
//
int Webutil::validNumber(UtlString password)
		{

	char buf[100] ;
	sprintf(buf, password.data()) ;

	for (unsigned int i=0 ; i<password.length(); i++) {
		switch (buf[i]) {
		case '0' : case '1' : case '2' : case '3' : case '4' : 
		case '5' : case '6' : case '7' : case '8' : case '9' :
			break ;
			// do nothing
		default :
			return -1 ;
		}
	}

	return 0 ;
}


// 
// Utility method to remove non-digits from phone numbers e.g. spaces
//
void Webutil::onlyDigits(UtlString& strNumber)
{
	char buf[100] ;
	sprintf(buf, strNumber.data()) ;
	int iLen = strNumber.length() ;
	strNumber.remove(0) ;
	
	for (int i=0;i<iLen;i++) {
		switch (buf[i]) {
		case '0' : case '1' : case '2' : case '3' : case '4' : 
		case '5' : case '6' : case '7' : case '8' : case '9' :
			strNumber.append(buf[i]) ;
			break ;
		default : 
			break ;
		}
	}
}

// 
// Utility method to escape characters to allow them to appear in a browser 
//
void Webutil::escapeChars(UtlString& strNumber)
{
	UtlString lessThan("<");
	UtlString greaterThan(">");
	UtlString quote("\"");
   UtlString percentage("%");

	UtlString tempString(strNumber);
	UtlString modifiedString;
	replaceAll( tempString, modifiedString , lessThan , "&lt;");
	
   tempString.remove(0);
	tempString.append(modifiedString );
	modifiedString.remove(0);
	replaceAll( tempString, modifiedString , greaterThan , "&gt;");
 	
   tempString.remove(0);
	tempString.append(modifiedString );
	modifiedString.remove(0);
	replaceAll( tempString, modifiedString , percentage , "&#37;");

  	/*tempString.remove(0);
	tempString.append(modifiedString );
	modifiedString.remove(0);
	replaceAll( tempString, modifiedString , percentage , "%");*/

	tempString.remove(0);
	tempString.append(modifiedString );
	modifiedString.remove(0);
	replaceAll( tempString, modifiedString , quote , "&#034;");
	
   strNumber.remove(0);
	strNumber.append(modifiedString);

}

void Webutil::unescapeChars(UtlString& strNumber)
{

	UtlString lessThan("&lt;");
	UtlString greaterThan("&gt;");
	UtlString quote("&#034;");
   UtlString percentage("&#37;");

	UtlString tempString(strNumber);
	UtlString modifiedString;
	replaceAll( tempString, modifiedString , lessThan , "<");
	
   tempString.remove(0);
	tempString.append(modifiedString );
	modifiedString.remove(0);
	replaceAll( tempString, modifiedString , greaterThan , ">");
 	
   tempString.remove(0);
	tempString.append(modifiedString );
	modifiedString.remove(0);
	replaceAll( tempString, modifiedString , percentage , "%");

	tempString.remove(0);
	tempString.append(modifiedString );
	modifiedString.remove(0);
	replaceAll( tempString, modifiedString , quote , "\"");
	
   strNumber.remove(0);
	strNumber.append(modifiedString);
}

void Webutil::replaceAll(const UtlString& originalString ,
								 UtlString &modifiedString ,
								 const UtlString& replaceWhat,
								 const UtlString& replaceWith)
{
	UtlString tempString(originalString);
	modifiedString.append(originalString);
	int index = UTL_NOT_FOUND;
	while ( (unsigned int) (index = tempString.index(replaceWhat, 0, UtlString::ignoreCase) ) != UTL_NOT_FOUND)
	{
		modifiedString.replace(index, replaceWhat.length(), replaceWith);
		tempString.remove(0);
		tempString.append(modifiedString);
	}
}
//
// Utility method to validate that a user has entered a valid number 
//
int Webutil::validPhoneNumber(UtlString password)
		{

	char buf[100] ;
	sprintf(buf, password.data()) ;

	for (unsigned int i=0 ; i<password.length(); i++) {
		switch (buf[i]) {
		case '0' : case '1' : case '2' : case '3' : case '4' : 
		case '5' : case '6' : case '7' : case '8' : case '9' : case ' ' : case '+' : case '-' : case '*' :
			break ;
			// do nothing
		default :
			return -1 ;
		}
	}

	return 0 ;
}




UtlBoolean Webutil::isValidSipUrl(const UtlString& sipUrl)
{
	UtlString protocol;
	UtlString address;
	UtlString userId;
   UtlBoolean isValidSipUrl = FALSE;
	if( sipUrl.isNull())
	{
		return isValidSipUrl;
	}
	Url url(sipUrl);
	url.getUrlType(protocol);
	if ( protocol.compareTo("sip" , UtlString::ignoreCase) != 0 )
	{
		return isValidSipUrl;
	}
	url.getHostAddress(address);
	url.getUserId(userId);
   if ( (userId.isNull() && address.isNull() ) 
         || address.compareTo("0.0.0.0") == 0)
	{
		return isValidSipUrl;
	}
   //check if port is valid
   RegEx allDigits("^\\d+$");
   int port = url.getHostPort();
	char strPort[10];
	sprintf(strPort, "%d", port);
   UtlString sPort(strPort);
   if(!allDigits.Search(sPort.data()))
   {
      return isValidSipUrl;
   }
   if(!address.isNull())
   {
      RegEx ip4Address("^\\d{3}\\.\\d{3}\\.\\d{3}\\.\\d{3}$");
      if( ip4Address.Search(address.data()) && !OsSocket::isIp4Address(address))
      {
         return isValidSipUrl;
      }
      
   }
   isValidSipUrl = TRUE;
   return isValidSipUrl;

}
