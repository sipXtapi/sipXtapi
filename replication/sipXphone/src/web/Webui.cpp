// $Id$
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
#include "os/OsUtil.h"
#include "web/Webui.h"
#include "web/Webutil.h"
#include "web/WebuiLineMgrUtil.h"
#include "pinger/Pinger.h"
#include "pinger/SipConfigDeviceAgent.h"
#include "os/OsSysLog.h"
#include <os/iostream>

#include <ctype.h>
#include <assert.h>

#ifdef _WIN32
#   include <io.h>
#elif defined(__pingtel_on_posix__)
#   include <malloc.h>
#   include <stdlib.h>
#endif
#include <time.h>

// APPLICATION INCLUDES
#include "cp/CallManager.h"
#include "ps/PsPhoneTask.h"

#include "pinger/PingerInfo.h"
#include "pinger/PingerConfig.h"
#include "net/SipUserAgent.h"
#include "net/SipRefreshMgr.h"
#include "net/HttpServer.h"
#include "net/HttpRequestContext.h"
#include "net/NameValueTokenizer.h"
#include "net/SipNotifyStateTask.h"

//multiline
#include "net/SipLine.h"
#include "net/SipLineList.h"
#include "net/SipLineCredentials.h"
#include "net/SipLineMgr.h"
#include "net/Url.h"

#include "pingerjni/ApplicationRegistry.h"
#include "pingerjni/SpeedDialWebDSP.h"
#include "pingerjni/VoicemailNotifyStateListener.h"
#include "pingerjni/JXAPI.h"
	

// EXTERNAL FUNCTIONS
extern "C" {extern bool hostIsPingable(char* ipAddress);}
// EXTERNAL VARIABLES

// CONSTANTS

#define CONFIGDB_PREFIX_ON_SERVER "pinger-config-"


#define DEPLOYMENT_HTTP_PORT 80
#define MAX_FILELINE_SIZE	160
#define DEFAULT_SIP_PORT	5060

// Set the RAM Disk size for the JAR files to 4MB
#define JAR_RAM_DISK_SIZE_IN_KBYTES 4096
#undef  JAR_RAM_DISK_SIZE_IN_KBYTES

#define OK 0

#define UPGRADE_KERNEL_FILE   "/flash0/kernel.aif.install"	// kernel file uploaded via web form
#define UPGRADE_JAR_FILE		"/flash0/pingtel.jar.install"	// jar file uploaded via web form
#define PINGTEL_JAR_FILE		"/flash0/pingtel.jar"			// target for jar file in upgrade process
#define UPGRADE_LOG_FILE		"/flash0/upgrade-log"			// where we place our log/information

#define UPGRADE_CURRENT_VERSION "CURRENT_VERSION"
#define UPGRADE_PLATFORM		"PLATFORM"

// Speeddial data
#define MAX_SPEEDDIAL_ENTRIES 100
#define MAX_SPEEDDIAL_LENGTH 256
#define SPEED_DIAL_SCHEMA_ID "id"
#define SPEED_DIAL_SCHEMA_LABEL "label"
#define SPEED_DIAL_SCHEMA_PHONE_NUMBER "phone_number"
#define SPEED_DIAL_SCHEMA_URL "url"

//Muiltiline
#define MULTI_LINE_URL "SipUrl"
#define MULTI_LINE_CALL_HANDLING "Callhandling"
#define MULTI_LINE_STATE "LineState"
#define MULTI_LINE_OUTBOUND "isOutBound"
#define MAX_CALLS               5


// New UI HTML

#define HTML_HEADER_1 \
" <html>\n\
<head>\n "

#define HTML_HEADER_2 \
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n\
<META HTTP-EQUIV = \"PRAGMA\" CONTENT = \"no-cache\">\n\
<META HTTP-EQUIV=\"expires\" CONTENT=\"09 Jan 2005 18:42:25 GMT\">\n\
<link rel=stylesheet href=\"/fonts.css\" type=\"text/css\">\n\
<script language=\"JavaScript\">function setDialUrl(newUrl) { document.dial.dial_url.value=newUrl; }</script>\n\
</head>\n "

#define HTML_HEADER_3 \
"\n\
<!-- Page Header table -->\n\
<table width=\"750\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n\
  <tr>\n\
    <td width=\"30%\" rowspan=\"2\">\n\
      <table width=\"100%\" border=\"0\">\n\
        <tr>\n\
          <td><a href=\"http://www.sipfoundry.org\"><img src=\"/sipfoundry-sipxphone-logo.gif\" width=\"220\" height=\"100\" alt=\"SIPFoundry sipXphone\" border=0></a></td>\n\
         </tr>\n\
      </table>\n\
    </td>\n\
    <td width=\"70%\" align=\"right\" valign=\"top\">\n\
      <div id=\"menuarea\"></div>\n\
    </td>\n\
	<td>\n\
	  <div align=\"right\"></div>\n\
	</td>\n\
  </tr>\n\
  <tr>\n\
    <td>\n\
    </td>\n\
  </tr>\n\
</table>\n\
<!--end of page header table-->\n "

#define HTML_FOOTER \
"	<!--end main body of page table-->\n\
<p align=\"center\" class=\"smallText\">Copyright © 2005 SIPFoundry Corporation. Distributed under the LGPL License.</p>\n\
</body>\n\
</html> "

//View Lines
#define HTML_VIEW_LINES_START_TABLE "\
	<!--\"Lines Table\" table-->\n\
  <table width=\"750\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
		<tr>\n\
			<td  width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p><b>SIP URL</b></p></td>\n\
	  		<td  width=\"75\" bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p><b>Allow Forwarding</b></p></td>\n\
	  		<td  width=\"100\" bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p><b>Registration State</b></p></td>\n\
	  		<td  width=\"75\" bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p><b>Call Out As</b></p></td>\n\
			<td  width=\"75\" bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p><b>Line Edit</b></p></td>\n\
		</tr>"

#define HTML_VIEW_LINES_EMPTY_COLUMN "<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\">&nbsp;</td>\n"
    
#define HTML_VIEW_LINES_END_TABLE "\
				</table>\n\
			  <!--End \"Lines Table\" table-->"

#define HTML_VIEW_LINE_INSTRUCTIONS "<!--View Line Instruction Table -->\n\
		<table width=\"750\" border=\"0\" cellspacing=\"10\">\n\
			<tr>\n\
            <td><H1>Line Preferences</H1></td>\n\
			</tr>\n\
			<tr>\n\
				<td width=\"40%\" valign=\"top\">\n\
					<p class=\"textOrange\">Instructions:</p>\
					<p align=\"left\" class=\"normalText\">\n\
						On this page you provide an identity for this sipXphone by defining its \"device line\". You can also set up any number of additional user lines to reflect the different people and entities who use that phone.</p>\n\
						<ul class=\"normalText\">\n\
							<li>To set up a device line, click Add New Line in the Device Line section. To change or delete current device information, click Edit next to the existing line.</li>\n\
							<li>To set up a new user line, click Add New Line in the User Lines section. To change or delete an existing user line, click Edit next to that line.</li>\n\
						</ul>\n\
					<p class=\"normalText\">You choose one of the lines on a phone to be the \"Call Out As\" line. When outbound calls are made from the phone, the SIP URL for the selected line identifies the caller.</p>\n\
				</td>\n\
		</tr>\n\
		</table>\n\n\
		<!--View Line Instruction Table--> "

#define HTML_VIEW_LINES_REFERENCE "\
	<p class=\"normalText\">When you choose the \"Call Out As\" line, you set the value for the  USER_DEFAULT_OUTBOUND_LINE configuration parameter. </p>"

//Add Edit Lines
#define HTML_ADD_LINE_EDITLINES "\
<!--table for body of page-->\n\
<table width=\"750\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
	<tr>\n\
     	<td colspan=\"2\"><p class=\"textOrange\">Enter Line Information:</p></td>\n\
	</tr>\n\
	  <form method=\"post\" name=\"line\" action=\"/cgi/addLines.cgi\">\n<input type=\"hidden\" name=\"issubmit\" value=\"yes\" size=\"30\">\n\
      <tr>\n\
			<td width=\"30%\" bgcolor=\"#FFCC33\" valign=\"top\" align=\"right\">Sip URL:</td>\n\
			<td bgcolor=\"#FFCC33\" valign=\"top\">"

#define HTML_ADD_LINE_AFTERURL "&nbsp;</td>\n\
            </tr>\n\
			   <tr> \n\
               <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"right\">Allow Forwarding:</td>\n\
               <td bgcolor=\"#FFCC33\"> \n\
				     <input type=\"radio\" name=\"isCallHandling\" value=\"Enabled\" "

#define HTML_ADD_LINE_AFTERCALLHANDLING1 ">\n\
                 &nbsp; Enabled &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
					  <input type=\"radio\" name=\"isCallHandling\" value=\"Disabled\" "

#define HTML_ADD_LINE_AFTERCALLHANDLING ">&nbsp; Disabled &nbsp;</td>\n\
            </tr>\n\
            <tr> \n\
               <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"right\">Registration:</td>\n\
               <td bgcolor=\"#FFCC33\"> \n\
				     <input type=\"radio\" name=\"isRegisterEnabled\" value=\"Register\" "

#define HTML_ADD_LINE_AFTERREGISTRATION ">\n\
                 &nbsp; Register &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
					  <input type=\"radio\" name=\"isRegisterEnabled\" value=\"Provision\" "

#define HTML_ADD_LINE_AFTERLINEREGISTRATION ">&nbsp; Provision &nbsp;</td>\n\
            </tr>\n\
            <tr>\n\
			      <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"right\">\n\
					Authentication Credentials:"

#define HTML_ADD_LINE_CREATECREDENTIALTABLE "</td>\n\
					<td bgcolor=\"#FFCC33\" valign=\"top\">\n\
						<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
							<tr> \n\
								<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"left\"><b><u>Realm</u></b></td>\n\
								<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"left\"><b><u>UserId</u></b></td>\n\
								<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"left\"><b><u>Edit<u></b></td>\n\
							</tr>"


#define HTML_ADD_LINE2 "</table><!--End Add Lines table-->\n"


#define HTML_ADD_LINES_INSTRUCTIONS "\n\
<!--Add Line Instruction Table -->\n\
	<table width=\"750\" border=\"0\" cellspacing=\"0\">\n\
		<tr>\n\
			<td><H1> Modify Line</H1></td>\n\
		</tr>\n\
		<tr>\n\
			<td width=\"40%\" valign=\"top\">\n\
				<p class=\"textOrange\">Instructions:</p>\n\
				<p align=\"left\" class=\"normalText\">To set up a line, you define:</p>\n\
					<ol class=\"normalText\">\n\
						<li>The SIP URL that identifies this line. For the device line, defaults to \n\
								sip:4444@<phone IP address>. Minimally, you replace \"4444\" with an identifier \n\
								such as the assigned extension or the name of a particular user. \n\
								If your network includes a SIP Registry Server, you supply its fully qualified \n\
								domain name to replace the phone's IP address.<br>\n\
								See <u>http://www.sipfoundry.org</u> for additional information.</li>\n\
						<li>Whether or not call forwarding preferences \n\
								for this phone should apply to this line. If you choose disable, callers are not \n\
								forwarded to the defined destinations when the phone is busy, not answered, etc.</li>\n\
						<li>The registration method for this line to use. If Register, be sure to include \n\
								the SIP Registry Server in the SIP URL. If Provision, the phone does not attempt to register.</li>\n\
						<li>Optionally, click Add Credentials to specify authentication credentials for \n\
								outbound calls to a destination that requires a user name and password.</li>\n\
						<li>Click Update to add the new line or save your edits.\n\
					</ol>\n\
				<p>To delete a line completely, click Delete.</p>\n\
			</td>\n\
	</tr>\n\
</table>\n\
<!--End Add Line Instruction Table -->"


#define HTML_ADD_LINE3_PARAM_TABLE "\n\
<p class=\"normalText\">The line information that you enter is stored as values for the following configuration parameters: </p>\
		<!--Configuration parameter table -->\n\
		<table width=\"70%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
			<tr>\n\
  				<td class=\"normalText\" colspan=\"2\"><b>Field</b></td>\n\
				<td class=\"normalText\" colspan=\"2\"><b>Device line</b></td>\n\
				<td class=\"normalText\" colspan=\"2\"><b>User line</b></td>\n\
			</tr>\n\
			<tr>\n\
  				<td class=\"normalText\" colspan=\"2\">SIP URL</td>\n\
				<td class=\"normalText\" colspan=\"2\">PHONESET_LINE.URL</td>\n\
				<td class=\"normalText\" colspan=\"2\">USER_LINE.&lt;n&gt;.URL</td>\n\
			</tr>\n\
			<tr>\n\
  				<td class=\"normalText\" colspan=\"2\">Allow forwarding?</td>\n\
				<td class=\"normalText\" colspan=\"2\">PHONESET_LINE.ALLOW_FORWARDING</td>\n\
				<td class=\"normalText\" colspan=\"2\">USER_LINE.&lt;n&gt;.ALLOW_FORWARDING</td>\n\
			</tr>\n\
			<tr>\n\
  				<td colspan=\"2\"><p>Registration</p></td>\n\
				<td colspan=\"2\"><p>PHONESET_LINE.REGISTRATION</p></td>\n\
				<td colspan=\"2\"><p>USER_LINE.&lt;n&gt;.REGISTRATION</p></td>\n\
			</tr>\n\
		</table>\n\
		<!-- End Configuration parameter table -->\n\
<p>When you save values for a user line, the phone automatically assigns it an identifying number and includes that number in all associated USER_LINE parameters.</p>"

#define HTML_ADDLINES_UPDATEBUTTON "<input type=\"submit\" name=\"Submit\" value=\"Update\"> "

#define HTML_ADDLINES_DELETEBUTTON "<input type=\"submit\" name=\"Submit\" value=\"Delete\"> "

#define HTML_ADDLINES_AFTERBUTTON "</form> &nbsp;</td>\n\
               <td bgcolor=\"#FFCC33\">&nbsp;</td>\n\
            </tr>\n\
               </table>"

#define HTML_ADDLINES_AFTERSTATUS "&nbsp;</p></td>\n\
			<td bgcolor=\"#FFCC33\" valign=\"top\"><p>\n "

#define HTML_ADDLINES_AFTERDELETE "</td></tr>\n\n\
	           <tr>\n\
			    <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"right\"><p>\n "

#define HTML_ADDLINES_AFTERNUMBER ">&nbsp;</td> \n\
		        </tr> \n\n\
			  <tr>					\n\
                   <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"right\">&nbsp;</td>\n\
				<td bgcolor=\"#FFCC33\" valign=\"top\">\n\
					<input type=\"submit\" name=\"speeddial\" value=\"Enter\"></form>\n\
					\n "
					


//Credentials
#define HTML_ADD_CREDENTIALS1 "\
<!--Add Credentials Table-->\n\
<table width=\"750\" border=\"0\" valign=\"top\" cellpadding=\"2\" cellspacing=\"2\">\n\
	<tr>\n\
	  	<td colspan=\"2\"><p class=\"textOrange\">Enter Line Credentials Information:</p></td>\n\
  </tr>\n\n\
  <form method=\"post\" name=\"line\" action=\"/cgi/addCredentials.cgi\">\n\
  <input type=\"hidden\" name=\"issubmit\" value=\"yes\" size=\"30\">"

#define HTML_ADD_CREDENTIALS_StartTable "<tr>\n\
                 <td width=\"200\" bgcolor=\"#FFCC33\" valign=\"top\" align=\"right\">Sip URL:</td>\n\
				     <td bgcolor=\"#FFCC33\" valign=\"top\">"
            
#define HTML_ADD_CREDENTIALS_afterUrlDisplay "</td></tr>\n\
			   <tr> \n\
               <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"right\">Realm:</td>\n\
					<td bgcolor=\"#FFCC33\"> "

#define HTML_ADD_CREDENTIALS_inputRealm "<input type=\"text\" name =\"Realm\" size=\"70\" >"

#define HTML_ADD_CREDENTIALS_afterRealm "</td>\n\
            </tr>\n\
			   <tr> \n\
               <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"right\">User ID:</td>\n\
               <td bgcolor=\"#FFCC33\"> "

#define HTML_ADD_CREDENTIALS_inputUserId "<input type=\"text\" name =\"UserId\" size=\"70\" > "
	
#define HTML_ADD_CREDENTIALS_afterUserId "</td>\n\
            </tr>\n\
			   <tr> \n\
               <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"right\">Password:</td>\n\
               <td bgcolor=\"#FFCC33\"> \n\
				        <input type=\"password\" name =\"Password\" size=\"70\" > "
   
#define HTML_ADD_CREDENTIALS_afterPassword "&nbsp</td>\n\
            </tr>\n\
			   <tr> \n\
               <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"right\">Confirm Password:</td>\n\
               <td bgcolor=\"#FFCC33\"> \n\
				        <input type=\"password\" name =\"ConfirmPassword\" size=\"70\" > "
   
#define HTML_ADD_CREDENTIALS_afterConfirmPassword "</td>\n\
		      </tr> \n\
			   <tr>\n\
               <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"centre\"> "

#define HTML_ADD_CREDENTIALS2 "<!--End Add Credentials table-->\n\
</table>\n"

#define HTML_ADD_CREDENTIALS_INSTRUCTIONS "<!-- Add Credentials Instruction Table-->\n\
	<table width=\"750\" border=\"0\" cellspacing=\"0\">\n\
		<tr>\n\
			<td><H1> Add/Edit Credentials</H1></td>\n\
		</tr>\n\
		<tr>\n\
	     <td width=\"30%\" valign=\"top\">\n\
	 			<p class=\"textOrange\">Instructions:</p>\n\
				<p>Your sipXphone can be set up to require a user name and password, thereby limiting the calls you receive to a predefined set of known callers. For you to make a call to such a phone, you define your credentials for calling:</p>\n\
					<ol class=\"normalText\">\n\
						<li>Enter the Realm of the destination phone that requires authentication.</li>\n\
						<li>Enter the assigned user name.</li>\n\
						<li>Enter the assigned password.</li>\n\
						<li>Re-enter the password.</li>\n\
						<li>Click Save to add the new credential set or to save your edits.</li>\n\
					</ol>\n\
				<p>To delete a set of credentials completely, click Delete. </p>\n\
		</td>\n\
	</tr>\n\
</table>\n\
<!--End Add Credentials Instruction Table-->"

#define HTML_ADD_CREDENTIALS_PARAM_TABLE "\
<p class=\"normalText\">The credentials you enter are stored as values for the following configuration parameters: </p>\
<!--Configuration parameter table -->\n\
	<table width=\"60%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
		<tr>\n\
			<td colspan=\"2\"><b>Field</b></td>\n\
			<td colspan=\"2\"><b>Device line</b></td>\n\
			<td colspan=\"2\"><b>User line</b></td>\n\
		</tr>\n\
		<tr>\n\
			<td colspan=\"2\">Realm</td>\n\
			<td colspan=\"2\">PHONESET_LINE.CREDENTIAL.&lt;n&gt;.REALM</td>\n\
			<td colspan=\"2\">USER_LINE.&lt;n&gt;.CREDENTIAL.&lt;n&gt;.REALM</td>\n\
		</tr>\n\
		<tr>\n\
			<td colspan=\"2\">User ID</td>\n\
			<td colspan=\"2\">PHONESET_LINE.CREDENTIAL.&lt;n&gt;.USERID</td>\n\
			<td colspan=\"2\">USER_LINE.&lt;n&gt;.CREDENTIAL.&lt;n&gt;.USERID</td>\n\
		</tr>\n\
		<tr>\n\
			<td colspan=\"2\">Password</td>\n\
			<td colspan=\"2\">PHONESET_LINE.CREDENTIAL.&lt;n&gt;.PASSTOKEN</td>\n\
			<td colspan=\"2\">USER_LINE.&lt;n&gt;.CREDENTIAL.&lt;n&gt;.PASSTOKEN</td>\n\
		</tr>\n\
	</table>\n\
				<!-- End Configuration parameter table -->\n\
<p>When you save a set of credentials, the phone automatically assigns it an identifying number and includes \
that number in all associated PHONESET_LINE.CREDENTIAL or USER_LINE.&lt;n&gt;.CREDENTIAL parameters.</p>"

// More HTTP SERVER related definition
#define HTTP_BODY_BEGIN "<HTML>\n<BODY>\n"
#define HTTP_BODY_END   "</BODY>\n</HTML>\n"
#define HTTP_TABLE_BEGIN "<TABLE BORDER> \n"
#define HTTP_TABLE_END   "</TABLE>\n"
#define HTTP_CONFIG_TABLE_ROW(COL1, COL2) "<TR><TD>" #COL1 "</TD> <TD>" #COL2 "</TD></TR>"

#define SYSLOG_ACTION_REFRESH    "Refresh Log"
#define SYSLOG_ACTION_CLEAR      "Clear Log"
#define SYSLOG_ACTION_PRIORITY   "Set"


// STATIC VARIABLE INITIALIZATIONS
Webui* Webui::spInstance = 0;
OsBSem  Webui::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

 	/*
	 * Device profile (pinger-config) parameters
	 */
const char* Webui::deviceConfig[] = {
		BASE_PHONESET_LINE_KEY,  
		CONFIG_PHONESET_ADMIN_DOMAIN,
		CONFIG_PHONESET_DEPLOYMENT_SERVER,
		CONFIG_PHONESET_DIALPLAN_LENGTH,
		CONFIG_PHONESET_EXTENSION,               // deprecated (as of v2.0)
		CONFIG_PHONESET_EXTERNAL_IP_ADDRESS,
        SIP_SYMMETRIC_SIGNALING,
        SIP_STUN_SERVER,
        SIP_STUN_REFRESH_PERIOD,
		CONFIG_PHONESET_LOGICAL_ID,
		CONFIG_PHONESET_OUTGOING_CALL_PROTOCOL,  // deprecated (as of 1.0?)
		CONFIG_PHONESET_RTP_PORT_START,
		PHONESET_VERSIONCHECK_PERIOD,
		PHONESET_VERSIONCHECK_URL,
		SIP_AUTHORIZE_PASSWORD,                  // deprecated (as of v2.0)
		SIP_AUTHORIZE_USER,                      // deprecated (as of v2.0)
		SIP_CONFIG_SERVER_ADDRESS,               // deprecated (as of v2.0)
		SIP_CONFIG_SERVER_PORT,                  // deprecated (as of v2.0)
		SIP_DIRECTORY_SERVERS,
		SIP_PROXY_SERVERS,
		SIP_REGISTER_PERIOD,
		SIP_REGISTRY_SERVERS,                    // deprecated (as of v2.0)
		SIP_SESSION_REINVITE_TIMER,
		SIP_TCP_PORT,
		SIP_UDP_PORT,
    0
};

 	/*
	 * Array identifying all of the configuration parameters
   * (device and user) that are managed by the web UI.
	 */
const char* Webui::allConfig[] = { 
		BASE_PHONESET_LINE_KEY,  
		BASE_USER_LINE_KEY,
		CONFIG_PHONESET_AVAILABLE_BEHAVIOR,
		CONFIG_PHONESET_BUSY_BEHAVIOR,
		CONFIG_PHONESET_CALL_WAITING_BEHAVIOR,
		CONFIG_PHONESET_DND,
		CONFIG_PHONESET_DND_METHOD,
		CONFIG_PHONESET_DIALPLAN_LENGTH,
		CONFIG_PHONESET_DIGITMAP,    
		CONFIG_PHONESET_EXTERNAL_IP_ADDRESS,
        SIP_SYMMETRIC_SIGNALING,
        SIP_STUN_SERVER,
        SIP_STUN_REFRESH_PERIOD,
		CONFIG_PHONESET_FORWARD_UNCONDITIONAL,
		CONFIG_PHONESET_HTTP_AUTH_DB,
		CONFIG_PHONESET_HTTP_PORT,
		CONFIG_PHONESET_HTTP_PROXY_HOST,
		CONFIG_PHONESET_HTTP_PROXY_PORT,
		CONFIG_PHONESET_LOGO_URL,
		CONFIG_PHONESET_NO_ANSWER_TIMEOUT,
		CONFIG_PHONESET_RINGER,
		CONFIG_PHONESET_RTP_PORT_START,
		CONFIG_PHONESET_TELNET_ACCESS,
		CONFIG_PHONESET_TIME_DST_RULE,
		CONFIG_PHONESET_TIME_OFFSET,
		CONFIG_PHONESET_TIME_SERVER,
		PHONESET_MSG_WAITING_SUBSCRIBE,
		PHONESET_SNMP_TRAP_DESTS,
		PHONESET_VOICEMAIL_RETRIEVE,
		SIP_DIRECTORY_SERVERS,
		SIP_FORWARD_ON_BUSY,
		SIP_FORWARD_ON_NO_ANSWER,
		SIP_FORWARD_UNCONDITIONAL,
		SIP_PROXY_SERVERS,
		SIP_REGISTER_PERIOD,
		SIP_SESSION_REINVITE_TIMER,
		SIP_TCP_PORT,
		SIP_UDP_PORT,
		USER_DEFAULT_OUTBOUND_LINE,
		0
	} ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Return a pointer to the Webui task, creating it if necessary
Webui* Webui::getWebuiTask(void)
{

   // If the task object already exists, and the corresponding low-level task
   // has been started, then use it
   if (spInstance != NULL)
      return spInstance;

   // If the task does not yet exist or hasn't been started, then acquire
   // the lock to ensure that only one instance of the task is started
   sLock.acquire();
   if (spInstance == NULL)
       spInstance = new Webui();
   sLock.release();

   return spInstance;
}

// Destructor
Webui::~Webui()
{
	delete spInstance ;
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Default constructor (called only indirectly via getPingerTask())
Webui::Webui()
{



	/* Total set of pinger-config parameters. Any other parameters are in user-config.
     * Note this list does not include "multiple" parameters such as SIP_AUTHORIZE_USER
	 */






}
/* //////////////////////////// PRIVATE /////////////////////////////////// */

void Webui::initWebui(HttpServer* pHttpServer)
{

	osPrintf("Webui::initWebui reached\n") ;



	if(pHttpServer)
	{
		osPrintf("Webui::access HttpServer pointer\n") ;

	   // New user interface

	   // Multiple ways of calling home page
	   pHttpServer->addRequestProcessor("/cgi/homepage.cgi", processHomePage) ;       
	   pHttpServer->addRequestProcessor("/", processHomePage);
       pHttpServer->addRequestProcessor("/index.html", processHomePage);

	   pHttpServer->addRequestProcessor("/cgi/applications.cgi",processApplications) ;
	   pHttpServer->addRequestProcessor("/cgi/xpressaconfiguser.cgi", getPingerConfigUser) ;
	   pHttpServer->addRequestProcessor("/cgi/xpressaconfigadmin.cgi", getPingerConfigAdmin) ;
 	   pHttpServer->addRequestProcessor("/cgi/setpingerconfiguser.cgi", setPingerConfigUser) ;
	   pHttpServer->addRequestProcessor("/cgi/setpingerconfigadmin.cgi", setPingerConfigAdmin) ;
       pHttpServer->addRequestProcessor("/cgi/login.cgi", processLogin);	//User maintenance screen
       pHttpServer->addRequestProcessor("/cgi/password.cgi", processChangePassword); //Change password screen
 	   pHttpServer->addRequestProcessor("/cgi/setPassword.cgi", processSetPassword); //User maintenance - change password
	   pHttpServer->addRequestProcessor("/cgi/deleteUser.cgi", processDeleteUser) ; //Delete a user
       pHttpServer->addRequestProcessor("/cgi/config.cgi", processConfig);
       pHttpServer->addRequestProcessor("/cgi/configEnroll.cgi", processConfigEnroll);
       pHttpServer->addRequestProcessor("/cgi/restart.cgi", processRestart);
       pHttpServer->addRequestProcessor("/cgi/appManager.cgi", processAppManager);

	   //Additional "helper" files that the new UI uses
	   pHttpServer->addRequestProcessor("/fonts.css", processFonts) ;     
	   pHttpServer->addRequestProcessor("/dhtmlMenu.js", processDhtmlMenu) ; 	
	   pHttpServer->addRequestProcessor("/sipfoundry-sipxphone-logo.gif", processLogo) ; 	
	   pHttpServer->addRequestProcessor("/1x1trans.gif", processBackgroundImage) ; 	  

	   // Speeddial interface
	   pHttpServer->addRequestProcessor("/cgi/addspeeddial.cgi", processAddSpeeddial) ;
 	   pHttpServer->addRequestProcessor("/cgi/viewspeeddial.cgi", processViewSpeeddial) ;
 	   pHttpServer->addRequestProcessor("/cgi/deletespeeddial.cgi", processDeleteSpeeddial) ;

/*
      // Visitor (Presto) login
      pHttpServer->addRequestProcessor("/cgi/phoneLogin.cgi", processPhoneLoginCgi);
*/

      //multiline interface
      pHttpServer->addRequestProcessor("/cgi/MultiLine.cgi", processViewLinesCgi);
      pHttpServer->addRequestProcessor("/cgi/addLines.cgi", processAddLinesCgi);
      pHttpServer->addRequestProcessor("/cgi/addCredentials.cgi", processAddCredentialsCgi);

      // System Log 
      pHttpServer->addRequestProcessor("/cgi/syslog.cgi", processViewSyslog) ;
      pHttpServer->addRequestProcessor("/cgi/config_syslog.cgi", processConfigSyslog) ;

		osPrintf("Webui::add requests to web server\n") ;
	} else {
		osPrintf("Webui::couldn't add requests\n") ;
	}

}

#define MAX_ID_ARRAY_SIZE 20
void Webui::processHomePage(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{


	// Method variables
	UtlString feedbackMessage;
    requestContext.getCgiVariable("MESSAGE", feedbackMessage);
	UtlString callIDFromURl;
    requestContext.getCgiVariable("CALL_ID", callIDFromURl);
	if(callIDFromURl.isNull())
	    callIDFromURl="sip:";
    
    

	int numCalls = 0;
    int numAddresses = 0;
    int addressIndex;
	UtlString callIdArray[MAX_ID_ARRAY_SIZE];
    UtlString addressArray[MAX_ID_ARRAY_SIZE];
    Pinger* pinger = Pinger::getPingerTask();
	CallManager* callManager = pinger->getCallManager() ;
	if (pinger && callManager) {
		callManager->getCalls(MAX_ID_ARRAY_SIZE, 
            numCalls, callIdArray);
			osPrintf("Webui::processHomePage pinger and call manager enabled\n") ;
	}

	osPrintf("Webui::processHomePage number of calls = %d\n", numCalls) ;

/*    if(pinger && pinger->mpCallMgrTask)
        pinger->mpCallMgrTask->getCalls(MAX_ID_ARRAY_SIZE, 
            numCalls, callIdArray);
*/
	UtlString homePageHTML(HTML_HEADER_1) ;
	homePageHTML.append("<title>sipXphone Home</title>") ;

    UtlBoolean  bIsSecure = FALSE;
    HttpMessage tmprequest = (HttpMessage)request;
    if (tmprequest.getSendProtocol() == OsSocket::SSL_SOCKET)
            bIsSecure = TRUE;

	insertHeader(homePageHTML,bIsSecure) ;

	homePageHTML.append("\
<!--table for body of page-->\n\
<table width=\"750\" border=\"0\" valign=\"top\" cellspacing=\"10\">\n\
	<tr>\n\
		<td><H1>Home</H1></td>\n\
	</tr>\n\
	<tr>\n\
    <td valign=\"top\" width=\"60%\">\n\
			<!--\"Introduction Table\" table-->\n\
			<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
			  <tr>\n\
			  	<td colspan=\"2\"><p class=\"textOrange\">Introduction:</p></td>\n\
			  </tr>\n\
              <tr>\n\
                <td width=\"600\" height=\"100\" colspan=\"2\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
					<h3 align=\"left\">Welcome to <i>sipXphone&#153;!</i></h3>\n\
					<p align=\"left\">From these web pages you can configure and maintain your <i>sipXphone</i> or perform tasks that are easier to do using a computer keyboard than with <i>sipXphone</i>'s dial pad.</p>\n\
                </td>\n\
               </tr>\n\
			 \n\
              </table>\n\
			  <!--End \"Introduction\" table-->\n ") ;

	homePageHTML.append("\
			<!--\"Dial by URL\" table-->\n\
			<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
			  <tr>\n\
			  	<td colspan=\"2\"><p class=\"textOrange\"><a name=\"dialbyurl\">Dial by URL:</a></p></td>\n\
			  </tr>\n\
              <tr>\n\
                <td width=\"30%\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>Enter the URL to dial:</p>\n\
                </td>\n\
			    <td bgcolor=\"#FFCC33\" valign=\"top\">\n\
			  		<!--dial by URL-->\n\
					<form method=\"post\" action=\"/cgi/dial.cgi\">\n\
						<input type=\"text\" name=\"dial_url\" size=\"30\" value=\"");
	homePageHTML.append(callIDFromURl);
	homePageHTML.append("\">&nbsp;\n\
						<input type=\"submit\" name=\"DialButton\" value=\"Dial\"></form>\n\
                </td>\n\
			  </tr>\n ") ;
			 
    // Have different paths for VXW and NT
	PingerInfo pingerInfo ;
	UtlString path("") ;
	pingerInfo.getFlashFileSystemLocation(path) ;

	UtlString homePageBottomHTML("\
              </table>\n\
			  <!--End \"Dial by URL\" table--><br><br>\n\
			<!-- Audio table-->\n\
			<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
				<tr>\n\
				  <td colspan=\"2\"><p class=\"textOrange\"><a name=\"ringtone\">Set your Ring Tone:</a></p></td>\n\
				</tr>\n\
                <tr>\n\
                  <td width=\"30%\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
				  <p>Enter ring tone file:</p></td>\n\
                  <td width =\"70%\" bgcolor=\"#FFCC33\">\n\
                  <p>The audio file must be:\n\
				  <ul type=\"compact\" class=\"normalText\">\n\
				  <li>.WAV (RIFF-WAV) format.</li>\n\
				  <li>16-bit signed PCM and in little-endian byte order.</li>\n\
				  <li>All formats must be mono.</li>\n\
				  <li>All formats must use a sampling rate of 8000 samples/second.</li>\n\
				  <li>The maximum file size is 500KB.</li></ul></p>\n\
					<p>Enter the path and file name of an audio file to use as the <b>ring tone</b> for your incoming calls:</p> \n\
				  <form method=\"post\" action=\"/cgi/putFile.cgi\" ENCTYPE=\"multipart/form-data\">\n\
 					<input type=\"file\" name=\"");   
      homePageBottomHTML.append(path);

      homePageBottomHTML.append("/ringTone.wav\" size=\"30\">\n\
					<input type=\"submit\" name=\"PUT_CONFIG\" value=\"Upload\">\n\
				  </form>\n\
                  </td>\n\
                </tr>\n\
              </table>\n\
			  <!-- End audio table-->\n ") ;
	
	homePageBottomHTML.append("\
			<td width=\"40%\" valign=\"top\">\n\
            <p class=\"textOrange\">Instructions:</p>\n\
			  <p align=\"left\">\n\
			    <b>All Users:</b><br>\n\
				To perform different tasks for sipXphone, you click on options at the top of this page.<br>\n\
				  <ul type=\"compact\" class=\"normalText\">\n\
							<li>To work with sipXphone applications, click on <a href = \"/cgi/applications.cgi\">Applications</a>: Manage Apps.</li>\n");
						
	int iSpeedDialReadOnly = JNI_isSpeedDialReadOnly();
	//if speedial is readonly
	//don't show the instruction to add or view speeddial.
	if( ! iSpeedDialReadOnly == 1 ){
		homePageBottomHTML.append("\
			         <li>To set up Speed Dial numbers, click on <a href=\"/cgi/viewspeeddial.cgi\">Speed Dial</a> and choose Add or View.</li> \n");
	}
							
	homePageBottomHTML.append("\
					 <li>To set Call Handling and Time and Locale preferences for your sipXphone, click on Preferences: <a href=\"/cgi/xpressaconfiguser.cgi\">Preferences</a>.</li>\n\
                     <li>To add and edit lines for your sipXphone, click on Preferences: <a href=\"/cgi/MultiLine.cgi\">Lines</a>.</li></ul>\n\
				<p>\n\
				<br><br><b>System Administrators:</b><br>\n\
				The Administration option allows system administrators to access these additional options:\n\
				  <ul type=\"compact\" class=\"normalText\">\n\
				<li>To protect sipXphone from unauthorized access, click <a href=\"/cgi/password.cgi\">Change Password</a> or <a href=\"/cgi/login.cgi\">User Maintenance</a>.</li> \n\
				<li>To enter configuration values for sipXphone, click <a href=\"/cgi/xpressaconfigadmin.cgi\">Phone Configuration</a>.</li> \n\
				<li>To install new files, click <a href=\"/cgi/config.cgi\">File Uploads</a>. You can install configuration files, audio files, and release files.</li> \n\
				<li>To access diagnostic tools for sipXphone, click on <a href=\"/cgi/sip.cgi\">SIP Log</a>.</li></ul>\n\
				<p>For more information, read the documentation provided at <a href=\"http://www.sipfoundry.org\">www.sipfoundry.org</a>\n\
     </td>\n\
		</tr>\n\
</table>\n " ) ;

	homePageBottomHTML.append(HTML_FOOTER) ;

	// Build Page

	UtlString html(homePageHTML) ;

	// Add in current call info

	for(int callIndex = 0; callIndex < numCalls; callIndex++)
    {
        numAddresses = 0;
		Pinger* pinger = Pinger::getPingerTask();
		CallManager* callManager = pinger->getCallManager() ;
		if (pinger && callManager) {
			callManager->getConnections((callIdArray[callIndex]).data(),
                MAX_ID_ARRAY_SIZE, numAddresses, addressArray);
		}
/*        if(pinger && pinger->mpCallMgrTask)
            pinger->mpCallMgrTask->getConnections((callIdArray[callIndex]).data(),
                MAX_ID_ARRAY_SIZE, numAddresses, addressArray);
*/
        html.append("<tr>\n<td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\"><p>Call ");
		html.append("</p></td><td bgcolor=\"#FFCC33\" valign=\"top\">") ;

        for(addressIndex = 1; addressIndex < numAddresses; addressIndex++)
        {
            if(addressIndex > 1) html.append("        <P>");
            html.append(addressArray[addressIndex]);
        }

        html.append("</p><p><FORM METHOD=POST ACTION=/cgi/drop.cgi><INPUT TYPE=SUBMIT NAME=DROP VALUE=\"Hang up\">\n<INPUT TYPE=HIDDEN NAME=CALL_ID VALUE=\"");
        html.append(callIdArray[callIndex]);
        html.append("\">\n      </FORM></p>\n    </TD>\n  </TR>\n");
    }

    if(feedbackMessage.isNull())
    {
        html.append("&nbsp;");
    }
    else
    {
        html.append("<tr>\n<td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\"><p>Status</p></td><td bgcolor=\"#FFCC33\" valign=\"top\"><p>") ;
		html.append(feedbackMessage) ;
		html.append("</p></TD>\n  </TR>\n") ;
    }
    
	html.append(homePageBottomHTML);


	// Build Response
    response = new HttpMessage();
    HttpBody* body = new HttpBody(html.data(),
        html.length(), CONTENT_TYPE_TEXT_HTML);
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_OK_CODE, HTTP_OK_TEXT);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);
}

void Webui::processApplications(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
   
    UtlString installURL;
    UtlString uninstallURL;

    requestContext.getCgiVariable("install", installURL);
    requestContext.getCgiVariable("uninstall", uninstallURL);

	int retCodeInstall = 0 ; // use this later to display status message
	int retCodeUninstall = 0 ;

	// First install/uninstall the application
    if (installURL.length() != 0)
    {
         retCodeInstall = CGI_INSTALL_APPLICATION_OK;
 
         osPrintf("Install application %s\n", installURL.data());
         retCodeInstall = JNI_installJavaApplication(installURL.data()) ;
         osPrintf("    %s   return code %d\n", (retCodeInstall == CGI_INSTALL_APPLICATION_OK) ? "Succeeded" : "Failed", retCodeInstall);
    } else if (uninstallURL.length() != 0)
    {
         retCodeUninstall =  CGI_UNINSTALL_APPLICATION_OK;

         osPrintf("Uninstall application %s\n", uninstallURL.data());
         retCodeUninstall = JNI_uninstallJavaApplication(uninstallURL.data()) ;
         osPrintf("    %s   return code %d\n", (retCodeUninstall == CGI_UNINSTALL_APPLICATION_OK) ? "Succeeded" : "Failed", retCodeUninstall);
	}

    UtlBoolean  bIsSecure = FALSE;
    HttpMessage tmprequest = (HttpMessage)request;
    if (tmprequest.getSendProtocol() == OsSocket::SSL_SOCKET)
            bIsSecure = TRUE;

	UtlString appHTML(HTML_HEADER_1) ;
	appHTML.append("<title>sipXphone Applications</title>") ;
	insertHeader(appHTML,bIsSecure) ;

	appHTML.append("\
<!--table for body of page-->\n\
<table width=\"750\" border=\"0\" valign=\"top\" cellspacing=\"10\">\n\
	<tr>\n\
		<td colspan=\"2\"><H1>Applications</H1></td>\n\
	</tr>\n\
	<tr>\n\
    <td valign=\"top\" width=\"60%\">\n\
			<!--\"Applications\" table-->\n\
			<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
			  <tr>\n\
			  	<td colspan=\"2\"><p class=\"textOrange\">Manage sipXphone applications</p></td>\n\
			  </tr>\n\
              <tr>\n\
                <td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>Install application:</p>\n\
                </td>\n\
			    <td bgcolor=\"#FFCC33\" valign=\"top\">\n\
			  		<!--install app-->\n\
					<form name=\"xpressa application\" action=\"/cgi/applications.cgi\">\n\
						<input type=\"text\" name=\"install\" size=\"60\"><br>\n\
						<input type=\"submit\" value=\"Install\"></form>\n\
                </td>\n\
			  </tr>\n ") ;
	appHTML.append("\
			  <tr>\n\
                <td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>List of currently installed applications:</p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\" valign=\"top\">\n ") ;

	UtlString afterApps ("\
			  <input type=\"submit\" value=\"Uninstall\">\n\
				  </form>\n\
                </td></tr>\n" ) ;

	UtlString loadedApps("\
 <table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
 <tr>\n\
 	<td colspan=\"2\"><p class=\"textOrange\">Currently loaded applications:</p></td>\n\
 </tr>\n ") ;

	UtlString afterStatusString ("\
			  </td><td>\n ") ;



	afterStatusString.append("\
	<table><tr><td valign=\"top\">\n\
            <p class=\"textOrange\">Instructions:</p>\n\
			<p align=\"left\">On this page you can manage your sipXphone applications. </p>\n\
			<p align=\"left\">To install a new sipXphone application on your sipXphone, enter its location then click <b>Install</b>.</p> \n\
				<ul class=\"normalText\">\n\
				<li>Every sipXphone application is packaged in a file with the .JAR extension.</li>\n\
				<li>Enter the location of the sipXphone application in the form of a URL. (For example, enter <font face=\"Courier\">file:///c:/sipXphone-apps/myApplet.jar</font>)</li>\n\
				</ul> \n\
				<p align=\"left\">To review what's already installed on your sipXphone, click on the drop-down list of current sipXphone applications.</p>\n\
				<p align=\"left\">To remove a sipXphone application from your sipXphone, select it from the drop-down list of sipXphone applications then click <b>Uninstall</b>.</p>\n\
				<p align=\"left\">If you need more help, check the SIPFoundry website at <a href=\"http://www.sipfoundry.org\">www.sipfoundry.org</a>.</p>\n\
      </td><tr></table></td>\n\
	  		</tr>\n\
	</table>\n ") ;


	afterStatusString.append(HTML_FOOTER) ;

	// Build HTML Response String
	UtlString html(appHTML) ;

	// Add list of applications

#define MAX_APPS 15
#define MAX_URL_LENGTH 256

    int iActualApps = 0;
    int app;
    char* szApps[MAX_APPS];

    for (int j=0; j<MAX_APPS; j++) {
            szApps[j] = (char*) malloc(MAX_URL_LENGTH);
            memset(szApps[j], 0, MAX_URL_LENGTH);
    }

    JNI_queryInstalledJavaApplications(MAX_APPS, szApps, iActualApps);

	UtlString messageBody ;
    messageBody.append("<FORM ACTION=\"/cgi/applications.cgi\"> ");
    messageBody.append("<select name=\"uninstall\"><option selected value=\"\">--Select an application to Uninstall--");

    for (app=0; app < iActualApps; app++)
    {

        messageBody.append("<option value=\"");
        messageBody.append(szApps[app]);
        messageBody.append("\">");
        messageBody.append(szApps[app]);

    }

	//free memory for the app names
	for (int loop = 0;loop < MAX_APPS;loop++)
		free(szApps[loop]);
/*
/////////////////
	APPINFO appInfo[MAX_APPS];
	        JNI_queryInstalledJavaApplications(MAX_APPS, appInfo, iActualApps);
	     for (app=0; app < iActualApps; app++)
	     {
	         ////message body is the html string.
	        messageBody.append(appInfo[app].szTitle); //append title 
	        messageBody.append("-----");
	        messageBody.append(appInfo[app].szUrl); //append url
	     }

/////////////////
*/

    messageBody.append("</select>");

	html.append(messageBody) ;

	html.append(afterApps) ;

	UtlString installInfo ;


	// Insert install/uninstall information
    char buf[256];

    if (installURL.length() != 0)
    {
         if (retCodeInstall == CGI_INSTALL_APPLICATION_OK)
         {
             installInfo.append("<tr>\n<td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Status:</p>\n</td>\n<td colspan=\"2\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Installation Succeeded</p></td></tr>");
         } else if (retCodeInstall == CGI_INSTALL_APPLICATION_ERROR_URL) {
           // Malformed URL exception
             sprintf(buf, "<tr>\n<td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Status:</p>\n</td>\n<td colspan=\"2\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Installation Failed. The URL was not valid.</p></td></tr>");
             installInfo.append(buf);
         
         } else if (retCodeInstall == 
                    CGI_INSTALL_APPLICATION_ERROR_APP_ALREADY_INSTALLED) {
           // Application Already Exists
             sprintf(buf, "<tr>\n<td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Status:</p>\n</td>\n<td colspan=\"2\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Installation Failed. The application is already installed.</p></td></tr>");
             installInfo.append(buf);
         
         }else if (retCodeInstall == 
                   CGI_INSTALL_APPLICATION_ERROR_EXCEEDED_MAXIMUM_ALLOWED) {
           // Maximum quota of user applications that can be installed reached.
             sprintf(buf, "<tr>\n<td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Status:</p>\n</td>\n<td colspan=\"2\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Installation Failed. Exceeded maximum number of applications allowed.</p></td></tr>");
             installInfo.append(buf);
         
         } else if (retCodeInstall == 
                   CGI_INSTALL_APPLICATION_ERROR_TIMEOUT) {
           // Unable to load the application due to issues like incorrect url, 
		   // network connectivity problems, server from where the app is to be retrieved 
		   // is down, etc.
             sprintf(buf, "<tr>\n<td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Status:</p>\n</td>\n<td colspan=\"2\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Installation Failed. Timeout loading the application.</p></td></tr>");
             installInfo.append(buf);
         
         } else if (retCodeInstall == 
                   CGI_INSTALL_APPLICATION_ERROR_VERSIONFAILED) {
           // App version is not compatible with the core software version of the phone
             sprintf(buf, "<tr>\n<td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Status:</p>\n</td>\n<td colspan=\"2\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Installation Failed. Application version is not compatible with core software version on the phone.</p></td></tr>");
             installInfo.append(buf);
         
         } else {	
           // Any other failure
           sprintf(buf, "<tr>\n<td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Status:</p>\n</td>\n<td colspan=\"2\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Installation Failed. The phone failed to load the application.</p></td></tr>");
           installInfo.append(buf);
         }
         
    } else if (uninstallURL.length() != 0)
    {
         if (retCodeUninstall == CGI_UNINSTALL_APPLICATION_OK)
         {
             installInfo.append("<tr>\n<td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Status:</p>\n</td>\n<td colspan=\"2\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Uninstallation Succeeded</p></td></tr>");
         } else if(retCodeUninstall == CGI_UNINSTALL_APPLICATION_ERROR_URL) {
             sprintf(buf, "<tr>\n<td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Status:</p>\n</td>\n<td colspan=\"2\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Uninstallation Failed. The URL was not valid.</p></td></tr>");
             installInfo.append(buf);
         } else {
             sprintf(buf, "<tr>\n<td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Status:</p>\n</td>\n<td colspan=\"2\" bgcolor=\"#FFCC33\" valign=\"top\">\n<p>Uninstallation Failed. The application failed to unload the application.</p></td></tr>");
             installInfo.append(buf);		
		 }
	}

	html.append(installInfo) ;
	html.append("</table>") ;


	

	// Display loaded apps
    int iLoadedApps = 0;
    int iCount;
    char* szLoadedApps[MAX_APPS];

    for (int i=0; i<MAX_APPS; i++) {
            szLoadedApps[i] = (char*) malloc(MAX_URL_LENGTH);
            memset(szLoadedApps[i], 0, MAX_URL_LENGTH);
    }

    JNI_queryLoadedApplications(MAX_APPS, szLoadedApps, iLoadedApps);

	if (iLoadedApps > 0) {
		for (iCount=0; iCount < iLoadedApps; iCount++)
		{
			loadedApps.append("<td colspan=\"2\" bgcolor=\"#FFCC33\" valign=\"top\"><p>") ;
			loadedApps.append(szLoadedApps[iCount]);
			loadedApps.append("</p></td></tr>") ;
			
		}
		
		//free all the space for all the appnames
		for (int loop = 0;loop < MAX_APPS;loop++)
			free(szLoadedApps[loop]);

		loadedApps.append("</table>") ;

		html.append(loadedApps) ;
	}

	// Display rest of page
	html.append(afterStatusString) ;

    // Build the response message
    HttpBody* body = new HttpBody(html.data(),
        html.length(), CONTENT_TYPE_TEXT_HTML);
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_OK_CODE, HTTP_OK_TEXT);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);


}

// 
// Displays User Preferences in web page
// 
void Webui::getPingerConfigUser(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
	osPrintf("Webui::getPingerConfigUser reached") ;

	UtlString userName;
	UtlString html ;

	OsConfigDb configDbNew ;
	UtlString paramValue ;
	configDbNew.loadFromFile(CONFIGDB_NAME_IN_FLASH) ;
	configDbNew.loadFromFile(CONFIGDB_USER_IN_FLASH) ;

	UtlString top2(HTML_HEADER_1) ;
	top2.append("<title>sipXphone Preferences</title>") ;

	
	top2.append("\
<script language=\"Javascript\">\n\
function promptOnRestart() {\n\
var msg=\"You have selected to restart the phone.\\n\\n\
Restarting sipXphone will interrupt the current call, and the phone's ability to send and receive calls for up to 2 minutes.\\n\\n\
Are you sure you want to continue?\";\n\
if (confirm(msg))\n\
	window.location.href= \"/cgi/restart.cgi\" ;\n\
}\n\
</script>\n ");
 
    UtlBoolean  bIsSecure = FALSE;
    HttpMessage tmprequest = (HttpMessage)request;
    if (tmprequest.getSendProtocol() == OsSocket::SSL_SOCKET)
            bIsSecure = TRUE;

insertHeader(top2,bIsSecure) ;
	top2.append("\
<!--table for body of page-->\n\
<form name=\"pingerconfig\" method=\"post\" action=\"/cgi/setpingerconfiguser.cgi\">\n\
  <table width=\"750\" border=\"0\" cellspacing=\"10\">\n\
	<tr>\n\
		<td><H1>Call Handling Preferences</H1></td>\n\
	</tr>\n\
	<tr>\n\
	  <td width=\"40%\" valign=\"top\">\n\
            <p class=\"textOrange\">Instructions:</p>\n"
            "<p align=\"left\">On this page you can set up call forwarding and call waiting.<br><br>\n"
			"<b>Note:</b> Be sure to read the information provided for each option before you supply a value.<br><br>\n\
			When your entries are complete, click Save. Then click restart so that the changes will take effect. When you restart sipXphone, you will not be able to make or receive calls for approximately 2 minutes. <br>\n\
			If you need more help, check the SIPFoundry website at <a href=\"http://www.sipfoundry.org\">www.sipfoundry.org</a>.<br>\n\
      </td>\n\
		</tr>\n\
	<tr>\n\
    <td valign=\"top\" width=\"100%\">\n\
	<a name=\"dialing\"></a>\n");
		
	//////////////////
	html.append("\n\
			<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\"><tr>	\n\
				<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"center\"><input type=submit name=\"submit\" value=\"Save\">&nbsp;&nbsp;\n\
					  <input type=\"button\" name=\"RESTART\" value=\"Restart\" onClick=\"promptOnRestart();\">\n\
                  </td>\n\
			 </tr>\n\
			</table> \n ") ;

    ///////////////////
	html.append("		<!--\"Dialing Table\" table-->\n\
			<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
		   <a name=\"callhandling\"></a>		  \n\
			<!--\"CallHandling Table\" table-->\n\
			  <tr>\n\
			  	<td colspan=\"2\"><p class=\"textOrange\">Call Handling:</p></td>\n\
			  </tr>\n\
			   <!--PHONESET_FORWARD_UNCONDITIONAL-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>Do you want to forward all of your calls?  </p>\n\
				  <p><i>To forward all calls and override all other call handling settings set this value to Enable. Then, supply the alternate destination for your calls in the next field. To stop forwarding your calls, choose Disable.\n\
				  <font class=\"smallText\">(PHONESET_FORWARD_UNCONDITIONAL)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n");
					configDbNew.get("PHONESET_FORWARD_UNCONDITIONAL", paramValue) ;
	
                    html.append("<select NAME=\"phoneset_forward_unconditional\">		\n\
					  	<Option value=\"DISABLE\"");
					if(! paramValue.compareTo("DISABLE"))
					{
						html.append(" SELECTED");
					}
					html.append(">Disable\n\
						<Option value=\"ENABLE\"");
					if(! paramValue.compareTo("ENABLE"))
					{
						html.append(" SELECTED");
					}
					html.append(">Enable\n\
					  </Select>\n\
                  </td>\n\
               </tr>\n");
		  html.append("\
		 	  <!--SIP_FORWARD_UNCONDITIONAL-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>To forward all of your incoming calls automatically, set sipXphone's PHONESET_FORWARD_UNCONDITONAL value to ENABLE, then enter the SIP URL or telephone number of the destination phone or voice mail box here.<br>	\n\
				  <font class=\"smallText\">(SIP_FORWARD_UNCONDITIONAL)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
					<input type=\"text\" size=\"15\" name=\"sip_forward_unconditional\" value=\"");
					configDbNew.get("SIP_FORWARD_UNCONDITIONAL", paramValue) ;
					html.append(paramValue.data());
					html.append("\">\n\
				</td>\n\
			  </tr>		\n\
			   <!--PHONESET_DND-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>Do you want to set your phone to <b>Do Not Disturb</b>?</p>\n\
				  <p><i>Set this value to <b>Enable</b> when you don't want to receive any calls. To receive calls normally, choose <b>Disable</b>.<br>\n\
				  <font class=\"smallText\">(PHONESET_DND)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
                      <select NAME=\"phoneset_dnd\">		\n\
					  	<Option value=\"DISABLE\"");
					configDbNew.get("PHONESET_DND", paramValue) ;
	                if(! paramValue.compareTo("DISABLE"))
					{
						html.append(" SELECTED");
					}
					html.append(">Disable\n\
						<Option value=\"ENABLE\"");
					if(! paramValue.compareTo("ENABLE"))
					{
						html.append(" SELECTED");
					}
					html.append(">Enable\n\
					  </Select>\n\
                  </td>\n\
               </tr>\n\
			   <!--PHONESET_AVAILABLE_BEHAVIOR-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>When you receive a call and your sipXphone is available, do you want it to ring? Alternatively, you can forward the call to another destination.</p>\n\
				  <p><i> <b>Forward on No Answer</b> forwards your calls only after a certain number of rings. Supply this destination, and the number of rings, in fields below.</i><br>\n\
				  \n\
				  <font class=\"smallText\">(PHONESET_AVAILABLE_BEHAVIOR)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
                      <select NAME=\"phoneset_available_behavior\">\n\
					  	<Option value=\"RING\"");
					configDbNew.get("PHONESET_AVAILABLE_BEHAVIOR", paramValue) ;
					if(! paramValue.compareTo("RING"))
					{
						html.append(" SELECTED");
					}
					html.append(">Ring\n\
						<Option value=\"FORWARD_ON_NO_ANSWER\"");
					if( (! paramValue.compareTo("FORWARD_ON_NO_ANSWER")) ||
					    //backward compatibility..FORWARD is deprecated.
						(! paramValue.compareTo("FORWARD")) )
					{
						html.append(" SELECTED");
					}
					html.append(">Forward On No Answer\n\
					  </Select>		  \n\
                  </td>\n\
               </tr>\n ") ;
	
	html.append("\
		 	  <!--SIP_FORWARD_ON_NO_ANSWER-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>Enter the SIP URL of another phone or voice mail box to forward your incoming calls to if your phone is available but you do not answer.</p>\n\
				  <p><i>Set sipXphone's available behavior to <b>Forward on No Answer</b>, then enter the SIP URL of the destination for the calls you can't answer in time.</i><br>\n\
				  <font class=\"smallText\">(SIP_FORWARD_ON_NO_ANSWER)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
					<input type=\"text\" size=\"15\" name=\"sip_forward_on_no_answer\" value=\"");
					configDbNew.get("SIP_FORWARD_ON_NO_ANSWER", paramValue) ;
					html.append(paramValue.data());
					html.append("\">\n\
					</td>\n\
               </tr>	\n\
		 	  <!--PHONESET_NO_ANSWER_TIMEOUT-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>How long should your phone ring when you're unable to answer it? Enter the number of seconds that a phone must ring before it is considered to be unanswered.</p>\n\
				  <p><i>A ring is considered to be six seconds in length.</i><br>	\n\
				  <font class=\"smallText\">(PHONESET_NO_ANSWER_TIMEOUT)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
				<select NAME=\"phoneset_no_answer_timeout\">\n");
					  configDbNew.get("PHONESET_NO_ANSWER_TIMEOUT", paramValue) ;
            // if not set, default PHONESET_NO_ANSWER_TIMEOUT to "24" seconds
            if (paramValue.isNull()) paramValue = "24";

					char buffer[5];
					for( int i = 1; i<=5; i++ ){
						 sprintf(buffer, "%d", (i*6) );
                     
						 html.append(" <option value=\"");
						 html.append(buffer);
						 html.append("\"");
						 if(! paramValue.compareTo(buffer))
						 {
							html.append(" SELECTED");
						 }
						 html.append(">");
						 html.append(buffer );
						 html.append("\n");
					}
          // $$$ The following line is needed because of a OsString bug
          paramValue = OsUtil::NULL_OS_STRING;

					html.append("</Select>\n\
				</td>\n\
               </tr>		\n\
		 	  <!--PHONESET_CALL_WAITING_BEHAVIOR-->\n ") ;
	html.append("\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>Do you want to use call waiting when your phone is busy? Select <b>Alert</b> to use this feature, or <b>Busy</b> to make other choices when your phone is busy.<br>	\n\
				  <font class=\"smallText\">(PHONESET_CALL_WAITING_BEHAVIOR)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
					  <select NAME=\"phoneset_call_waiting_behavior\">\n\
					  	<Option value=\"ALERT\"");
					configDbNew.get("PHONESET_CALL_WAITING_BEHAVIOR", paramValue) ;
					if(! paramValue.compareTo("ALERT"))
					{
						html.append(" SELECTED");
					}
					html.append(">Alert\n\
						<Option value=\"BUSY\"");
					if(! paramValue.compareTo("BUSY"))
					{
						html.append(" SELECTED");
					}
					html.append(">Busy\n\
					  </Select>\n\
				  </td>\n\
               </tr>\n ") ;
	html.append("\
			  <!--PHONESET_BUSY_BEHAVIOR-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>When you receive a call and your sipXphone is busy, do you want it to play a busy signal? Choose <b>busy</b>. Alternatively, you can <b>forward</b> the call to another destination.</p>\n\
				  <p><i>This setting applies only when you are not using the call waiting feature (that is, your call waiting behavior is set to busy). If you choose <b>Forward</b>, supply the destination in the next field.</i><br>\n\
 				  <font class=\"smallText\">(PHONESET_BUSY_BEHAVIOR)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
                      <select NAME=\"phoneset_busy_behavior\">\n\
					  	<Option value=\"BUSY\"");
					configDbNew.get("PHONESET_BUSY_BEHAVIOR", paramValue) ;
					if(! paramValue.compareTo("BUSY"))
					{
						html.append(" SELECTED");
					}
					html.append(">Busy\n\
						<Option value=\"FORWARD\"");
					if(! paramValue.compareTo("FORWARD"))
					{
						html.append(" SELECTED");
					}
					html.append(">Forward\n\
					  </Select>\n\
                  </td>\n\
               </tr>\n\
			  <!--SIP_FORWARD_ON_BUSY-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>Enter the SIP URL of another phone or voice mail box to forward your incoming calls to if your phone is busy.</p>\n\
				  <p><i>Set sipXphone's busy behavior to <b>Forward</b>, then enter the SIP URL of the destination for the calls that come in when your phone is busy. </i><br>\n\
				  <font class=\"smallText\">(SIP_FORWARD_ON_BUSY)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
					<input type=\"text\" size=\"15\" name=\"sip_forward_on_busy\" value=\"");
					configDbNew.get("SIP_FORWARD_ON_BUSY", paramValue) ;
					html.append(paramValue.data());
					html.append("\">\n\
				</td>\n\
               </tr>\n ") ;
	html.append("\
			  <!--PHONESET_RINGER-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>How would you like to be alerted to an incoming call? Choose <b>Audible</b>, <b>Visual</b>, or <b>Both</b>.</p>	\n\
				  <font class=\"smallText\">(PHONESET_RINGER)</font>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
                      <select NAME=\"phoneset_ringer\">\n\
					  	<Option value=\"AUDIBLE\"");
	configDbNew.get("PHONESET_RINGER", paramValue) ;

  // if not set, default PHONESET_RINGER to "BOTH"
  if (paramValue.isNull()) paramValue = "BOTH";

					if(! paramValue.compareTo("AUDIBLE"))
					{
						html.append(" SELECTED");
					}
					html.append(">Audible\n\
						<Option value=\"VISUAL\"");
					if(! paramValue.compareTo("VISUAL"))
					{
						html.append(" SELECTED");
					}
					html.append(">Visual\n\
						<Option value=\"BOTH\"");
					if(! paramValue.compareTo("BOTH"))
					{
						html.append(" SELECTED");
					}
					html.append(">Both\n\
					  </Select>          \n\
				</td>\n\
               </tr>			   \n\
              </table>\n\
			  <!--End \"CallHandling\" table-->\n ") ;
        // $$$ The following line is needed because of a OsString bug
        paramValue = OsUtil::NULL_OS_STRING;

	html.append("\n\
			<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\"><tr>	\n\
				<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"center\"><input type=submit name=\"submit\" value=\"Save\">&nbsp;&nbsp;\n\
					  <input type=\"button\" name=\"RESTART\" value=\"Restart\" onClick=\"promptOnRestart();\"></form>\n\
                  </td>\n\
			 </tr>\n\
			</table> \n ") ;

                  ///////////////////
		

	html.append("\
		</td>\n\
	</tr>\n\
  </table>\n ") ;

	html.append(HTML_FOOTER) ;
	


	// Top of HTML response
	UtlString htmlResponse(top2) ;


	// Add status code (if any)
	UtlString message ;
	requestContext.getCgiVariable("MESSAGE", message) ;
	htmlResponse.append("<p><font color=\"Red\"><br>") ;
	htmlResponse.append(message) ;
	htmlResponse.append("</font></p>") ;

	// Add rest of HTML
	htmlResponse.append(html) ;

	// Build Response
    response = new HttpMessage();
    HttpBody* body = new HttpBody(htmlResponse.data(),
        htmlResponse.length(), CONTENT_TYPE_TEXT_HTML);
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_OK_CODE, HTTP_OK_TEXT);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);
}


void Webui::getPingerConfigAdminMiddle(UtlString& html, OsConfigDb& configDbNew)
{
	char buffer[5];
	UtlString paramValue ;
	UtlString strMultipleValues;

html.append("\n\
			<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\"><tr>	\n\
				<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"center\"><input type=submit name=\"submit\" value=\"Save\">&nbsp;&nbsp;\n\
					  <input type=\"button\" name=\"RESTART\" value=\"Restart\" onClick=\"promptOnRestart();\">\n\
                  </td>\n\
			 </tr>\n\
			</table> \n ") ;


html.append("		<!--\"Dialing Table\" table-->\n\
			<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
			  <tr>\n\
			  	<td colspan=\"2\"><p class=\"textOrange\">Call Addressing:</p></td>\n\
			  </tr>");
	configDbNew.get("PHONESET_DIALPLAN_LENGTH", paramValue) ;
   if (paramValue.isNull())
   {
      paramValue = "12";
   }
	html.append("\
			   <!--PHONESET_DIALPLAN_LENGTH-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
				   <p>When you make a call, how many digits does the number usually have? For example, a 3-digit extension number, a 7-digit local number, a longer number?</p>\n\
				   <p><i>You set this parameter to help sipXphone speed up dialing: when you dial a number that is this long, sipXphone will automatically dial it (after a short pause). You won't have to press Dial or #.<br>\n\
				  <font class=\"smallText\">(PHONESET_DIALPLAN_LENGTH)</font></p>\n\
                </td>\n\
				<td bgcolor=\"#FFCC33\">\n\
                      <select NAME=\"phoneset_dialplan_length\">\n");

	for( int i = 1; i<=20; i++ ){
		sprintf(buffer, "%d", i );
                     
		html.append(" <Option value=\"");
		html.append(buffer);
		html.append("\"");
		if(! paramValue.compareTo(buffer))
		{
			html.append(" SELECTED");
		}
		html.append(">");
		html.append(buffer );
		html.append("\n");
	}


	html.append("</Select>\n</td>\n</tr>\n ") ;

	html.append("\
			  <!--PHONESET_DIGITMAP-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>To set up a digitmap, specify a dial plan and its corresponding SIP address separated by a colon (:).<br>Digitmaps override a simple dial plan length (above).</p>\n\
 				  <p><i>An example of a digitmap for local phone calls might look like this:<br>\n\
					<b>9xxxxxxx : sip:{digits}@localprovider.net</b>. See <u>http://www.sipfoundry.org</u> for detailed information.</i><font class=\"smallText\"><br>(PHONESET_DIGITMAP)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
                      <textarea rows=\"10\" cols=\"40\" name=\"phoneset_digitmap\">");
						createJsConfigAdmin(strMultipleValues, "PHONESET_DIGITMAP") ;
						html.append(strMultipleValues) ;
						strMultipleValues.remove(0);
						html.append("</textarea>\n\
									</td>\n\
									</tr>	   			   \n\
									</table>\n\n\
									<!--End \"Dialing\" table-->\n") ;


    html.append("\n\
		<a name=\"network\"></a>		  \n\
			<!--\"Network Settings Table\" table-->\n\
			<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
			  <tr>\n\
			  	<td colspan=\"2\"><p class=\"textOrange\">Network Settings:</p></td>\n\
			  </tr>\n\
			  <!--PHONESET_RTP_PORT_START-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>Does your sipXphone make calls through a firewall using NAT (Network Address Translation)? Define the starting port number for RTP and RTCP transmission.</p>	\n\
				  <font class=\"smallText\">(PHONESET_RTP_PORT_START)</font>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
                      <input type=\"text\" size=\"15\" name=\"phoneset_rtp_port_start\" value=\"");
					  configDbNew.get("PHONESET_RTP_PORT_START", paramValue) ;
						html.append(paramValue.data());
						html.append("\">\n\
                  </td>\n\
               </tr>	\n  ") ;

	html.append("\
			   <!--PHONESET_HTTP_PORT-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>Do you want to run the phone's embedded web server? If so, enter the port number it is to run on:</p>	\n\
				  <p><i>Enter 0 to disable HTTP server</i><br>	\n\
					<font class=\"smallText\">(PHONESET_HTTP_PORT)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
                      <input type=\"text\" size=\"15\" name=\"phoneset_http_port\" value=\"");
						configDbNew.get("PHONESET_HTTP_PORT", paramValue) ;
						html.append(paramValue.data());
						html.append("\">\n\
                  </td>\n\
               </tr>\n");
    html.append("\
    				<!--PHONESET_EXTERNAL_IP_ADDRESS-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>If making calls from behind a NAT firewall, please add NAT port mappings on your NAT firewall device and specify your external IP here.  Please see your NAT firewall for instructions on adding port mappings.  By default, you will need to map port 5060 (SIP) and ports 8766-8770 (RTP):<br>\n\
 				  <font class=\"smallText\">(PHONESET_EXTERNAL_IP_ADDRESS)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
                      <input type=\"text\" size=\"15\" name=\"phoneset_external_ip_address\" value=\"");
						configDbNew.get("PHONESET_EXTERNAL_IP_ADDRESS", paramValue) ;
						html.append(paramValue.data());
						html.append("\">\n\
                  </td>\n\
               </tr>\n");

	configDbNew.get(SIP_SYMMETRIC_SIGNALING, paramValue) ;
	html.append("\
			  <!--SIP_SYMMETRIC_SIGNALING-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>To receive SIP responses and requests on the same port used to send outbound requests, enable SIP symmetric signaling.  Generally, symmetric signaling helps traverse NAT firewalls; however, some early failures indicators are lost resulting in a longer delay before calls fails.<p>If unsure, ENABLE symmetric signaling.<p>\n\
				  <font class=\"smallText\">(SIP_SYMMETRIC_SIGNALING)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
                      <select NAME=\"SIP_SYMMETRIC_SIGNALING\">\n\
					  	<Option value=\"DISABLE\"");
					if(paramValue.compareTo("DISABLE") == 0)
					{
						html.append(" SELECTED");
					}
					html.append(">DISABLE\n\
						<Option value=\"ENABLE\"");
					if(paramValue.compareTo("DISABLE") != 0)
					{
						html.append(" SELECTED");
					}
					html.append(">ENABLE</Select>\n\
                  </td>\n\
               </tr>\n\n") ;

	html.append("\
			  <!--SIP_STUN_SERVER-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>To enable STUN discovery of NAT address, enter your STUN server hostname or IP address.<p>Your system administrator should provide you with this setting.<p>\n\
				  <font class=\"smallText\">(SIP_STUN_SERVER)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
                      <input type=\"text\" size=\"15\" name=\"SIP_STUN_SERVER\" value=\"");
						configDbNew.get("SIP_STUN_SERVER", paramValue) ;
						html.append(paramValue.data());
						html.append("\">\n\
                  </td>\n\
               </tr>\n\n") ;

	html.append("\
			  <!--SIP_STUN_REFRESH_PERIOD-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>Enter the keep alive time in seconds for STUN connections.  By default, sipXphone will send STUN requests every 28 seconds to ensure that firewall holes are kept open.<p>\n\
				  <font class=\"smallText\">(SIP_STUN_REFRESH_PERIOD)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
                      <input type=\"text\" size=\"15\" name=\"SIP_STUN_REFRESH_PERIOD\" value=\"");
						configDbNew.get("SIP_STUN_REFRESH_PERIOD", paramValue) ;
						html.append(paramValue.data());
						html.append("\">\n\
                  </td>\n\
               </tr>\n\n") ;

    html.append("\
			   <!--PHONESET_HTTP_PROXY_HOST-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>Is your sipXphone installed behind a proxied firewall? Before you can install a sipXphone application .JAR file, you must identify the HTTP proxy host and port.</p>\n\
					<p><i>Enter a domain name such as proxy.pingtel.com.</i><br>	\n\
				  <font class=\"smallText\">(PHONESET_HTTP_PROXY_HOST)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
                      <input type=\"text\" size=\"15\" name=\"phoneset_http_proxy_host\" value=\"");
						configDbNew.get("PHONESET_HTTP_PROXY_HOST", paramValue) ;
						html.append(paramValue.data());
						html.append("\">\n\
                </td>\n\
               </tr>		\n ") ;



	html.append("<!--PHONESET_HTTP_PROXY_PORT-->\n"
              	"<tr>\n"
                "<td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n"
  		"<p>If your sipXphone is installed behind a proxied firewall, enter an HTTP port number such as 8080.</p>\n"
		"<font class=\"smallText\">(PHONESET_HTTP_PROXY_PORT)</font>\n"
                "</td>\n"
                "<td bgcolor=\"#FFCC33\">\n"
                "<input type=\"text\" size=\"15\" name=\"phoneset_http_proxy_port\" value=\"");

	configDbNew.get("PHONESET_HTTP_PROXY_PORT", paramValue) ;
	html.append(paramValue.data());
	html.append("\">\n\
                </td>\n\
               </tr>				   \n");

	html.append("\
			   <!--PHONESET_MSG_WAITING_SUBSCRIBE-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\" colspan=\"2\">\n\
				  <p>To subscribe to message waiting status indication from the sipXphone user’s voice mail box, enter the subscription address of the voicemail server. The voice mail server will then notify the phone each time the message waiting status changes.</p>  \n\
				  <p><i>Refer to the documentation supplied with your voice mail server for the format of this subscription address.</i><br>	\n\
				  <font class=\"smallText\">(PHONESET_MSG_WAITING_SUBSCRIBE)</font></p>\n\
                      <input type=\"text\" size=\"100\" name=\"phoneset_msg_waiting_subscribe\" value=\"");
						configDbNew.get("PHONESET_MSG_WAITING_SUBSCRIBE", paramValue) ;
						html.append(paramValue.data());
						html.append("\">\n\
                  </td><br>\n\
               </tr>\n");

	html.append("\
			   <!--PHONESET_VOICEMAIL_RETRIEVE-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\" colspan=\"2\">\n\
				<p>For the phone user to retrieve waiting voice mail messages with a single button press, enter the complete SIP URL for retrieving messages from the target mailbox.</p>  \n\
				<p><i>Refer to the documentation supplied with your voice mail server for the format of this SIP URL.</i><br> \n\
				<font class=\"smallText\">(PHONESET_VOICEMAIL_RETRIEVE)</font></p>\n\
					<input type=\"text\" size=\"100\" name=\"phoneset_voicemail_retrieve\" value=\"");
						configDbNew.get("PHONESET_VOICEMAIL_RETRIEVE", paramValue) ;
						html.append(paramValue.data());
						html.append("\">\n\
                  </td><br>\n\
               </tr>\n");

	html.append("</table>\n\n") ;

    html.append("\
   	<a name=\"sip\"></a>		  \n\
		  <table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">	\n\
		  	  <tr>\n\
			 	<td colspan=\"2\"><p class=\"textOrange\">SIP Servers: </p></td>\n\
			  </tr>\n\
			\n");
}


//
// Displays Phone Configuration values in web page
//
void Webui::getPingerConfigAdmin(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
	osPrintf("Webui::getPingerConfigAdmin reached") ;

	UtlString html ;
	UtlString userName ;
	UtlString paramValue;
	UtlString strMultipleValues;
//	char buffer[5];

	requestContext.getEnvironmentVariable(HttpRequestContext::HTTP_ENV_USER, userName);

	OsConfigDb configDbNew ;
	configDbNew.loadFromFile(CONFIGDB_NAME_IN_FLASH) ;
	configDbNew.loadFromFile(CONFIGDB_USER_IN_FLASH) ;

	UtlString top2(HTML_HEADER_1) ;
	top2.append("<title>sipXphone Configuration</title>") ;
	

	top2.append("\
<script language=\"Javascript\">\n\
function promptOnRestart() {\n\
var msg=\"You have selected to restart the phone.\\n\\n\
Restarting sipXphone will interrupt your ability to send and receive calls for up to 2 minutes.\\n\\n\
Are you sure you want to continue?\";\n\
if (confirm(msg))\n\
	location.replace(\"/cgi/restart.cgi\") ;\n\
}\n\
</script>\n ") ;
// Add Config parameters as Javascript function
//UtlString script ;
//createJsConfigAdmin(script) ;
//top2.append(script) ;

    UtlBoolean  bIsSecure = FALSE;
    HttpMessage tmprequest = (HttpMessage)request;
    if (tmprequest.getSendProtocol() == OsSocket::SSL_SOCKET)
            bIsSecure = TRUE;

insertHeader(top2,bIsSecure) ;

top2.append("\
<!--table for body of page-->\n\
<form name=\"pingerconfig\" method=\"post\" action=\"/cgi/setpingerconfigadmin.cgi\">\n\
  <table width=\"750\" border=\"0\" cellspacing=\"10\">\n\
	<tr>\n\
		<td><H1>Phone Configuration</H1></td>\n\
	</tr>\n\
	<tr>\n\
	  <td width=\"40%\" valign=\"top\">\n\
            <p class=\"textOrange\">Instructions:</p>\n\
            <p align=\"left\">On this page you can enter, verify, and change the values that sipXphone requires to work efficiently in your organization's network.\n\
			Note: Some parameters require you to enter values in a specific format. Be sure to read the information provided for a parameter before you enter a value.</p>\n\
			<p>When your entries are complete, press <b>Save</b>. Then <b>Restart</b> sipXphone so that the changes will take effect.<br>\n\
			Note: Restarting a phone interrupts any ongoing call and prevents the phone user from making and receiving calls for up to 2 minutes.<br>\n\
			If you need more help, check the SIPFoundry website at <a href=\"http://www.sipfoundry.org\">www.sipfoundry.org</a>.</p>\n\
      </td>\n\
		</tr>\n\
	<tr>\n\
    <td valign=\"top\" width=\"100%\">\n\
	<a name=\"dialing\"></a>\n");

	getPingerConfigAdminMiddle(html, configDbNew) ;	

/* took these next two out.  We don't want to expose them to the user just yet */
/*	html.append("\
			  <!--SIP_CONFIG_SERVER_ADDRESS-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
					<p>Host name or IP address of the SIP config server.<br>\n\
					If this configuration setting is not present or is set to NULL, the phone will attempt to \"discover\" the \n\
					address of the SIP config server.<br></p>\n\
					<p>For the present, this means it will attempt to contact the default SIP Config Server (\"sipuaconfig\").<br></p>\n\
				  <font class=\"smallText\">(SIP_CONFIG_SERVER_ADDRESS)</font> 	\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
					<input type=\"text\" size=\"15\" name=\"sip_config_server_address\" value=\"");
						configDbNew.get("SIP_CONFIG_SERVER_ADDRESS", paramValue) ;
						html.append(paramValue.data());
						html.append("\">\n\
				</td>\n\
               </tr>	\n");
	html.append("\
			  <!--SIP_CONFIG_SERVER_PORT-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
					<p>SIP config server port.<br>\n\
					If this configuration setting is:<br>\n\
					  not present<br>\n\
					  set to NULL<br>\n\
					  set to an integer <= zero<br>\n\
					then the phone will attempt to discover the SIP config server port.  For the present, this means it will <br>\n\
					attempt to use the default port to contact the SIP Config Server.<br>\n\
				  <font class=\"smallText\">(SIP_CONFIG_SERVER_PORT)</font> \n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
					<input type=\"text\" size=\"15\" name=\"sip_config_server_port\" value=\"");
						configDbNew.get("SIP_CONFIG_SERVER_PORT", paramValue) ;
						html.append(paramValue.data());
						html.append("\">\n\
				</td>\n\
               </tr>	\n");
*/

	html.append("\
			  <!--SIP_DIRECTORY_SERVERS-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>Enter the address of the SIP Proxy or Redirect Server that your installation uses to convert dialed numbers into SIP addresses.</p>\n\
				  <p><i>Generally, this is entered in the format sip:[domain name].com.</i><br>\n\
				  <font class=\"smallText\">(SIP_DIRECTORY_SERVERS)</font> </p>	\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
					<input type=\"text\" size=\"15\" name=\"sip_directory_servers\" value=\"");
						configDbNew.get("SIP_DIRECTORY_SERVERS", paramValue) ;
						html.append(paramValue.data());
						html.append("\">\n\
				</td>\n\
               </tr>	\n\
			   ") ;
	html.append("\
		 	  <!--SIP_PROXY_SERVERS-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>If all calls must go through a Proxy Server at your installation (similar to a firewall), enter its address here.</p>\n\
				  <p><i>Generally, this is entered in the format sip:[domain name].com.</i><br>	\n\
				  <font class=\"smallText\">(SIP_PROXY_SERVERS)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
					<input type=\"text\" size=\"15\" name=\"sip_proxy_servers\" value=\"");
			configDbNew.get("SIP_PROXY_SERVERS", paramValue) ;
						html.append(paramValue.data());
						html.append("\">\n\
				</td>\n\
               </tr>	\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>Supply the number of seconds until your sipXphone's registration with the Registry Server expires.\n\
							Your phone automatically re-registers itself with each registry server defined for the device or user \n\
							line(s) before this time period elapses.</p>	\n\
				  <font class=\"smallText\">(SIP_REGISTER_PERIOD)</font>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
					<input type=\"text\" size=\"15\" name=\"sip_register_period\" value=\"");
						configDbNew.get("SIP_REGISTER_PERIOD", paramValue) ;
						html.append(paramValue.data());
						html.append("\">\n\
				</td>\n\
               </tr>\n ") ;


	html.append("\
			  <!--SIP_TCP_PORT-->			   \n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>Identify the IP ports on which SIP TCP messages are expected.</p>\n\
				  <p><i>Should be set to the same value as SIP_UDP_PORT.</i><br>	\n\
				  <font class=\"smallText\">(SIP_TCP_PORT)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
					<input type=\"text\" size=\"15\" name=\"sip_tcp_port\" value=\"");
						configDbNew.get("SIP_TCP_PORT", paramValue) ;
						html.append(paramValue.data());
						html.append("\">\n\
				</td>\n\
               </tr>	\n\
			  <!--SIP_UDP_PORT-->			   \n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>Identify the IP ports on which SIP UDP messages are expected.</p>\n\
				  <p><i>Should be set to the same value as SIP_TCP_PORT.</i><br>	\n\
				  <font class=\"smallText\">(SIP_UDP_PORT)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
					<input type=\"text\" size=\"15\" name=\"sip_udp_port\" value=\"");
						configDbNew.get("SIP_UDP_PORT", paramValue) ;
						html.append(paramValue.data());
						html.append("\">\n\
				</td>\n\
               </tr>	\n\
			   <!--SIP_SESSION_REINVITE_TIMER-->			   \n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>Enter a number of seconds to pause between sending session reinvite messages during calls. These messages can help track call duration.</p>\n\
				  <p><i>All phones participating in a call must support SIP session reinvite for these messages to be sent.</i><br>	\n\
				  <font class=\"smallText\">(SIP_SESSION_REINVITE_TIMER)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
					<input type=\"text\" size=\"15\" name=\"sip_session_reinvite_timer\" value=\"");
						configDbNew.get("SIP_SESSION_REINVITE_TIMER", paramValue) ;
						html.append(paramValue.data());
						html.append("\">\n\
				</td>\n\
               </tr>\n\
			  </table>\n\n\
			  <!--End \"Deployment Server\" table-->\n ") ;

	configDbNew.get("PHONESET_DND_METHOD", paramValue) ;
	html.append("\
		  <table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">	\n\
		  	  <tr>\n\
			 	<td colspan=\"2\"><p class=\"textOrange\">Do Not Disturb: </p></td>\n\
			  </tr>\n\
			  <!--PHONESET_DND_METHOD-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>This parameter specifies how calls will be handled when the user has the PHONESET_DND method set to ENABLE.<p>\n\
				  <p><i>To send a busy response to all SIP call invitations, set this parameter to <b>Send Busy</b>. To use the phone's call forwarding preferences, choose <b>Forward on No Answer</b> to send incoming calls to the <b>Forward on No Answer</b> destination, or <b>Forward on Busy</b> to send calls to the <b>Forward on Busy</b> destination.</i><br>				  \n\
				  <font class=\"smallText\">(PHONESET_DND_METHOD)</font></p>\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
                      <select NAME=\"phoneset_dnd_method\">		\n\
					  	<Option value=\"SEND_BUSY\"");
					if(! paramValue.compareTo("SEND_BUSY"))
					{
						html.append(" SELECTED");
					}
					html.append(">SEND_BUSY\n\
						<Option value=\"FORWARD_ON_BUSY\"");
					if(! paramValue.compareTo("FORWARD_ON_BUSY"))
					{
						html.append(" SELECTED");
					}
					html.append(">FORWARD_ON_BUSY\n\
						<Option value=\"FORWARD_ON_NO_ANSWER\"");
					if(! paramValue.compareTo("FORWARD_ON_NO_ANSWER"))
					{
						html.append(" SELECTED");
					}
					html.append(">FORWARD_ON_NO_ANSWER</Select>\n\
                  </td>\n\
               </tr>\n\
		  </table>\n\n ") ;

	html.append("\
	  <!--\"Phone Display\" table-->		  \n\
		  <table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">	\n\
		  	  <tr>\n\
			 	<td colspan=\"2\"><p class=\"textOrange\">Phone Display:</p></td>\n\
			  </tr>\n\
			  <!--PHONESET_LOGO_URL-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>To specify an image file to display on sipXphone's home screen, enter its URL.</p>\n\
				  <p><i>The image must be a file in .gif or .jpg format.<br>Enter the URL in the format http://[domain]/[image name].gif.</i></p>	\n\
				  <font class=\"smallText\">(PHONESET_LOGO_URL)</font>\n\
                </td>\n\
			   <td bgcolor=\"#FFCC33\">\n\
   					<input type=\"text\" size=\"40\" name=\"phoneset_logo_url\"  value=\"");
						configDbNew.get("PHONESET_LOGO_URL", paramValue) ;
						html.append(paramValue.data());
						html.append("\">\n\
					</td>\n\
			  </tr>\n\
		 </table>\n\n") ;

  // $$$ The following line is needed because of a OsString bug
  paramValue = OsUtil::NULL_OS_STRING;


	html.append("\
		  <table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">	\n\
		  	  <tr>\n\
			 	<td colspan=\"2\"><p class=\"textOrange\">Additional Parameters: </p></td>\n\
			  </tr>\n\
			  <!--Additional Parameters-->\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <p>Enter any additional parameters here.</p>\n\
				  <p><i>For all additional parameters, enter the parameter name, then a colon (:), then its value. For example, <b>PHONESET_HTTP_PORT : 80</b></i></p>	\n\
                </td>\n\
                <td bgcolor=\"#FFCC33\">\n\
					<textarea rows=\"10\" cols=\"60\" name=\"additional_parameters\">");
					createJsConfigAdmin(strMultipleValues, "") ;
						html.append(strMultipleValues) ;
						strMultipleValues.remove(0);
					html.append("</textarea>\n\
                </td>\n\
               </tr>\n\
			   </table>\n\n");

	html.append("\
			<table width=\"100%\" border=\"0\" cellpadding=\"1\" cellspacing=\"2\">	\n\
				<!--Note about where to set multi-lines-->\n\
				<tr>\n\
                <td width=\"600\" bgcolor=\"#FFFFFF\" valign=\"top\">\n\
                  <p>You can also set up identifying <a href=\"MultiLine.cgi\">device line</a> information for sipXphone and optional <a href=\"MultiLine.cgi\">user lines</a> for the people and entities that use it.</p>\n\
                </td>\n\
               </tr>\n\
		   </table>\n\n");

     html.append("\
                 <table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\"><tr>	\n\
				<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"center\"><input type=submit name=\"submit\" value=\"Save\">&nbsp;&nbsp;\n\
					  <input type=\"button\" name=\"RESTART\" value=\"Restart\" onClick=\"promptOnRestart();\"></form>\n\
                  </td>\n\
			 </tr>\n\
			</table> \n ") ;


	html.append("\
		</td>\n\
	</tr>\n\
  </table>\n ") ;

	html.append(HTML_FOOTER) ;

	// Check user is "admin," if not redirect to not authorized page
	if (userName.compareTo("admin")!=0) {
		osPrintf("Webui::getPingerConfigAdmin - user is not admin\n") ;
		const char * text = "<HTML><BODY>User Not Authorized<br><br>Must be user \"admin\" to access this page.</BODY></HTML>\n" ;
		HttpServer::processUserNotAuthorized(requestContext,request,response, text) ;
	} else {

	// Top of HTML response
	UtlString htmlResponse(top2) ;

	// Add status code (if any)
	UtlString message ;
	requestContext.getCgiVariable("MESSAGE", message) ;
	htmlResponse.append("<p><font color=\"Red\"><br>") ;
	htmlResponse.append(message) ;
	htmlResponse.append("</font></p>") ;

	// Add rest of HTML
	htmlResponse.append(html) ;

	// Build Response
    response = new HttpMessage();
    HttpBody* body = new HttpBody(htmlResponse.data(),
        htmlResponse.length(), CONTENT_TYPE_TEXT_HTML);
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_OK_CODE, HTTP_OK_TEXT);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);

	}
}

//
// Set configuration values for Preferences page
//
void Webui::setPingerConfigUser(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
	UtlString strMessage;						 // Response message
	OsConfigDb* changeDb = new OsConfigDb() ; //store parameters user has changed

	// Get configuration information from config files
	OsConfigDb configDb ;
	configDb.loadFromFile(CONFIGDB_NAME_IN_FLASH) ;
	configDb.loadFromFile(CONFIGDB_USER_IN_FLASH) ;

	// User-config parameters set in Preferences page
	const char* paramName[] = { 
		CONFIG_PHONESET_AVAILABLE_BEHAVIOR,
		CONFIG_PHONESET_BUSY_BEHAVIOR,
		CONFIG_PHONESET_CALL_WAITING_BEHAVIOR,
		CONFIG_PHONESET_DND,
		CONFIG_PHONESET_FORWARD_UNCONDITIONAL,
		CONFIG_PHONESET_NO_ANSWER_TIMEOUT,
		CONFIG_PHONESET_RINGER,
		CONFIG_PHONESET_TIME_DST_RULE,
		CONFIG_PHONESET_TIME_OFFSET,
		CONFIG_PHONESET_TIME_SERVER,
		SIP_FORWARD_ON_BUSY,
		SIP_FORWARD_ON_NO_ANSWER,
		SIP_FORWARD_UNCONDITIONAL,
		0
	} ;

	int i = 0 ;
	UtlString paramValue ;
	UtlString oldValue ;
	paramValue.remove(0) ;
	oldValue.remove(0) ;

	// Parameter validation - perform basic validation on config parameters

	UtlString strParamValue ;
	UtlString strForward ;
	int iValidationError = 0 ;

	requestContext.getCgiVariable(CONFIG_PHONESET_FORWARD_UNCONDITIONAL, strParamValue) ;
	if (strParamValue.compareTo("ENABLE")==0) {
		// forward URL must be set
		requestContext.getCgiVariable(SIP_FORWARD_UNCONDITIONAL, strForward) ;
		if (strForward.length()==0) {
			strMessage += " Please enter a value for SIP_FORWARD_UNCONDITIONAL\n" ;
			iValidationError = 1;
		}
	}

	strParamValue.remove(0) ;
	strForward.remove(0) ;
	

	requestContext.getCgiVariable(CONFIG_PHONESET_AVAILABLE_BEHAVIOR, strParamValue) ;
	if (strParamValue.compareTo("FORWARD_ON_NO_ANSWER")==0) {
		// forward URL must be set
		requestContext.getCgiVariable(SIP_FORWARD_ON_NO_ANSWER, strForward) ;
		if (strForward.length()==0) {
			strMessage = " Please enter a value for SIP_FORWARD_ON_NO_ANSWER" ;
			iValidationError = 1;
		}
	}
	
	strParamValue.remove(0) ;
	strForward.remove(0) ;

	
	requestContext.getCgiVariable(CONFIG_PHONESET_BUSY_BEHAVIOR, strParamValue) ;
	if (strParamValue.compareTo("FORWARD")==0) {
		// forward URL must be set
		requestContext.getCgiVariable(SIP_FORWARD_ON_BUSY, strForward) ;
		if (strForward.length()==0) {
			strMessage += "<br> Please enter a value for SIP_FORWARD_ON_BUSY" ;
			iValidationError = 1;
		}
	}

	if (iValidationError == 0) {
		while (paramName[i] != 0) {
			requestContext.getCgiVariable((const char *) paramName[i], paramValue) ;

			configDb.get(paramName[i], oldValue) ;

			if (oldValue.compareTo(paramValue.data()) != 0) {
				// value has changed
				changeDb->set(paramName[i], paramValue) ;
			}
			i++ ;
		}

		//If anything in new Db, write to pinger-config and user-config
		if (changeDb->isEmpty() == FALSE) {
			setConfigValuesUser(changeDb) ;
			//added by Harippriya on Aug 2, 2001.
			//This method refreshes the java layer and hence changes to 
			//pinger-config take effect even without restarting the phone.
			JXAPI_onConfigurationChanged();
			strMessage.append("New configuration saved.") ;
		}
	}


	// Redirect user to configuration page.

    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
    HTTP_MOVED_TEMPORARILY_CODE, HTTP_MOVED_TEMPORARILY_TEXT);

	UtlString location("/cgi/xpressaconfiguser.cgi");
	location.append("?MESSAGE=");
	HttpMessage::escape(strMessage) ;
	location.append(strMessage.data()) ;
    response->setLocationField(location);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);

}

//
// Set administration configuration values
//
void Webui::setPingerConfigAdmin(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
	UtlString message;							// Response message
	OsConfigDb* changeDb = new OsConfigDb() ;	// store parameters user has changed
	int i = 0 ;									// loop count
	UtlString paramValue ;						// value of param
	UtlString oldValue ;							// current value in pinger/user-config

	// Individual parameters set in web page configuration page
	const char* paramName[] = {	
		CONFIG_PHONESET_ADMIN_DOMAIN,
		CONFIG_PHONESET_DND_METHOD,
		CONFIG_PHONESET_DEPLOYMENT_SERVER,
		CONFIG_PHONESET_DIALPLAN_LENGTH,
		CONFIG_PHONESET_EXTERNAL_IP_ADDRESS,
        SIP_SYMMETRIC_SIGNALING,
        SIP_STUN_SERVER,
        SIP_STUN_REFRESH_PERIOD,
		CONFIG_PHONESET_HTTP_PORT,
		CONFIG_PHONESET_HTTP_PROXY_HOST,
		CONFIG_PHONESET_HTTP_PROXY_PORT,
		CONFIG_PHONESET_LOGICAL_ID,
		CONFIG_PHONESET_LOGO_URL,
		CONFIG_PHONESET_RTP_PORT_START,
		CONFIG_PHONESET_TELNET_ACCESS,
		PHONESET_MSG_WAITING_SUBSCRIBE,
		PHONESET_SNMP_TRAP_DESTS,
		PHONESET_VOICEMAIL_RETRIEVE,
		SIP_AUTHENTICATE_REALM,
		SIP_AUTHENTICATE_SCHEME,
		SIP_DIRECTORY_SERVERS,
		SIP_PROXY_SERVERS,	
		SIP_REGISTER_PERIOD,
		SIP_SESSION_REINVITE_TIMER,
		SIP_TCP_PORT,
		SIP_UDP_PORT,
		0
	} ;

	// Parameter validation - performs basic validation of config parameters



	// Multiple parameters in web page - these params can have multiple values
	const char* paramNameMultiple[] = {
		CONFIG_PHONESET_DIGITMAP,    
		SIP_AUTHENTICATE_DB,
		SIP_AUTHORIZE_USER, 
		SIP_AUTHORIZE_PASSWORD, 
		0
  } ;

	// Traverse individual parameters
	while (paramName[i] != 0) {
		paramValue.remove(0) ;
		requestContext.getCgiVariable(paramName[i], paramValue) ;
		changeDb->set(paramName[i], paramValue) ;
		i++ ;
	}

	// Set Multiple Values
	int j = 0 ;
	while (paramNameMultiple[j]!=0) {
		requestContext.getCgiVariable(paramNameMultiple[j],paramValue) ;
		setMultipleValues(paramNameMultiple[j], paramValue, *changeDb) ;
		j++ ;
	}

	// Set Additional parameters
	UtlString strAdditional ;
	requestContext.getCgiVariable("additional_parameters", strAdditional) ;
	addAdditionalEntries(strAdditional, *changeDb) ;

	//If anything in new Db, write to pinger-config and user-config
	setConfigValuesAdmin(changeDb) ;

	message.append("Configuration saved.") ;

	// Redirect user to configuration page.
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
    HTTP_MOVED_TEMPORARILY_CODE, HTTP_MOVED_TEMPORARILY_TEXT);

	UtlString location("/cgi/xpressaconfigadmin.cgi");
	location.append("?MESSAGE=");
	HttpMessage::escape(message) ;
	location.append(message.data()) ;
    response->setLocationField(location);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);
    delete changeDb;
}

void Webui::processLogin(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{

    UtlString userId("");
    UtlString password("") ;
    UtlString passwordConfirmed("");
	UtlString statusMessage("") ;
	UtlString userName("") ;

    requestContext.getCgiVariable("USER_ID", userId);
    requestContext.getCgiVariable("PASSWORD", password);
    requestContext.getCgiVariable("PASSWORD_CONFIRM", passwordConfirmed);
	requestContext.getCgiVariable("MESSAGE", statusMessage) ;
	requestContext.getEnvironmentVariable(HttpRequestContext::HTTP_ENV_USER, userName);

	UtlString loginHTML(HTML_HEADER_1) ;
	loginHTML.append("<title>sipXphone User Maintenance</title>") ;

    UtlBoolean  bIsSecure = FALSE;
    HttpMessage tmprequest = (HttpMessage)request;
    if (tmprequest.getSendProtocol() == OsSocket::SSL_SOCKET)
            bIsSecure = TRUE;
	insertHeader(loginHTML,bIsSecure) ;

	loginHTML.append("\
<!--table for body of page-->\n\
<table width=\"750\" border=\"0\" valign=\"top\" cellspacing=\"10\">\n\
\n\
	<tr>\n\
		<td><H1>User Maintenance</H1></td>\n\
	</tr>\n\
	<tr>\n\
     <td valign=\"top\" width=\"60%\">\n\
			<!--\"Add User\" table-->\n ") ;

	loginHTML.append("\
			<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
			  <tr>\n\
			  	<td colspan=\"2\"><p class=\"textOrange\">Add User:</p></td>\n\
			  </tr>\n\
	          <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
	 				<FORM ACTION=/cgi/login.cgi METHOD=POST>		\n\
				 	<p>User name:</p>\n\
				</td>\n\
				<td bgcolor=\"#FFCC33\" valign=\"top\">\n\
					<INPUT TYPE=TEXT NAME=USER_ID VALUE=\"\"></td>\n\
			  </tr>\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
				 	<p>Password (only a-z, A-Z, 0-9, *, # allowed):</p>\n\
				</td>\n\
				<td bgcolor=\"#FFCC33\" valign=\"top\">\n\
					<INPUT TYPE=PASSWORD NAME=PASSWORD></td>\n\
			  </tr>\n\
			  <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
	 				<p>Confirm Password:</p>\n\
				</td>\n\
				<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"center\">\n\
					<INPUT TYPE=PASSWORD NAME=PASSWORD_CONFIRM><br><br>\n\
	 				<INPUT TYPE=SUBMIT NAME=CHANGE_PASSWORD VALUE=\"Add User\"></FORM>\n\
			 	</td>\n\
               </tr>\n ") ;

	UtlString afterLoginHTML("\
              </table>\n\
			<!--End \"Add User\" table-->	 \n\
			<!--\"Change Password\" table-->\n\
			<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
			  <tr>\n\
			  	<td colspan=\"2\"><p class=\"textOrange\">Change Password:</p></td>\n\
			  </tr>\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
	 				<FORM ACTION=/cgi/setPassword.cgi METHOD=POST>\
				 	<p>User name:</p>\n\
				</td>\n\
				<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"center\">\n\
					<INPUT TYPE=TEXT NAME=USER_ID VALUE=\"\"></td>\n\
			  </tr>\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
				 	<p>Current Password:</p>\n\
				</td>\n\
				<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"center\">\n\
					<INPUT TYPE=PASSWORD NAME=OLD_PASSWORD></td>\n\
			  </tr>\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
				 	<p>New Password (only a-z, A-Z, 0-9, *, # allowed):</p>\n\
				</td>\n\
				<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"center\">\n\
					<INPUT TYPE=PASSWORD NAME=PASSWORD></td>\n\
			  </tr>			  \n\
			  <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
	 				<p>Confirm Password:</p>\n\
				</td>\n\
				<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"center\">\n\
					<INPUT TYPE=PASSWORD NAME=PASSWORD_CONFIRM><br><br>\n\
	 				<INPUT TYPE=SUBMIT NAME=CHANGE_PASSWORD VALUE=\"Change Password\"></FORM>\n\
			 	</td>			\n\
               </tr>\n ") ;

	UtlString deleteUserHTML("\
              </table>\n\
			  <!--End \"Change Password\" table-->\n\
			<!--\"Delete User\" table-->\n\
			<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
			  <tr>\n\
			  	<td colspan=\"2\"><p class=\"textOrange\">Remove User:</p></td>\n\
			  </tr>\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
	 				<FORM ACTION=/cgi/deleteUser.cgi METHOD=POST>\
				 	<p>User name:</p>\n\
				</td>\n\
				<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"center\">\n\
					<INPUT TYPE=TEXT NAME=USER_ID VALUE=\"\"><br><br>\n\
					<INPUT TYPE=SUBMIT NAME=CHANGE_PASSWORD VALUE=\"Delete User\"></FORM>\n\
					</td>\n\
			  </tr>\n ") ;

		UtlString afterPasswordHTML("\
              </table>\n\
			  <!--End \"Delete User\" table-->\n\
	 </td>\n\
     <td width=\"40%\" valign=\"top\">\n \
	 		<!--Instructions column-->\n\
            <p class=\"textOrange\">Instructions:</p>\n\
			<p>On this page you can set up user names and passwords to protect these web pages from unauthorized access.</p> \n\
			<ul class=\"normalText\">\n\
			<li>If you set up a user name and password, that user must supply them every time to access these maintenance pages.</li>\n\
			<li>If you do not set up user names and passwords, only users who know the administrative user name and password can access these pages and change sipXphone's configuration.</li>\n\
			</ul>\n\
			<p>To set up a user name and password, enter values in the fields on the left.  Enter the password a second time to confirm, then press <b>Add User</b>.</p> \n\
			<p>Individuals can change this assigned password later by choosing Administration->Change Password.</p>\n\
			<p>To change the administrative password, enter name \"admin\" and the current password and then the new password. After you enter the new password a second time to confirm, press <b>Change Password</b>.</p> \n\
   </td>\n\
	</tr>\n\
</table>\n ") ;


		afterPasswordHTML.append(HTML_FOOTER) ;



	// Check user is "admin," if not redirect to not authorized page
	if (userName.compareTo("admin")!=0) {
		const char * text = "<HTML><BODY>User Not Authorized<br><br>Must be user \"admin\" to access this page.</BODY></HTML>\n" ;
		HttpServer::processUserNotAuthorized(requestContext,request,response, text) ;
	}
	else {

	// Build Response HTML

	UtlString html(loginHTML) ;

	// Set the username/password

		UtlString errorHTML ;


		if (validateChangePassword(userId, password, passwordConfirmed, errorHTML) !=0)
			html.append(errorHTML.data()) ;
		else {
			// Add user
			UtlString messageHTML ;
			Pinger* pinger = Pinger::getPingerTask();

			if (pinger->addUser(userId, password) == 0) {
				messageHTML.append("<tr>\n<td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\">Status:</td><td bgcolor=\"#FFCC33\" valign=\"top\">") ;
				messageHTML.append("<p>User and password enabled for: ") ;
				messageHTML.append(userId.data());
				messageHTML.append(".</p></TD>\n  </TR>\n") ;
			} else {
				messageHTML.append("<tr>\n<td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\">Status:</td><td bgcolor=\"#FFCC33\" valign=\"top\">") ;
				messageHTML.append("<p>Unable to save user name/password</p>") ;
				messageHTML.append("</TD>\n  </TR>\n") ;
			}							
			
			html.append(messageHTML) ;
		}
			
		html.append(afterLoginHTML) ;

		UtlString deleteMessageHTML ;


		if (statusMessage.contains("delete")) {
			deleteMessageHTML.append("<tr>\n<td width=\"300\"bgcolor=\"#FFCC33\" valign=\"top\"><p>Status</p></td><td bgcolor=\"#FFCC33\" valign=\"top\"><p>") ;
			deleteMessageHTML.append(statusMessage) ;
			deleteMessageHTML.append("</p></TD>\n  </TR>\n") ;
		} else if (statusMessage.compareTo("") != 0 ) {
			UtlString passwordMessageHTML ;
			passwordMessageHTML.append("<tr>\n<td width=\"300\"bgcolor=\"#FFCC33\" valign=\"top\"><p>Status</p></td><td bgcolor=\"#FFCC33\" valign=\"top\"><p>") ;
			passwordMessageHTML.append(statusMessage) ;
			passwordMessageHTML.append("</p></TD>\n  </TR>\n") ;
			html.append(passwordMessageHTML) ;
		}

		html.append(deleteUserHTML) ;

		if (deleteMessageHTML.compareTo("")!=0) {
			html.append(deleteMessageHTML) ;
		}

		html.append(afterPasswordHTML) ;

		// Build the response message
		HttpBody* body = new HttpBody(html.data(),
			html.length(), CONTENT_TYPE_TEXT_HTML);
		response = new HttpMessage();
		response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
			HTTP_OK_CODE, HTTP_OK_TEXT);
		response->setBody(body);
		response->setContentType(CONTENT_TYPE_TEXT_HTML);
	}

}

// 
// CGI which allows users to change their password
// Displays HTML form, then processes form, upadating user-config with new password
//
void Webui::processChangePassword(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
    UtlString password("") ;
    UtlString passwordConfirmed("");
	UtlString statusMessage("") ;
	UtlString userName("") ;
	UtlString changePassword("") ;
    requestContext.getCgiVariable("PASSWORD", password);
    requestContext.getCgiVariable("PASSWORD_CONFIRM", passwordConfirmed);
	requestContext.getCgiVariable("MESSAGE", statusMessage) ;
	requestContext.getCgiVariable("CHANGED", changePassword) ;
	requestContext.getEnvironmentVariable(HttpRequestContext::HTTP_ENV_USER, userName);

	UtlString loginHTML(HTML_HEADER_1) ;
	loginHTML.append("<title>sipXphone Change Password</title>") ;

    UtlBoolean  bIsSecure = FALSE;
    HttpMessage tmprequest = (HttpMessage)request;
    if (tmprequest.getSendProtocol() == OsSocket::SSL_SOCKET)
            bIsSecure = TRUE;
	insertHeader(loginHTML,bIsSecure) ;

	loginHTML.append("\
<!--table for body of page-->\n\
<table width=\"750\" border=\"0\" valign=\"top\" cellspacing=\"10\">\n\
\n\
	<tr>\n\
		<td><H1>Change Password</H1></td>\n\
	</tr>\n\
	<tr>\n\
     <td valign=\"top\" width=\"60%\">\n\
			<!--\"Add User\" table-->\n ") ;

	loginHTML.append("\
			<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
			  <tr>\n\
			  	<td colspan=\"2\"><p class=\"textOrange\">Change Password:</p></td>\n\
			  </tr>\n\
	          <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
	 				<FORM ACTION=/cgi/password.cgi METHOD=POST>		\n\
				 	<p>User name:</p>\n\
				</td>\n\
				<td bgcolor=\"#FFCC33\" valign=\"top\"><p><b> ") ;

	UtlString afterUsernameHTML("\
					</b><p></td>\n\
			  </tr>\n\
              <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
				 	<p>Password (only a-z, A-Z, 0-9, *, # allowed):</p>\n\
				</td>\n\
				<td bgcolor=\"#FFCC33\" valign=\"top\">\n\
					<INPUT TYPE=PASSWORD NAME=PASSWORD></td>\n\
			  </tr>\n\
			  <tr>\n\
                <td width=\"600\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
	 				<p>Confirm Password:</p>\n\
				</td>\n\
				<td bgcolor=\"#FFCC33\" valign=\"top\">\n\
					<INPUT TYPE=PASSWORD NAME=PASSWORD_CONFIRM><br><br>\n\
					<INPUT TYPE=HIDDEN NAME=CHANGED VALUE=TRUE>\n\
	 				<INPUT TYPE=SUBMIT NAME=CHANGE_PASSWORD VALUE=\"Change Password\"></FORM>\n\
			 	</td>\n\
               </tr>\n ") ;

	UtlString afterLoginHTML("\
              </table>\n\
			<!--End \"Add User\" table-->	 \n\
	 </td>\n\
     <td width=\"40%\" valign=\"top\">\n\
	 		<!--Instructions column-->\n\
            <p class=\"textOrange\">Instructions:</p>\n\
			<p>On this page you can change your current password.</p> \n\
			<p>To change your password, enter the new password. After you enter the new password a second time to confirm, press <b>Change Password</b>.</p> \n\
			<p>If you need more help, check the SIPFoundry website at <a href=\"http://www.sipfoundry.org\">www.sipfoundry.org</a>.</p> \n\
     </td>\n\
	</tr>\n\
</table>\n ") ;

		afterLoginHTML.append(HTML_FOOTER) ;


	// Build Response HTML

	UtlString html(loginHTML) ;

	// Add current username
	html.append(userName.data()) ; 
	html.append(afterUsernameHTML) ;

	// Set the username/password

	UtlString errorHTML ;

	if (!changePassword.isNull()) {
		if (validateChangePassword(userName, password, passwordConfirmed, errorHTML) !=0)
			html.append(errorHTML.data()) ;
		else {
			UtlString messageHTML ;

			if (changeUserPassword(userName, password) ==0) {
				messageHTML.append("<tr>\n<td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\"><P>Status</p></td><td bgcolor=\"#FFCC33\" valign=\"top\">") ;
				messageHTML.append("<p>User and password enabled for: ") ;
				messageHTML.append(userName.data());
				messageHTML.append(".</p></TD>\n  </TR>\n") ;

            } else {
				messageHTML.append("<tr>\n<td width=\"300\" colspan=\"2\" bgcolor=\"#FFCC33\" valign=\"top\"><p>Status</p></td><td bgcolor=\"#FFCC33\" valign=\"top\">") ;
				messageHTML.append("<p>Unable to save user name/password</p>") ;
				messageHTML.append(".</p></TD>\n  </TR>\n") ;
            }

			html.append(messageHTML) ;
        }

	}
		
	html.append(afterLoginHTML) ;





    // Build the response message
    HttpBody* body = new HttpBody(html.data(),
        html.length(), CONTENT_TYPE_TEXT_HTML);
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_OK_CODE, HTTP_OK_TEXT);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);

}

void Webui::processSetPassword(const HttpRequestContext& requestContext,
                                  const HttpMessage& request,
                                  HttpMessage*& response)
{
    UtlString html;
    UtlString userId;
    UtlString password;
    UtlString passwordConfirmed;
    requestContext.getCgiVariable("USER_ID", userId);
    requestContext.getCgiVariable("PASSWORD", password);
    requestContext.getCgiVariable("PASSWORD_CONFIRM", passwordConfirmed);
	UtlString message ;
	UtlString errorHTML;

	if (validateChangePassword(userId, password, passwordConfirmed, errorHTML) !=0) {
			message.append("Could not save password") ;
	} else {
	    // Set the password
		if (changeUserPassword(userId, password) == 0) {
            message.append("Password changed for: ");
            message.append(userId.data());
        } else {
            message.append("<BR><B>Unable to save password in config. file.");
        }

        }

	// Redirect user to configuration page.

    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
    HTTP_MOVED_TEMPORARILY_CODE, HTTP_MOVED_TEMPORARILY_TEXT);

	UtlString location("/cgi/login.cgi");
	location.append("?MESSAGE=");
	HttpMessage::escape(message) ;
	location.append(message.data()) ;
    response->setLocationField(location);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);

}

void Webui::processDeleteUser(const HttpRequestContext& requestContext,
                                  const HttpMessage& request,
                                  HttpMessage*& response)
{
    UtlString html;
    UtlString userId;
    UtlString password;
    UtlString passwordConfirmed;
    requestContext.getCgiVariable("USER_ID", userId);
	UtlString message ;
	UtlString errorHTML;

	if (userId.compareTo("admin")!=0) {
		if (deleteUser(userId, password) == 0) {
				message.append(userId.data());
				message.append(" deleted.\n") ;
		} else {
			message.append("Unable to delete user.\n");
		}
	} else {
		message.append("Cannot delete admin user.\n") ;
	}

	// Redirect user to configuration page.

    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
    HTTP_MOVED_TEMPORARILY_CODE, HTTP_MOVED_TEMPORARILY_TEXT);

	UtlString location("/cgi/login.cgi");
	location.append("?MESSAGE=");
	HttpMessage::escape(message) ;
	location.append(message.data()) ;
    response->setLocationField(location);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);

}


void Webui::processConfig(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{

	UtlString userName("") ;
	requestContext.getEnvironmentVariable(HttpRequestContext::HTTP_ENV_USER, userName);

	UtlString configHTML(HTML_HEADER_1) ;
	configHTML.append("<title>sipXphone Configuration</title>") ;
	configHTML.append("\
<script language=\"Javascript\">\n\
function promptOnRestart() {\n\
var msg=\"You have selected to restart the phone.\\n\\n\
Restarting sipXphone will interrupt your ability to send and receive calls for up to 2 minutes.\\n\\n\
Are you sure you want to continue?\";\n\
if (confirm(msg))\n\
	location.replace(\"/cgi/restart.cgi\") ;\n\
}\n\
</script>\n ") ;

    UtlBoolean  bIsSecure = FALSE;
    HttpMessage tmprequest = (HttpMessage)request;
    if (tmprequest.getSendProtocol() == OsSocket::SSL_SOCKET)
            bIsSecure = TRUE;
	insertHeader(configHTML,bIsSecure) ;

	configHTML.append("\
<!--table for body of page-->\n\
<table width=\"750\" border=\"0\" valign=\"top\" cellspacing=\"10\">\n\
\n\
\n\
	<tr>\n\
		<td><H1>File Uploads</H1></td>\n\
	</tr>\n\
	<tr>\n\
    <td valign=\"top\" width=\"60%\">\n\
\n\
\n\
			<!--\"Basic Configuration\" table-->\n\
			<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
			  <tr>\n\
			  	<td colspan=\"2\"><p class=\"textOrange\">Phone configuration:</p></td>\n\
			  </tr>\n\
              <tr>\n\
                <td width=\"30%\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <h2>Phone configuration</h2>\n\
				  <p class=\"smallText\"><p class=\"smallText\"><a href=\"/pinger-config\">(View current configuration)</a> </p>\n\
                  </td>\n\
                 <td width=\"70%\" bgcolor=\"#FFCC33\" align=\"left\">\n\
                  <p>Enter the path to the <b>pinger-config</b> file on your computer.\n\
                  <form method=\"post\" action=\"/cgi/putFile.cgi\" ENCTYPE=\"multipart/form-data\">\n ") ;

                      
	UtlString afterPingerConfig("\
					</form>\n\
                      <form><input type=\"button\" name=\"RESTART\" value=\"Restart\" onClick=\"promptOnRestart();\"></form>\n\
                  </td>\n\
               </tr>\n\
              <tr>\n\
                <td width=\"30%\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <h2>User configuration</h2>\n\
				  <p class=\"smallText\"><a href=\"/user-config\">(View current configuration)</a> </p>\n\
                 </td>\n\
                 <td width=\"70%\" bgcolor=\"#FFCC33\">\n\
                  <p>Enter the path to the <b>user-config</b> file on your computer.\n\
                  <form method=\"post\" action=\"/cgi/putFile.cgi\" ENCTYPE=\"multipart/form-data\">\n ") ;
				  
	UtlString afterUserConfig("\
					</form>\n\
                      <form><input type=\"button\" name=\"RESTART\" value=\"Restart\" onClick=\"promptOnRestart();\"></form>\n\
                  </td>\n\
               </tr>\n\
              <tr>\n\
                <td width=\"30%\" bgcolor=\"#FFCC33\" valign=\"top\">\n\
                  <h2>Application configuration</h2>\n\
				  <p class=\"smallText\"><a href=\"/app-config\">(View current configuration)</a> </p>\n\
                 </td>\n\
                 <td width=\"70%\" bgcolor=\"#FFCC33\">\n\
                  <p>Enter the path to the <b>app-config</b> file on your computer.\n\
                  <form method=\"post\" action=\"/cgi/putFile.cgi\" ENCTYPE=\"multipart/form-data\">\n ") ;				  
				  
	UtlString afterAppConfig("\
					</form>\n\
                      <form><input type=\"button\" name=\"RESTART\" value=\"Restart\" onClick=\"promptOnRestart();\"></form>\n\
                 </td>\n\
               </tr>\n\
              </table>\n\
			  <!--End \"Basic Configuration\" table-->\n\
			<!-- Audio table-->\n\
            <table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
				<tr>\n\
				  <td><p class=\"textOrange\">Audio:</p></td>\n\
				</tr>\n\
                <tr>\n\
                  <td width=\"30%\" bgcolor=\"#FFCC33\" valign=\"top\"><h2>Ring sound</h2></td>\n\
                  <td width=\"70%\" bgcolor=\"#FFCC33\">\n\
                  <p>The audio file must be:\n\
				  <ul type=\"compact\" class=\"normalText\">\n\
				  <ul type=\"compact\" class=\"normalText\">\n\
				  <li>.WAV (RIFF-WAV) format.</li>\n\
				  <li>16-bit signed PCM and in little-endian byte order.</li>\n\
				  <li>All formats must be mono.</li>\n\
				  <li>All formats must use a sampling rate of 8000 samples/second.</li>\n\
				  <li>The maximum file size is 500KB.</li></ul></p>\n\
                  <p>Enter the path and file name of an audio file to use as the <b>ring sound</b> for your incoming calls:</p> \n\
				  <form method=\"post\" action=\"/cgi/putFile.cgi\" ENCTYPE=\"multipart/form-data\">\n ") ;
	
	UtlString afterRingSound("\
					</form>\n\
                  </td>\n\
                </tr>\n\
				<tr>\n\
                  <td width=\"30%\" bgcolor=\"#FFCC33\" valign=\"top\"><h2>Startup Splash</h2></td>\n\
                  <td width=\"70%\" bgcolor=\"#FFCC33\">\n\
                  <p>Enter the path and file name of an audio file to use as the <b>start up splash </b>for your phone:</p> \n\
				  <form method=\"post\" action=\"/cgi/putFile.cgi\" ENCTYPE=\"multipart/form-data\">\n ") ;

UtlString afterStartUpSplash1("\
					</form>\n\
                  </td>\n\
                </tr>\n\
              </table>\n\
			  <!-- End audio table-->\n") ;

UtlString afterFont2 ("\
						<!-- Restart-->\n\
            <table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
				<tr>\n\
				  <td><p class=\"textOrange\">Restart:</p></td>\n\
				</tr>\n\
                <tr>\n\
                  <td width=\"30%\" bgcolor=\"#FFCC33\" valign=\"top\"><h2>Restart your phone:</h2></td>\n\
                  <td width=\"70%\" bgcolor=\"#FFCC33\">\n\
                  <p>Restarting your phone will interrupt your ability to send and receive calls for up to 2 minutes.</p> \n\
                      <form><input type=\"button\" name=\"RESTART\" value=\"Restart\" onClick=\"promptOnRestart();\"></form>\n\
                  </td>\n\
                </tr>\n\
              </table>\n\
			<!-- End restart table-->\n\
	</td>\n\
     <td width=\"40%\" valign=\"top\">\n\
            <p class=\"textOrange\">Instructions:</p>\n\
			<p>On this page you can: \n\
			<ul type=\"compact\" class=\"normalText\">\n\
			<li>upload ASCII configuration files</li>\n\
			<li>install new audio files to play different startup and ringtone sounds </li>\n"
			"<li>restart sipXphone</li></ul>\n\
			<p>To perform your task, scroll to the correct section of this page. <br><br>\n\
			To indicate a file for installation, browse the file system of your local PC or enter the file's full pathname (for example, c:\\sipfoundry\\new\\sipxphone-os-1.xos). Then click Upload.<br> \n\
			After you upload any new file, click restart so that the changes will take effect.<br>\n\
			If you need more help, check the SIPFoundry website at <a href=\"http://www.sipfoundry.org\">www.sipfoundry.org</a>.\n\
		</td>\n\
		</tr>\n\
</table>\n ") ;

	afterFont2.append(HTML_FOOTER) ;


	// Build HTML Response

	UtlString html(configHTML) ;

	// Check user is "admin," if not redirect to not authorized page
	if (userName.compareTo("admin")!=0) {
		osPrintf("Webui::getPingerConfigAdmin - user is not admin\n") ;
		const char * text = "<HTML><BODY>User Not Authorized<br><br>Must be user \"admin\" to access this page.</BODY></HTML>\n" ;
		HttpServer::processUserNotAuthorized(requestContext,request,response, text) ;
	} else {

	// Have different paths for VXW and NT
	PingerInfo pingerInfo ;
	UtlString path("") ;
	pingerInfo.getFlashFileSystemLocation(path) ;

	html.append("<input type=\"file\" NAME=\"") ;
	html.append(path.data()) ;
	html.append("/pinger-config\" VALUE=\"pinger-config\" size=\"38\">\n") ;
	html.append("<input type=\"submit\" name=\"PUT_CONFIG\" value=\"Upload pinger-config\">\n") ;

	html.append(afterPingerConfig) ;

	html.append("<input type=\"file\" NAME=\"") ;
	html.append(path.data()) ;
	html.append("/user-config\" VALUE=\"user-config\" size=\"38\">\n") ;
	html.append("<input type=\"submit\" name=\"PUT_CONFIG\" value=\"Upload user-config\">\n") ;

	html.append(afterUserConfig) ;

	html.append("<input type=\"file\" NAME=\"") ;
	html.append(path.data()) ;
	html.append("/app-config\" VALUE=\"user-config\" size=\"38\">\n") ;
	html.append("<input type=\"submit\" name=\"PUT_CONFIG\" value=\"Upload app-config\">\n") ;

	html.append(afterAppConfig) ;

	html.append("<input type=\"file\" name=\"") ;
	html.append(path.data()) ;
	html.append("/ringTone.wav\" size=\"38\">\n") ;
	html.append("<input type=\"submit\" name=\"PUT_CONFIG\" value=\"Upload Ringtone\">\n") ;

	html.append(afterRingSound) ;

	html.append("<input type=\"file\" name=\"") ;
	html.append(path.data()) ;
	html.append("/audioSplash.wav\" size=\"38\">\n") ;
	html.append("<input type=\"submit\" name=\"PUT_CONFIG\" value=\"Upload Startup Splash\">\n") ;

	html.append(afterStartUpSplash1) ;

    html.append(afterFont2) ;

    // Build the response message
    HttpBody* body = new HttpBody(html.data(),
        html.length(), CONTENT_TYPE_TEXT_HTML);
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_OK_CODE, HTTP_OK_TEXT);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);

	}

}

void Webui::processRestart(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{

	UtlString userName("") ;
	requestContext.getEnvironmentVariable(HttpRequestContext::HTTP_ENV_USER, userName);

	// Check user is "admin," if not redirect to not authorized page
	if (userName.compareTo("admin")!=0) {
		osPrintf("Pinger::getPingerConfigAdmin - user is not admin\n") ;
		const char * text = "<HTML><BODY>User Not Authorized<br><br>Must be user \"admin\" to access this page.</BODY></HTML>\n" ;
		HttpServer::processUserNotAuthorized(requestContext,request,response, text) ;
	} else {

		// Build the resp monse message
		UtlString messageBody("<HTML><HEAD><META HTTP-EQUIV = \"PRAGMA\" CONTENT = \"no-cache\"></HEAD><BODY><p>Requesting restart...</p></BODY></HTML>\n") ;
		HttpBody* body = new HttpBody(messageBody.data(),
		messageBody.length(), CONTENT_TYPE_TEXT_HTML);
		response = new HttpMessage();
		response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
			HTTP_OK_CODE, HTTP_OK_TEXT);
		response->setBody(body);
		response->setContentType(CONTENT_TYPE_TEXT_HTML);
		response->setContentLength(messageBody.length());

      Pinger::getPingerTask()->restart(TRUE, 0, "administrator initiated") ;
	}

}

void Webui::processConfigEnroll(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
	UtlString serverUrlString;
    requestContext.getCgiVariable("SERVER", serverUrlString);

    Pinger* pinger = Pinger::getPingerTask();
    SipConfigDeviceAgent* configAgent = NULL;
    if(pinger)
    {
        configAgent = pinger->getConfigAgent();
    }

    UtlString statusMessage;

    if(configAgent && !serverUrlString.isNull())
    {
        Url serverUrl(serverUrlString);
        int port;
        UtlString host;
        serverUrl.getHostAddress(host);
        port = serverUrl.getHostPort();

        statusMessage = "Server: ";
        statusMessage += serverUrlString;

        // If the host exists
        if(configAgent->setServer(host.data(), port))
        {
            // enroll (SUBSCRIBE) to be managed by it
            configAgent->enroll();

            statusMessage += " found. Enrolling.\n";
        }
        else
        {
            statusMessage += " not found\n";
        }
    }

    // No server was provided assume we are to enroll now
    else if(configAgent)
    {
        configAgent->enroll();
        statusMessage = "Enrolling to previously discovered server.\n";
    }

    HttpBody* body = new HttpBody(statusMessage.data(),
		statusMessage.length(), CONTENT_TYPE_TEXT_HTML);
	response = new HttpMessage();
	response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
		HTTP_OK_CODE, HTTP_OK_TEXT);
	response->setBody(body);
	response->setContentType(CONTENT_TYPE_TEXT_HTML);
	response->setContentLength(statusMessage.length());
}


void Webui::processFonts(const HttpRequestContext& requestContext,
                         const HttpMessage& request,
                         HttpMessage*& response)
{
    static char* pFontsData = NULL ;
    static int   iFontsData = 0 ;

    // Lazy Initialization: Obtain the resource when first needed
    if (pFontsData == NULL)
    {
        JXAPI_GetXpressaResource("fonts.css", &pFontsData, iFontsData) ;
    }
        
    // Serve up the document
    if (pFontsData != NULL)
    {
        // If we have the resource then send it
        HttpBody* body = new HttpBody(pFontsData,
            iFontsData, "text/css");
        response = new HttpMessage();
        response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
            HTTP_OK_CODE, HTTP_OK_TEXT);
        response->setBody(body);
        response->setContentType("text/css");
    }
    else
    {
        // Otherwise return an error to the browser
        response = new HttpMessage();
        response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
            HTTP_FILE_NOT_FOUND_CODE, HTTP_FILE_NOT_FOUND_TEXT);
    }
}

void Webui::processLogo(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
    static char* pLogoData = NULL ;
    static int   iLogoData = 0 ;

    // Lazy Initialization: Obtain the resource when first needed
    if (pLogoData == NULL)
    {
        JXAPI_GetXpressaResource("sipfoundry-sipxphone-logo.gif", &pLogoData, iLogoData) ;
    }
        
    // Serve up the document
    if (pLogoData != NULL)
    {
        // If we have the resource then send it
        HttpBody* body = new HttpBody(pLogoData,
            iLogoData, "image/gif");
        response = new HttpMessage();
        response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
            HTTP_OK_CODE, HTTP_OK_TEXT);
        response->setBody(body);
        response->setContentType("image/gif");
    }
    else
    {
        // Otherwise return an error to the browser
        response = new HttpMessage();
        response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
            HTTP_FILE_NOT_FOUND_CODE, HTTP_FILE_NOT_FOUND_TEXT);
    }
}


void Webui::processDhtmlMenu(const HttpRequestContext& requestContext,
                             const HttpMessage& request,
                             HttpMessage*& response)
{
    static char* pMenuData = NULL ;
    static int   iMenuData = 0 ;

    // Lazy Initialization: Obtain the resource when first needed
    if (pMenuData == NULL)
    {
        JXAPI_GetXpressaResource("dhtmlMenu.js", &pMenuData, iMenuData) ;
    }
        
    // Serve up the document
    if (pMenuData != NULL)
    {
        // If we have the resource then send it
        HttpBody* body = new HttpBody(pMenuData,
            iMenuData, "application/octet-stream");
        response = new HttpMessage();
        response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
            HTTP_OK_CODE, HTTP_OK_TEXT);
        response->setBody(body);
        response->setContentType("application/octet-stream");
    }
    else
    {
        // Otherwise return an error to the browser
        response = new HttpMessage();
        response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
            HTTP_FILE_NOT_FOUND_CODE, HTTP_FILE_NOT_FOUND_TEXT);
    }
}



void Webui::processSpeeddialJar(const HttpRequestContext& requestContext,
                                const HttpMessage& request,
                                HttpMessage*& response)
{
    static char* pJarData = NULL ;
    static int   iJarData = 0 ;

    // Lazy Initialization: Obtain the resource when first needed
    if (pJarData == NULL)
    {
        JXAPI_GetXpressaResource("SpeedDialApplet.jar", &pJarData, iJarData) ;
    }
        
    // Serve up the document
    if (pJarData != NULL)
    {
        // If we have the resource then send it
        HttpBody* body = new HttpBody(pJarData,
            iJarData, "application/octet-stream");
        response = new HttpMessage();
        response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
            HTTP_OK_CODE, HTTP_OK_TEXT);
        response->setBody(body);
        response->setContentType("application/octet-stream");
    }
    else
    {
        // Otherwise return an error to the browser
        response = new HttpMessage();
        response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
            HTTP_FILE_NOT_FOUND_CODE, HTTP_FILE_NOT_FOUND_TEXT);
    }
}



void Webui::processBackgroundImage(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{

	
	unsigned char rawData[43] = {
		0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x01, 0x00, 0x01, 0x00, 0x80, 0xFF, 0x00, 0xC0, 0xC0, 0xC0, 
		0x00, 0x00, 0x00, 0x21, 0xF9, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 
		0x01, 0x00, 0x01, 0x00, 0x00, 0x02, 0x02, 0x44, 0x01, 0x00, 0x3B, 
    } ;

  	// Create Response
    HttpBody* body = new HttpBody((const char *) rawData,
        43, "image/gif");
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_OK_CODE, HTTP_OK_TEXT);
    response->setBody(body);
    response->setContentType("image/gif");	

}

//
// Get values for configuration parameters
// Puts them in a Javascript object where they can be called from a web page
//
void Webui::createJsConfigAdmin(UtlString& html, UtlString strParamName) 
{
	// Get configuration information from config files
	OsConfigDb configDb ;
	configDb.loadFromFile(CONFIGDB_NAME_IN_FLASH) ;
	configDb.loadFromFile(CONFIGDB_USER_IN_FLASH) ;

	OsConfigDb paramValuesHash ;		//database of entries in config files that match 
	UtlString strNext("") ;		//next one in list
	UtlString strKey("") ;		//key returned from db lookup
	UtlString strValue("") ;		//value returned from db lookup
	UtlString strBuffer("") ;	//overall buffer which stores pingerconfig content
	
	if(strParamName != "")
	{
		// Get all matching params for current parameter prefix
		configDb.getSubHash(strParamName, paramValuesHash) ;
		while(paramValuesHash.getNext(strNext, strKey, strValue)==OS_SUCCESS) {
			// Add key/value pair to buffer
			if (strValue.length() != 0) {
				strBuffer.append(strKey.strip(UtlString::leading, '.')) ; //i.e. get next Key suffix (+1 is for period after main name of key)
				strBuffer.append(" : ") ;
				strBuffer.append(strValue) ;
				strBuffer.append("\n") ;
			}
			// Get next one in list
			strNext.remove(0) ;
			strNext.append(strKey) ;
		}
		html.append(strBuffer) ;
	}
	else
	{
		
		getAdditionalValues(paramValuesHash) ;

		// Add to the buffer
		while (paramValuesHash.getNext(strNext, strKey, strValue)==OS_SUCCESS) {
			// Add key/value pair to buffer
			if (strKey.length() != 0) {
				strBuffer.append(strKey) ;
				strBuffer.append(" : ") ;
				strBuffer.append(strValue) ;
				strBuffer.append("\n") ;
			}
			
			// Get next one in list
			strNext.remove(0) ;
			strNext.append(strKey) ;
		}

		html.append(strBuffer) ;

	}
}


	


// Sets relevant values in pinger-config and user-config files
// If specified in pingerConfig, writes out parameters to pinger-config
// Otherwise writes to user-config
// Writes out subsqent files to /flash0 or env
// This routine is called from a Web UI page in which user specifies config values
void Webui::setConfigValuesAdmin(OsConfigDb* dbPage) 
{
 	OsConfigDb dbOldConfig ;
 	OsConfigDb dbNewPingerConfig ;
 	OsConfigDb dbNewUserConfig ;
  UtlBoolean  isManagedByWebUI ;
 	UtlString strKey ;			
	UtlString strNewKey ;
	UtlString strValue ;

	int i ;
	int iUpdate = 0 ;				// Should we reinitialize parameters?

  /*
	 * Have to copy over parameters from old DB
	 * Note: only copy over "standard parameters" i.e. not "additional" ones.
	 * This is because don't want to copy over parameters that user has 
	 * deleted from the "additional" parameters section.
	 */

  /*
   * The default config file has some parameters in the device profile
   * that should be in the user profile.  To avoid losing configuration
   * information we load up both the device and user config information
   * before we determine what parameters need to be copied to the new
   * device and user profiles.
   */
	dbOldConfig.loadFromFile(CONFIGDB_NAME_IN_FLASH) ;
	dbOldConfig.loadFromFile(CONFIGDB_USER_IN_FLASH) ;

	// Copy old parameters to new pinger-config and user-config
	for (strKey = OsUtil::NULL_OS_STRING;
       dbOldConfig.getNext(strKey, strNewKey, strValue) == OS_SUCCESS;
       strKey = strNewKey) {

    // if the parameter is one of the multi-value parameters settable
    // from the Administration->Phone Configuration page, don't copy it
    if (isAdminMultiValueConfig(strNewKey))
      continue;

    // determine if the parameter is managed by the web UI
    isManagedByWebUI = FALSE;
    for (i = 0; allConfig[i] != 0; i++) {
      if (strNewKey.index(allConfig[i]) == 0) {
        isManagedByWebUI = TRUE;
        break;
      }
    }

    // we only copy parameters managed by the Web UI -- the other
    // parameters are the ones that show up in the "Additional
    // Parameters" section of the web page.
    if (isManagedByWebUI) {
      if (isContainedInPingerConfig(strNewKey)) {
        dbNewPingerConfig.set(strNewKey, strValue) ;
      } else {
        dbNewUserConfig.set(strNewKey, strValue) ;
      }
    }
	}

  // Update with new parameters
	for (strKey = OsUtil::NULL_OS_STRING;
       dbPage->getNext(strKey, strNewKey, strValue) == OS_SUCCESS;
       strKey = strNewKey) {
		if (isContainedInPingerConfig(strNewKey)) {
			dbNewPingerConfig.set(strNewKey, strValue) ;
		} else {
			dbNewUserConfig.set(strNewKey, strValue) ;
		}

		// Should we reinitialize any parameters?
		// For example, reset telnet access password
		if (strNewKey.compareTo(CONFIG_PHONESET_TELNET_ACCESS)==0) {
			// Set flag - will have to reinitialize
			iUpdate = 1 ;
		}
	}

	dbNewPingerConfig.storeToFile(CONFIGDB_NAME_IN_FLASH) ;
	dbNewUserConfig.storeToFile(CONFIGDB_USER_IN_FLASH) ;
}

// Sets relevant values in pinger-config and user-config files
// If specified in pingerConfig, writes out parameters to pinger-config
// Otherwise writes to user-config
// Writes out subsqent files to /flash0 or env
// This routine is called from a Web UI page in which user specifies config values
void Webui::setConfigValuesUser(OsConfigDb* dbPage) 
{
 	OsConfigDb dbNewPingerConfig ;
 	OsConfigDb dbNewUserConfig ;
 	UtlString strKey ;			
	UtlString strNewKey ;
	UtlString strValue ;

	// Write out pinger-config
	strKey.remove(0) ;
	strNewKey.remove(0) ;
	strValue.remove(0) ;

	dbNewPingerConfig.loadFromFile(CONFIGDB_NAME_IN_FLASH) ;
	dbNewUserConfig.loadFromFile(CONFIGDB_USER_IN_FLASH) ;

	// Update with new parameters
	while (dbPage->getNext(strKey, strNewKey, strValue) == OS_SUCCESS) {
		if (isContainedInPingerConfig(strNewKey)) {
			dbNewPingerConfig.set(strNewKey, strValue) ;
		} else {
			dbNewUserConfig.set(strNewKey, strValue) ;
		}
		strKey = strNewKey ;
	}

	dbNewPingerConfig.storeToFile(CONFIGDB_NAME_IN_FLASH) ;
	dbNewUserConfig.storeToFile(CONFIGDB_USER_IN_FLASH) ;
}

//
// Routine to save web UI textarea fields to config Db. ie allows parameters that span multiple lines
// to be saved to DB. Called from setPingerConfigAdmin 
//
void Webui::setMultipleValues(const char* paramKey, UtlString paramValue, OsConfigDb& dbMultiple) 
{

#ifdef TEST_PRINT
	osPrintf("Webui::setMultipleValues called paramKey = %s, paramValue = %s.\n", paramKey, paramValue.data()) ;
#endif
	
	int j = 0 ;					// No. lines of text
	int k = 0 ;					// No. chars in line
	char *textAreaValue = new char[paramValue.length()+1] ;	// paramValue as char*
	char buf[256] ;				// each line of text put here
	sprintf(textAreaValue, "%s", paramValue.data());
	int newline = 0 ;
	UtlString strKey	;			// newly created key
	UtlString strValue ;			// newly created value
	int iKey = 0 ;				// Whether we have a key
	int iValue = 0 ;			// Whether we've started to read value
	int iWriteBuffer = 0 ;		// Instruction to write to buffer

#ifdef TEST_PRINT
	osPrintf("Webui::setMultipleValues textAreaValue = %s\n", textAreaValue) ;
#endif
	
	while (textAreaValue[j] !=0) {

		if (textAreaValue[j] == ' ' && iValue == 1) {
			iWriteBuffer = 1 ;
		} else if (textAreaValue[j] == ' ' || textAreaValue[j] == '\r') {
			//swallow spaces
		} else if (textAreaValue[j] == ':') {
			//swallow first colon
		
			if (iKey == 0 ) {
				//Create key
				buf[k++] = '\0';
				strKey.append(paramKey) ;
				strKey.append('.') ;
				strKey.append(buf) ;
				k = 0 ;
				iKey = 1 ;
			} else {
				iWriteBuffer = 1 ;
			}
		} else if (textAreaValue[j] == '\n' || textAreaValue[j] == '\0') {
			//Create value
			newline = 1 ;
			buf[k++] = '\0' ;
			strValue.append(buf) ;
			k=0 ;
		} else if (iKey == 1) {
			// already have key - this must be a value
			iValue = 1 ;
			iWriteBuffer = 1 ;
		} else {
			iWriteBuffer = 1 ;
		}

		if (iWriteBuffer == 1) {
			//Add char to buffer
			buf[k] =  textAreaValue[j] ;
			newline = 0 ;
			k++ ;
			iWriteBuffer = 0 ;
		}

		if (newline) {
			// Write to Db
#ifdef TEST_PRINT
			osPrintf("Webui::setMultipleValues after newline paramKey: %s, paramValue: %s\n", strKey.data(), strValue.data());
#endif
			if (strKey.compareTo("") != 0) {
				dbMultiple.set(strKey, strValue) ;
			}
			buf[0] = '\0' ;		
			iKey = 0 ;
			strKey.remove(0) ;
			strValue.remove(0) ;
			iValue = 0 ;
		}

		j++ ;
	}	

		//make sure leftover lines (without \n)  are also written
	if (k>0 && strKey.compareTo("") != 0) {
		buf[k++] = '\0';
		strValue.append(buf) ;
		dbMultiple.set(strKey, strValue) ;
	}

    delete []textAreaValue;
}


//
// Routine to add additional key/value pairs to pinger-config file. Is called as part of setPingerConfigAdmin.cgi CGI
//
void Webui::addAdditionalEntries(UtlString paramValue, OsConfigDb& changeDb) 
{
#ifdef TEST_PRINT
	osPrintf("Webui::addAdditionalEntries called paramValue = %s.\n", paramValue.data()) ;
#endif

	int j = 0 ;					// No. lines of text
	int k = 0 ;					// No. chars in line
	char textAreaValue[2048] ;	// paramValue as char* - this is a limit of size of additional values data
	char buf[100] ;				// each line of text put here
	sprintf(textAreaValue, "%s", paramValue.data());
	int newline = 0 ;
	UtlString strKey	;			// newly created key
	UtlString strValue ;			// newly created value
	int iKey = 0 ;				// Whether we have a key
	int iValue = 0 ;			// We are writing the value
	int iWriteBuffer = 0 ;		// We should write to buffer

#ifdef TEST_PRINT
	osPrintf("Webui::addAdditionalEntries textAreaValue = %s\n", textAreaValue) ;
#endif

	while (textAreaValue[j] !=0) {

		if (textAreaValue[j] == ' ' && iValue == 1) {
			iWriteBuffer = 1 ;
		} else if (textAreaValue[j] == ' ' || textAreaValue[j] == '\r') {
			//swallow spaces
		} else if (textAreaValue[j] == ':') {
			//swallow first colon
		
			if (iKey == 0 ) {
				//Create key
				buf[k++] = '\0';
				strKey.append(buf) ;
                // Strip the leading and trailing spaces.
                strKey = strKey.strip(UtlString::both);
				k = 0 ;
				iKey = 1 ;
			} else {
				iWriteBuffer = 1 ;
			}
		} else if (textAreaValue[j] == '\n' || textAreaValue[j] == '\0') {
			//Create value
			newline = 1 ;
			buf[k++] = '\0' ;
			strValue.append(buf) ;
            // Strip the leading and trailing spaces.
            strValue = strValue.strip(UtlString::both);
			k=0 ;
		} else if (iKey == 1) {
			// already have key - this must be a value
			iValue = 1 ;
			iWriteBuffer = 1 ;
		} else {
			iWriteBuffer = 1 ;
		}

		if (iWriteBuffer == 1) {
			//Add char to buffer
			buf[k] =  textAreaValue[j] ;
			newline = 0 ;
			k++ ;
			iWriteBuffer = 0 ;
		}
				
		if (newline) {
			// Write to Db
#ifdef TEST_PRINT
			osPrintf("Webui::addAdditionalEntries after newline paramKey: %s, paramValue: %s\n", strKey.data(), strValue.data());
#endif
			if (strKey.length() != 0) {
				changeDb.set(strKey, strValue) ;
			}
			buf[0] = '\0' ;		
			iKey = 0 ;
            iValue = 0;
			strKey.remove(0) ;
			strValue.remove(0) ;
		}

		j++ ;
	}	

	
	//make sure leftover lines (without \n)  are also written
	if (strKey.length() != 0) {
		buf[k++] = '\0';
		strValue.append(buf) ;
		changeDb.set(strKey, strValue) ;
	}

}

//
// Utility class to check that passwords match, returning HTML code if they do not
//
int Webui::validateChangePassword(UtlString userName, UtlString password1, UtlString password2, UtlString &errorHTML)
{

	if(!userName.isNull() && password1.compareTo(password2) != 0)
		{

		errorHTML.append("<tr>\n<td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\"><p>Status</p></td><td bgcolor=\"#FFCC33\" valign=\"top\">") ;
			errorHTML.append("<p>Passwords are different. Please re-enter.</p>") ;
			errorHTML.append("</TD>\n  </TR>\n") ;
		return -1 ;
	}
	else if (userName.isNull())
	{
		errorHTML.append("<tr>\n<td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\"><p>Status</p></td><td bgcolor=\"#FFCC33\" valign=\"top\">") ;
		errorHTML.append("<p>Must enter a username.</p>") ;
		errorHTML.append("</TD>\n  </TR>\n") ;
		return -1 ;
	} 
	else if (Webutil::validPassword(password1) !=0) 
	{
		errorHTML.append("<tr>\n<td width=\"300\" bgcolor=\"#FFCC33\" valign=\"top\"><p>Status</p></td><td bgcolor=\"#FFCC33\" valign=\"top\">") ;
		errorHTML.append("<p>Invalid Password. Password must contain only the characters a-z, A-Z, 0-9,# or *.") ;
		errorHTML.append("</p></TD>\n  </TR>\n") ;
		return -1 ;
		}

	return 0 ;
}

int Webui::deleteUser(UtlString userId, UtlString password)
{
   // The deployment server and browser-based UI differ on whether userIDs
   // are added in the device or user profiles.  When deleting a user from
   // the browser-based UI, we need to make sure that the user is deleted
   // from both profiles.

   OsConfigDb deviceProfile;
   OsConfigDb userProfile;
   UtlString   key("PHONESET_HTTP_AUTH_DB.");
   UtlBoolean  foundUser = FALSE;

   key.append(userId.data());

   // Load the device profile
   if (deviceProfile.loadFromFile(CONFIGDB_NAME_IN_FLASH) != OS_SUCCESS)
   {
      osPrintf("Could not open %s for reading\n", CONFIGDB_NAME_IN_FLASH);
      return -1;
   }

   // Load the user profile
   if (userProfile.loadFromFile(CONFIGDB_USER_IN_FLASH) != OS_SUCCESS)
   {
      osPrintf("Could not open %s for reading\n", CONFIGDB_USER_IN_FLASH);
      return -1;
   }

   // Remove key from device profile.  If successful, store the profile.
   if (deviceProfile.remove(key) == OS_SUCCESS)
   {
      deviceProfile.storeToFile(CONFIGDB_NAME_IN_FLASH);
      foundUser = TRUE;
   }

   // Remove key from user profile.  If successful, store the profile.
   if (userProfile.remove(key) == OS_SUCCESS)
   {
      userProfile.storeToFile(CONFIGDB_USER_IN_FLASH);
      foundUser = TRUE;
   }

   if (foundUser)
   {
        // remove the user from embedded HTTP server's memory
        HttpServer* pHttpServer   = Pinger::getPingerTask()->getHttpServer();
        pHttpServer->removeUser(userId.data(), password.data() );
        return 0;
   }
   else
   {
      osPrintf("Webui::deleteUser failed to remove user %s\n", userId.data()) ;
      return 1;
   }
}

int Webui::changeUserPassword(UtlString userId, UtlString password)
{
   // The deployment server and browser-based UI differ on whether userIDs
   // are added in the device or user profiles.  When changing a user
   // password, we check both profiles and change it in all of the profiles
   // that we find it.

   OsConfigDb deviceProfile;
   OsConfigDb userProfile;
   UtlBoolean  foundUser = FALSE;
   UtlString   digestPassword;
   UtlString   key("PHONESET_HTTP_AUTH_DB.");
   UtlString   realm(PASSWORD_SECRET);
   UtlString   value;

   key.append(userId.data());

   // Create digest of password
   HttpMessage::buildMd5UserPasswordDigest((const char*) userId.data(),
                                           (const char*) realm.data(), 
                                           (const char*) password.data(),
                                           digestPassword);

   // Load the device profile
   if (deviceProfile.loadFromFile(CONFIGDB_NAME_IN_FLASH) != OS_SUCCESS)
   {
      osPrintf("Could not open %s for reading\n", CONFIGDB_NAME_IN_FLASH);
      return -1;
   }

   // Load the user profile
   if (userProfile.loadFromFile(CONFIGDB_USER_IN_FLASH) != OS_SUCCESS)
   {
      osPrintf("Could not open %s for reading\n", CONFIGDB_USER_IN_FLASH);
      return -1;
   }

   // Check device profile, if entry found, change the password
   if (deviceProfile.get(key, value) == OS_SUCCESS)
   {
      foundUser = TRUE;
      deviceProfile.set(key, digestPassword);
      deviceProfile.storeToFile(CONFIGDB_NAME_IN_FLASH);
   }

   // Check user profile, if entry found, change the password
   if (userProfile.get(key, value) == OS_SUCCESS)
   {
      foundUser = TRUE;
      userProfile.set(key, digestPassword);
      userProfile.storeToFile(CONFIGDB_USER_IN_FLASH);
   }

   // If entry not found in either profile, add to user profile
   if (!foundUser)
   {
      userProfile.set(key, digestPassword);
      userProfile.storeToFile(CONFIGDB_USER_IN_FLASH);
   }

   // Change password on HTTP server
   Pinger* pinger = Pinger::getPingerTask();
   pinger->userLogin(userId, password);

   return 0;
}

// Gets any entries from Configuration Database that aren't part of standard values.
// Takes array of values. 
// Outputs any additional values (ie. other values in config files) and returns them in new DB
void Webui::getAdditionalValues(OsConfigDb& dbAdditional) 
{
	// Get configuration information from config files
	OsConfigDb configDb ;
	configDb.loadFromFile(CONFIGDB_NAME_IN_FLASH) ;
	configDb.loadFromFile(CONFIGDB_USER_IN_FLASH) ;

	OsConfigDb pingerConfig ;
	UtlString strPingerKey("") ;
	UtlString strPingerNewKey ;
	UtlString strPingerValue ;
	int i ;
	int iFound ;

	while (configDb.getNext(strPingerKey, strPingerNewKey, strPingerValue) == OS_SUCCESS) {
    iFound = 0;

 		for (i=0; allConfig[i] != 0; i++) {
			if ((strPingerNewKey.length()!=0) && (strPingerNewKey.index(allConfig[i])==0)) {
				iFound = 1 ;
        break;
			}
		}

		if (!iFound) {
			// param isn't a standard one - add to new database
			if (strPingerNewKey.length()!=0) {
				dbAdditional.set(strPingerNewKey, strPingerValue) ;
			}
		}

		strPingerKey = strPingerNewKey ;
	}
}


// 
// Creates a standard web UI header
//
void Webui::insertHeader(UtlString& html, UtlBoolean bIsSecure)
{
	html.append(HTML_HEADER_2) ;
	
	html.append("<body bgcolor=\"#FFFFFF\" class=\"sipxphone-localweb\">") ;

	html.append("<script type=\"text/javascript\">");
	html.append("function Go(){return;}");
	html.append("</script>");

	int iSpeedDialReadOnly = JNI_isSpeedDialReadOnly();
	//if speedial is readonly
	//add a dummy form with a hidden input element
	//which will be checked by dhtmlMenu.js file
	//to decide not to show "speeddial" menu.
	if( iSpeedDialReadOnly == 1 ){
		html.append("<form name=menu_controller>\n\
			<input name=speeddial_enable type=hidden value=false>\n\
			</form>\n");
	}
	html.append("<script type=\"text/javascript\" src=\"/dhtmlMenu.js\"></script>");
	

	html.append(HTML_HEADER_3) ;
}

void Webui::processAppManager(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{

    UtlString messageBody("<TITLE>Application Manager</TITLE>\n");
    messageBody.append(HTTP_BODY_BEGIN);

    UtlString installURL;
    UtlString uninstallURL;

    requestContext.getCgiVariable("install", installURL);
    requestContext.getCgiVariable("uninstall", uninstallURL);

    if (installURL.length() != 0)
    {
         int retCode = CGI_INSTALL_APPLICATION_OK;
 
         osPrintf("Install application %s\n", installURL.data());
         retCode = JNI_installJavaApplication(installURL.data()) ;
         osPrintf("    %s   return code %d\n", (retCode == CGI_INSTALL_APPLICATION_OK) ? "Succeeded" : "Failed", retCode);
 
         if (retCode == CGI_INSTALL_APPLICATION_OK)
         {
             messageBody.append("<HTML><P><H2>Installation succeeded</H2><P><A HREF=\"appManager.cgi\">Back to Application Manager</A></HTML>");
         } else {
             char buf[256];
             sprintf(buf, "<HTML><P><H2>Installation failed</H2><P>Return code was %d<P><A HREF=\"appManager.cgi\">Back to Application Manager</A></HTML>", retCode);
             messageBody.append(buf);
         }

    } else if (uninstallURL.length() != 0)
    {
         int retCode = CGI_UNINSTALL_APPLICATION_OK;

         osPrintf("Uninstall application %s\n", uninstallURL.data());
         retCode = JNI_uninstallJavaApplication(uninstallURL.data()) ;
         osPrintf("    %s   return code %d\n", (retCode == CGI_UNINSTALL_APPLICATION_OK) ? "Succeeded" : "Failed", retCode);
 
         if (retCode == CGI_UNINSTALL_APPLICATION_OK)
         {
             messageBody.append("<HTML><P><H2>Uninstallation succeeded</H2><P><A HREF=\"appManager.cgi\">Back to Application Manager</A></HTML>");
         } else {
             char buf[256];
             sprintf(buf, "<HTML><P><H2>Uninstallation failed</H2><P>Return code was %d<P><A HREF=\"appManager.cgi\">Back to Application Manager</A></HTML>", retCode);
             messageBody.append(buf);
         }
    } else {

#define MAX_APPS 15
#define MAX_URL_LENGTH 256

        int iActualApps = 0;
        int app;
        char* szApps[MAX_APPS];

        for (int i=0; i<MAX_APPS; i++) {
                szApps[i] = (char*) malloc(MAX_URL_LENGTH);
                memset(szApps[i], 0, MAX_URL_LENGTH);
        }

        JNI_queryInstalledJavaApplications(MAX_APPS, szApps, iActualApps);

        messageBody.append("<HTML><P><H2>Install or uninstall applications</H2><P>This page is used to install or uninstall applications.<br>");
        messageBody.append("These applications are visible from the More button under the \"Apps\" tab.<br>");
        messageBody.append("Please enter the URL of the application JAR file below, then click the \"Install\" button to install.<br>");
        messageBody.append("Likewise, enter the URL of the application to uninstall, then click \"Uninstall\".");
        messageBody.append("<P>An example of a URL might be:<br>file:///c:/sipXphone-apps/myApplet.jar<P>");
        messageBody.append("<FORM ACTION=\"/cgi/appManager.cgi\"><input type=\"text\" name=\"install\" size=\"30\">");
        messageBody.append("<INPUT TYPE=SUBMIT VALUE=\"Install\"><P>");
        messageBody.append("<select name=\"uninstall\"><option selected value=\"\">--Select an application to Uninstall--");

        for (app=0; app < iActualApps; app++)
        {

            messageBody.append("<option value=\"");
            messageBody.append(szApps[app]);
            messageBody.append("\">");
            messageBody.append(szApps[app]);

            free(szApps[app]);
        }

        messageBody.append("</select><INPUT TYPE=SUBMIT VALUE=\"Uninstall\"></FORM></HTML>");
    }

    messageBody.append(HTTP_BODY_END);

    // Build the response message
    HttpBody* body = new HttpBody(messageBody.data(),
        messageBody.length(), CONTENT_TYPE_TEXT_HTML);
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_OK_CODE, HTTP_OK_TEXT);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);
    response->setContentLength(messageBody.length());
}

/* ============================ SPEED DIAL UI ================================== */

//
// Add a speeddial entry
//
void Webui::processAddSpeeddial(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
#define METHOD_ADD	"ADD"
#define METHOD_EDIT "EDIT"

#ifdef DEBUG_WEB_UI
		osPrintf("Webui::processAddSpeeddial Reached processAddSpeeddial\n");
#endif

	// Are we adding or editing a speed dial
	UtlString strMethod ;
		
	// Form variables HTTP POST - for when user adds a new speeddial number
	UtlString strIsSubmit("") ;
	UtlString strID("") ;
	UtlString strLabel("") ;
	UtlString strPhoneNumber("");
	UtlString strIsAPhoneNumber("") ;
	UtlString strErrorMsg("") ;
	UtlString strUseNextId("") ;

	// Params passed in by HTTP GET - for when want to prepopulate (i.e. edit) this form
	UtlString strReqId ;
	UtlString strReqLabel ;
	UtlString strReqNumber ;
	UtlString strReqUrl ;

	UtlString strDelete ;
	int iError = 0 ;


	
	UtlString html(HTML_HEADER_1) ;
	html.append("<title>sipXphone Configuration</title>") ;
	html.append("<META HTTP-EQUIV = \"PRAGMA\" CONTENT = \"no-cache\">") ; 
	html.append("<META HTTP-EQUIV = \"expires\" CONTENT = \"Sunday  9 Janunary 2005 18:42:25 GMT\">");

    UtlBoolean  bIsSecure = FALSE;
    HttpMessage tmprequest = (HttpMessage)request;
    if (tmprequest.getSendProtocol() == OsSocket::SSL_SOCKET)
            bIsSecure = TRUE;
	insertHeader(html,bIsSecure) ;

	html.append("\
<!--table for body of page-->\n\
<table width=\"800\" border=\"0\" valign=\"top\" cellspacing=\"10\">\n\
\n\
	<tr>\n\
		<td><H1>Add/Edit Speed Dial</H1></td>\n\
	</tr>\n\
	<tr>\n\
     <td valign=\"top\" width=\"60%\">\n\
			<!--\"Speeddial\" table-->\n\
			<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n\
			  <tr>\n\
			  	<td colspan=\"2\"><p class=\"textOrange\">Enter Speed dial Number:</p></td>\n\
			  </tr>\n\n\
			  <form method=\"post\" name=\"speed\" action=\"/cgi/addspeeddial.cgi\">\n<input type=\"hidden\" name=\"issubmit\" value=\"yes\" size=\"30\">\n\
              <tr>\n\
                <td width=\"200\" bgcolor=\"#FFCC33\" valign=\"top\" align=\"right\">\n\
					<p>Speed Dial ID:</p></td>\n\
				<td bgcolor=\"#FFCC33\" valign=\"top\"><p>\n ") ;

	
	UtlString speeddialIdHTML("\
		        <input type=\"text\" name=\"speedid\" size=\"5\" value=") ;

	//UtlString afterSpeeddialId (">&nbsp;<input type=\"checkbox\" name=\"usenextid\">Use next free ID</p>") ;

	
	UtlString afterId("\
			  >&nbsp;\
				</td>\n\
			  </tr>\n\n\
			  <tr>\n\
                <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"right\">Label:</td>\n\
				<td bgcolor=\"#FFCC33\" valign=\"top\">\n\
						<input type=\"text\" name=\"label\" size=\"30\" ") ;

	UtlString  afterLabel(">&nbsp; </td> \n\
		</tr> \n\n\
			  <tr>					\n\
			<td bgcolor=\"#FFCC33\">&nbsp;</td> \n\
            <td bgcolor=\"#FFCC33\"> \n\
				     <input type=\"radio\" name=\"isaphonenumber\" value=\"number\" \n\
                      onclick='if (document.speed.phonenumber.value==\"sip:\") \n\
                      document.speed.phonenumber.value=\"\"' ") ;

	UtlString afterIsAPhoneNumber(">&nbsp; Phone Number&nbsp;&nbsp;&nbsp;\n\n\
					  <input type=\"radio\" name=\"isaphonenumber\" value=\"url\" \n\
					  onclick=\'if (document.speed.phonenumber.value==\"\") \n\
					  document.speed.phonenumber.value=\"sip:\"\' ") ;
	

	UtlString afterIsAUrl(">URL</td></tr>&nbsp;  \n\
			   <tr> \n\
				  <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"right\"> Phone Number/URL:</td>\n \
				<td bgcolor=\"#FFCC33\" valign=\"top\">\n\
						<input type=\"text\" name=\"phonenumber\" size=\"30\" ") ;
						
	UtlString afterStatusMsg("&nbsp;</p></td></tr>		 \n\
          </table>\n\
		  <!--End \"Speeddial\" table-->\n\
	 </td>\n\
     <td width=\"40%\" valign=\"top\">\n\
	 		<!--Instructions column-->\n\
            <p class=\"textOrange\">Instructions:</p>\n\
			<p>To set up a new speed dial number:<br><p><ol class=\"normalText\">\n\
				<li>Enter the number that you want to dial instead of the full phone number or SIP URL. \n\
				<li>Enter an identifying label.<br>\n\
				<li>Use the radio buttons to choose the type of value that will actually be dialed: either a phone number or a SIP URL.<br>\n\
				<li>Enter the value to dial: either a phone number or SIP URL, as indicated by the radio button setting.<br>\n\
                 To include a pause or interactive wait in a phone number, type \"p\" or \"i\" (without the quotes).<br>\n\
				<li>Press Enter to add the new speed dial number.</ol></p>\n\
				<p>The status line indicates whether or not your entry is successful. Repeat to add more speed dial numbers.<br>\n\
				You can also <a href=\"/cgi/viewspeeddial.cgi\">view</a> the list of all your speed dial numbers.\n\
				</p>\n\
			\n\
     </td>\n\
	</tr>\n\
</table>\n\
<!--end main body of page table-->\n\
") ;

	// Alternatively, check the <b>Use next free ID</b> check box to have a number supplied for you.<br>
				
	requestContext.getCgiVariable("Method", strMethod) ;

    requestContext.getCgiVariable("issubmit", strIsSubmit);
    requestContext.getCgiVariable("speedid", strID);
	requestContext.getCgiVariable("label", strLabel) ;
	requestContext.getCgiVariable("phonenumber", strPhoneNumber) ;
	requestContext.getCgiVariable("isaphonenumber", strIsAPhoneNumber) ;
	//requestContext.getCgiVariable("usenextid", strUseNextId) ;

	strID =  strID.strip(UtlString::both, ' ') ;
	strLabel = strLabel.strip(UtlString::both, ' ') ;
	strPhoneNumber = strPhoneNumber.strip(UtlString::both, ' ') ;
	strIsAPhoneNumber = strIsAPhoneNumber.strip(UtlString::both, ' ') ;

	requestContext.getCgiVariable("ID", strReqId) ;
	requestContext.getCgiVariable("LABEL", strReqLabel) ;
	requestContext.getCgiVariable("NUMBER", strReqNumber) ;
	requestContext.getCgiVariable("URL", strReqUrl) ;

	strReqId = strReqId.strip(UtlString::both, ' ') ;
	strReqLabel = strReqLabel.strip(UtlString::both, ' ') ;
	strReqNumber = strReqNumber.strip(UtlString::both, ' ') ;
	strReqUrl = strReqUrl.strip(UtlString::both, ' ') ;

#ifdef DEBUG_WEB_UI
		osPrintf("Webui::processAddSpeeddial strMethod=%s\n", strMethod.data());
		osPrintf("Webui::processAddSpeeddial Read in CGI vars\n");
		osPrintf("Webui::processAddSpeeddial ID = %s\n", strReqId.data()) ;
		osPrintf("Webui::processAddSpeeddial LABEL = %s\n", strReqLabel.data()) ;
		osPrintf("Webui::processAddSpeeddial NUMBER = %s\n", strReqNumber.data()) ;
		osPrintf("Webui::processAddSpeeddial URL = %s\n", strReqUrl.data()) ;
#endif


	// Add code to delete entries (add it into page later on)
	strDelete.append("<br><p><a href=\"/cgi/deletespeeddial.cgi?ID=") ;
	strDelete.append(strReqId.data()) ;
	strDelete.append("\">Delete</a></p>") ;

#ifdef DEBUG_WEB_UI
		osPrintf("Webui::processAddSpeeddial strIsSubmit = %s\n", strIsSubmit.data());
#endif


	if (strIsSubmit.compareTo("yes") == 0) {
		// user is submitting a new speeddial


#ifdef DEBUG_WEB_UI
		osPrintf("Webui::processAddSpeeddial yes comparison reached\n");
#endif
		// user is submitting a new speeddial


		/*
		if ((Webutil::validNumber(strID)!=0 || strID.length()==0)&&strUseNextId.compareTo("on")!=0) {
			strErrorMsg.append("Invalid Speed dial number<br>") ;
			iError = 1 ;
		} 
		if ((Webutil::validPhoneNumber(strPhoneNumber)!=0) && (strIsAPhoneNumber.compareTo("number")==0)) {
			strErrorMsg.append("Invalid Phone number<br>") ;
			iError = 1 ; 
		} 
		if (strPhoneNumber.length()==0) {
			strErrorMsg.append("Must specify a phone number/URL value<br>") ;
			iError = 1 ; 
		} 
		if ((strPhoneNumber.compareTo("")==0) && (strIsAPhoneNumber.compareTo("number")==0)) {
			strErrorMsg.append("Need to enter a phone number<br>") ;
			iError = 1 ; 
		} 
		if ((strPhoneNumber.compareTo("")==0) && (strIsAPhoneNumber.compareTo("url")==0)) {
			strErrorMsg.append("Need to enter a URL<br>") ;
			iError = 1 ; 
		}
		if (strIsAPhoneNumber.length()==0) {
			strErrorMsg.append("Need to enter specify if this is a phone number or URL<br>") ;
			iError = 1 ; 
		}
        */
		
		//	validate parameters for speecdial entry 
		int isAPhoneNumber = 1;
		if( strIsAPhoneNumber.compareTo("url") == 0 )
			isAPhoneNumber = 0;
		UtlString strError("");
		char* charPointerError = //"";
        JNI_validateEntry(strID, strLabel, strPhoneNumber, 
				strPhoneNumber, isAPhoneNumber, "<br>" );//, charPointerError)  ;
		strError.append(charPointerError) ;
		charPointerError = NULL;

		if( strError.strip().length() > 0 ) {
			strErrorMsg.append( strError );
			iError=1 ;
		}
		//only when all the fields are validated, check if the iD
		//already existed.
		if( iError == 0 ){
			if( strID.length()!=0  && 
				strMethod.compareTo(METHOD_EDIT)!=0  &&
			    JNI_doesIDAlreadyExist(strID) == 1 ){		
				
				strErrorMsg.append("This speed dial number already exists") ;
				iError = 1;
			}
		}

		// enter speeddial number
		if (iError) 
      {
#ifdef DEBUG_WEB_UI
		osPrintf("Webui::error found %s\n", strErrorMsg.data());
#endif


			if (strMethod.compareTo(METHOD_EDIT)==0) {
			////////
			html.append("<p><b>") ;
			html.append(strID) ;
			html.append("</b></p>") ;
		
			//html.append(speeddialIdHTML) ;
			//html.append(strReqId) ;
			//html.append(">");
		
			//If editing, keep edit Method
			html.append("<input type=\"hidden\" name=\"METHOD\" value=\"EDIT\">") ;
				//Also append ID as hidden parameter
				html.append("<input type=\"hidden\" name=\"speedid\" value=\"") ;
			html.append(strID) ;
			html.append("\"");
			} 
         else 
         {
		////////////

			html.append(speeddialIdHTML) ;
			if ( strID.strip().length()== 0 ){
				char buf1[4] ;
				sprintf(buf1, "%d", JNI_getNextAvailableID()) ;
				strID.append(buf1) ;
			}			
			html.append(strID) ;
			}
			html.append(afterId) ;
			html.append("value=") ;
			Webutil::escapeChars(strLabel) ;
			html.append("\"") ;
			html.append(strLabel) ;
			html.append("\"") ;

			html.append(afterLabel) ;

			if (strIsAPhoneNumber.compareTo("number")==0) {
				html.append("CHECKED") ;
			}

			html.append(afterIsAPhoneNumber) ;
			
			if (strIsAPhoneNumber.compareTo("url")==0) {
				html.append("CHECKED") ;
			}
			
			html.append(afterIsAUrl) ;
			html.append("value=") ;

			Webutil::escapeChars(strPhoneNumber) ;
			html.append("\"") ;
			html.append(strPhoneNumber) ;
			html.append("\"") ;
            html.append(HTML_ADDLINES_AFTERNUMBER) ;
			html.append(HTML_ADDLINES_AFTERDELETE) ;

			// add error message to page
			html.append("Status") ;
			html.append(HTML_ADDLINES_AFTERSTATUS) ;
			html.append("Couldn't save speed dial because:<BR>\n") ;
			html.append(strErrorMsg.data()) ;
			html.append(afterStatusMsg) ;

		}
      else 
      {
		// No errors found
			UtlString emptyString("") ;
			if (strIsAPhoneNumber.compareTo("number")==0) {
				JNI_addSpeeddialEntry((const char*)strID.data(), 
								  (const char*)strLabel.data(), 
								  (const char*)strPhoneNumber.data(),
								   emptyString.data()) ;

			} else {
				JNI_addSpeeddialEntry((const char*)strID.data(), 
								  (const char*)strLabel.data(), 
								  emptyString.data(), 
								  (const char*)strPhoneNumber.data()) ;
			}
			
			html.append(speeddialIdHTML) ;
			char bufID[4] ;	
			sprintf(bufID, "%d", JNI_getNextAvailableID()) ;
			html.append(bufID) ;
			html.append(afterId) ;
			html.append(afterLabel) ;
			//check phone number as default
			html.append("CHECKED") ;
			html.append(afterIsAPhoneNumber) ;
			html.append(afterIsAUrl) ;
			html.append(HTML_ADDLINES_AFTERNUMBER) ;
			html.append(HTML_ADDLINES_AFTERDELETE) ;
				

		

			// create HTML page
			html.append("Status") ;
			html.append(HTML_ADDLINES_AFTERSTATUS) ;
			html.append("Speed dial number ") ;
			Webutil::escapeChars(strPhoneNumber) ;
			html.append(strPhoneNumber.data()) ;
			html.append(" saved as speed dial ") ;
			html.append(strID.data()) ;

			html.append(afterStatusMsg) ;
		}


	} else if (strMethod.compareTo(METHOD_EDIT)==0) {
		// Want to edit this speed number - add current values to HTML page
		html.append("<p><b>") ;
		html.append(strReqId) ;
		html.append("</b></p>") ;

		//html.append(speeddialIdHTML) ;
		//html.append(strReqId) ;
		//html.append(">");
		
		//If editing, keep edit Method
		if (strMethod.compareTo(METHOD_EDIT)==0) {
			html.append("<input type=\"hidden\" name=\"METHOD\" value=\"EDIT\">") ;
		}
		//Also append ID as hidden parameter
		html.append("<input type=\"hidden\" name=\"speedid\" value=\"") ;
			html.append(strReqId) ;
		html.append("\"");

		html.append(afterId) ;

		html.append("value=\"") ;
		Webutil::escapeChars(strReqLabel) ;
		html.append(strReqLabel) ;
		html.append("\"") ;
		html.append(afterLabel) ;
        if (strReqNumber.length()!=0) {
			html.append(" CHECKED") ;
		} 

		html.append(afterIsAPhoneNumber) ;
		if (strReqUrl.length()!=0) {
			html.append(" CHECKED") ; 
		} 
    	html.append(afterIsAUrl) ;    
		html.append("value=\"") ;		
		if (strReqNumber.length()!=0) {
			html.append(strReqNumber) ;
		} else if (strReqUrl.length()!=0) {
			Webutil::escapeChars(strReqUrl) ;
			html.append(strReqUrl) ;
		}
		html.append("\"") ;
		html.append(HTML_ADDLINES_AFTERNUMBER) ;

		html.append(strDelete) ;
		html.append(HTML_ADDLINES_AFTERDELETE) ;

		html.append(HTML_ADDLINES_AFTERSTATUS) ;
		html.append(afterStatusMsg) ;

	} else {
		// display page only - haven't set a METHOD value
#ifdef DEBUG_WEB_UI
		osPrintf("Webui::processAddSpeeddial display page only\n");
#endif
		html.append(speeddialIdHTML) ;
		char bufID[4] ;	
	    sprintf(bufID, "%d", JNI_getNextAvailableID()) ;
		html.append(bufID) ;
		html.append(afterId) ;
		html.append(afterLabel) ;
		//check phone number as default
		html.append("CHECKED") ;
		html.append(afterIsAPhoneNumber) ;
		html.append(afterIsAUrl) ;
		html.append(HTML_ADDLINES_AFTERNUMBER) ;
		html.append(HTML_ADDLINES_AFTERDELETE) ;

		html.append(HTML_ADDLINES_AFTERSTATUS) ;
		html.append(afterStatusMsg) ;
	}


	html.append(HTML_FOOTER) ;


    // Build the response message
    HttpBody* body = new HttpBody(html.data(),
        html.length(), CONTENT_TYPE_TEXT_HTML);
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_OK_CODE, HTTP_OK_TEXT);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);



 
} 

void Webui::processViewSpeeddialMiddle(UtlString& html)
{
    int iSpeeddialId = 0;
	int iSpeeddialLabel = 0 ;
	int iSpeeddialPhoneNumber = 0 ;
	int iSpeeddialUrl = 0 ;
	int iNumOfEntries = JNI_getNumberOfEntries();
    if( iNumOfEntries > 0 ){
		
		char **arrSpeedDialId			= new char*[iNumOfEntries];
		char **arrSpeedDialLabel		= new char*[iNumOfEntries];
		char **arrSpeedDialURL			= new char*[iNumOfEntries];
		char **arrSpeedDialPhoneNumber	= new char*[iNumOfEntries];

	int i = 0 ;

		for (i=0; i<iNumOfEntries; i++) {
            arrSpeedDialId[i] = new char[MAX_SPEEDDIAL_LENGTH];
            memset(arrSpeedDialId[i], 0, MAX_SPEEDDIAL_LENGTH);
            
			arrSpeedDialLabel[i] = new char[MAX_SPEEDDIAL_LENGTH];
            memset(arrSpeedDialLabel[i], 0, MAX_SPEEDDIAL_LENGTH);
			
			arrSpeedDialURL[i] = new char[MAX_SPEEDDIAL_LENGTH];
            memset(arrSpeedDialURL[i], 0, MAX_SPEEDDIAL_LENGTH);
			
			arrSpeedDialPhoneNumber[i] = new char[MAX_SPEEDDIAL_LENGTH];
            memset(arrSpeedDialPhoneNumber[i], 0, MAX_SPEEDDIAL_LENGTH);
    }

		JNI_getSpeeddialEntries(SPEED_DIAL_SCHEMA_ID, iNumOfEntries, arrSpeedDialId, iSpeeddialId) ;
		JNI_getSpeeddialEntries(SPEED_DIAL_SCHEMA_LABEL, iNumOfEntries, arrSpeedDialLabel, iSpeeddialLabel) ;
		JNI_getSpeeddialEntries(SPEED_DIAL_SCHEMA_PHONE_NUMBER, iNumOfEntries, arrSpeedDialPhoneNumber, iSpeeddialPhoneNumber) ;
		JNI_getSpeeddialEntries(SPEED_DIAL_SCHEMA_URL, iNumOfEntries, arrSpeedDialURL, iSpeeddialUrl) ;


		//osPrintf("Webui::processViewSpeeddial iSpeeddialId = %d\n", iSpeeddialId) ;

	// Add speeddial entry to page
	i=0 ; 
	UtlString strDialUrl ;
	UtlString strDialLabel ;
	
    while (i<iSpeeddialId && (strcmp(arrSpeedDialId[i],"")!= 0)) {
		html.append("<tr>\n") ;
		
		//have to escape URL so it appears in page correctly
		strDialUrl.append(arrSpeedDialURL[i]) ;
		Webutil::escapeChars(strDialUrl) ;
		strDialLabel.append(arrSpeedDialLabel[i]) ;
		Webutil::escapeChars(strDialLabel) ;

		// Add dialing
		html.append("<td  width=\"100\" bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><form method=\"post\" action=\"/cgi/dial.cgi\">");
		html.append("<input type=\"hidden\" name=\"dial_url\" size=\"30\" value=\"");
		if (strcmp(arrSpeedDialPhoneNumber[i],"")!=0) {
			// Phone numbers
			UtlString strTemp(arrSpeedDialPhoneNumber[i]) ;
			Webutil::onlyDigits(strTemp) ;
			html.append(strTemp) ;
		} else {
			// URLs
			UtlString strDialURL(strDialUrl) ;
			if (strDialURL.index("sip:")==UTL_NOT_FOUND) {
				html.append("sip:") ;
			}
			html.append(strDialURL) ;
		}
		html.append("\">") ;
		html.append("&nbsp;<input type=\"submit\" name=\"DialButton\" value=\"Dial\"></form>") ;
		html.append("</td>\n") ;


		// Add Speeddial ID
		html.append("<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\">&nbsp;<p>") ;
		html.append(arrSpeedDialId[i]) ;
		html.append("</p></td>\n") ;

		// Add speeddial label
		html.append("<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\">&nbsp;<p>") ;
		html.append(strDialLabel) ;
		html.append("</p></td>\n") ;

		// Add speeddial number
		html.append("<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\">&nbsp;<p>") ;

		if (strcmp(arrSpeedDialPhoneNumber[i],"")!=0) {
			html.append(arrSpeedDialPhoneNumber[i]) ;
		} else {
			html.append(strDialUrl) ;
		}

		html.append("</p></td>\n") ;

		// Add edit button

		UtlString strEditUrl ;
		UtlString strParam ;

		strEditUrl.append("?ID=") ;	
		strParam.append(arrSpeedDialId[i]) ;
		HttpMessage::escape(strParam) ;
		strEditUrl.append(strParam) ;
		strParam.remove(0) ;

		strEditUrl.append("&LABEL=") ;	
		strParam.append(strDialLabel) ;
		HttpMessage::escape(strParam) ;
		strEditUrl.append(strParam) ;
		strParam.remove(0) ;

		strEditUrl.append("&NUMBER=") ;	
		strParam.append(arrSpeedDialPhoneNumber[i]) ;
		HttpMessage::escape(strParam) ;
		strEditUrl.append(strParam) ;
		strParam.remove(0) ;

		strEditUrl.append("&URL=") ;	
		strParam.append(strDialUrl) ;
		HttpMessage::escape(strParam) ;
		strEditUrl.append(strParam) ;
		strParam.remove(0) ;
		
		strEditUrl.append("&METHOD=EDIT") ;


		html.append("<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p>") ;
		html.append("<a href=\"/cgi/addspeeddial.cgi") ;
		html.append(strEditUrl.data()) ;
		html.append("\">Edit</a></p></td>\n") ;

		html.append("</tr>") ;

		i++ ;
		strDialUrl.remove(0) ;
		strDialLabel.remove(0) ;

	}
		//delete it all
		//cleaning action

		for (i=0; i<iNumOfEntries; i++) 
		{
			delete [] arrSpeedDialId[i];
			delete [] arrSpeedDialLabel[i];
			delete [] arrSpeedDialURL[i];
			delete [] arrSpeedDialPhoneNumber[i];
		}
		
		delete [] arrSpeedDialId;
		delete [] arrSpeedDialLabel;
		delete [] arrSpeedDialURL;
		delete [] arrSpeedDialPhoneNumber;
	}
}


//
// View speeddial entries
//
void Webui::processViewSpeeddial(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{

	UtlString strErrorMsg("") ;
	
	UtlString html(HTML_HEADER_1) ;
	html.append("<title>sipXphone Configuration</title>") ;

    UtlBoolean  bIsSecure = FALSE;
    HttpMessage tmprequest = (HttpMessage)request;
    if (tmprequest.getSendProtocol() == OsSocket::SSL_SOCKET)
            bIsSecure = TRUE;
	insertHeader(html,bIsSecure) ;

	html.append("\
<!--table for body of page-->\n\
<table width=\"800\" border=\"0\" valign=\"top\" cellspacing=\"10\">\n\
\n\
	<tr colspan=\"2\">\n\
		<td><H1>Speed Dial</H1>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<p><a href=\"/cgi/addspeeddial.cgi\">Add new Speed dial number</a></p></td>\n\
	</tr>\n\
	<tr>\n\
     <td valign=\"top\" width=\"60%\" colspan=2>\n\
			<!--\"Speeddial\" table-->\n\
			  <table border=\"0\" cellpadding=\"2\" cellspacing=\"2\"><tr>\n\
				<td  width=\"50\" bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p><b>Dial</b></p></td>\n\
			  	<td  width=\"50\" bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p><b>ID</b></p></td>\n\
			  	<td  width=\"150\" bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p><b>Label/Name</b></p></td>\n\
			  	<td  width=\"100\" bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p><b>Phone Number/URL</b></p></td>\n\
			  	<td  width=\"50\" bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p><b>Edit</b></p></td></tr>\n ") ;
    
	UtlString afterTable("\
	  </table>\n\
			  <!--End \"Speeddial\" table-->\n\
	 </td>\n\
     <td width=\"40%\" valign=\"top\">\n\
	 		<!--Instructions column-->\n\
            <p class=\"textOrange\">Instructions:</p>\n\
			<p>This page lists your speed dial numbers. You can <a href=\"/cgi/addspeeddial.cgi\">add</a> new speed dial numbers at any time.</p>\n\
			<li><p>To call a number, click \'Dial\' next to that speed dial number. sipXphone uses its speakerphone to make the call.</p>\n\
			<li><p>To change information for a speed dial number, or to Delete a speed number, click \'Edit\' next to that number.</p>\n\
			\n\
     </td>\n\
	 </tr>\n\
</table>\n\
<!--end main body of page table-->\n ") ;



processViewSpeeddialMiddle(html) ;


	// Add rest of HTML

	html.append(afterTable) ;
	html.append(HTML_FOOTER) ;

	
    // Build the response message
    HttpBody* body = new HttpBody(html.data(),
        html.length(), CONTENT_TYPE_TEXT_HTML);
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_OK_CODE, HTTP_OK_TEXT);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);	
}


void buildSelect( UtlString& results, 
                  const char** options, 
                  const int length, 
                  const int select, 
                  const char* selectName)
{


   // <select name="<SELECTNAME>" size="1">
   results.append("<select name=\"");
   results.append(selectName);
   results.append("\" size=\"1\">\n");

   for (int i=0; i<length; i++)
   {
      if (select == i)
      {
         results.append("<option selected>") ;
      }
      else
      {
         results.append("<option>") ;
      }
      results.append(options[i]); 
      results.append("</option>\n") ;
   }
   results.append("</select>\n") ;    
}

void Webui::processViewSyslog(const HttpRequestContext& requestContext,
                              const HttpMessage& request,
                              HttpMessage*& response)
{
   UtlString results ;
   UtlString controlPanel;	
   UtlString userName ;

   // Make we have an admin user   
   requestContext.getEnvironmentVariable(HttpRequestContext::HTTP_ENV_USER, userName) ;
	if (userName.compareTo("admin") != 0) 
   {
		const char * text = "<HTML><BODY>User Not Authorized<br><br>Must be user \"admin\" to access this page.</BODY></HTML>\n" ;
		HttpServer::processUserNotAuthorized(requestContext,request,response, text) ;
      return ;
	}


   // Build the sys log control panel

   controlPanel.append("<form action=\"config_syslog.cgi\" method=\"POST\">\n") ;
   controlPanel.append("<table width=\"100%\">\n");

   controlPanel.append("<tr>\n<td align=left>");
   
   controlPanel.append("&nbsp;<input type=\"submit\" name=\"action\" value=\"" SYSLOG_ACTION_REFRESH "\">\n") ;
   controlPanel.append("&nbsp;<input type=\"submit\" name=\"action\" value=\"" SYSLOG_ACTION_CLEAR "\">\n") ;

   controlPanel.append("</td>\n");
   controlPanel.append("<td align=right>\n") ;
               
   controlPanel.append("Logging Level:&nbsp;\n") ;
   buildSelect(controlPanel, (const char**) OsSysLog::sPriorityNames, 
         SYSLOG_NUM_PRIORITIES, OsSysLog::getLoggingPriority(),
         "priority") ;
   controlPanel.append("&nbsp;<input type=\"submit\" name=\"action\" value=\"" SYSLOG_ACTION_PRIORITY "\">\n");

   controlPanel.append("</td>\n");   
   controlPanel.append("</tr>\n");
   controlPanel.append("</table>\n");   
   controlPanel.append("</form>\n") ;

   results.append("<html><title>sipXphone Syslog</title><body>\n");
   results.append(controlPanel.data());

   results.append("<hr>\n<br>\n");

   results.append("<table border=1>\n") ;
   
   results.append("<tr>");

   results.append("<th>Count</th><th>Date</th><th>Facility</th><th>Priority</th><th>Host</th><th>Task</th><th>Task Id</th><th>Process Id</th>") ;

   results.append("</tr>\n");

   int maxEntries = 0 ;
   int actualEntries = 0 ;
   if (OsSysLog::getMaxInMemoryLogEntries(maxEntries) == OS_SUCCESS)
   {
      char **entries = new char*[maxEntries];
      OsSysLog::getLogEntries(maxEntries, entries, actualEntries) ;
      for (int i=0; i<actualEntries; i++)
      {
         // Print and free entries as we go...
         if (entries[i] != NULL)
         {
            //results.append(entries[i]) ;
            //results.append("\n") ;
            appendSysLogEntry(entries[i], results) ;

            free(entries[i]) ;
            entries[i] = NULL ;
         }
      }
   }   

   results.append("</table>\n<br><hr>\n");
   results.append(controlPanel.data());
   results.append("</body></html>\n") ;


   // Build the response message
   response = new HttpMessage();
   HttpBody* body = new HttpBody(results.data(), results.length(), CONTENT_TYPE_TEXT_HTML);
   response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION, HTTP_OK_CODE, HTTP_OK_TEXT);
   response->setBody(body);
   response->setContentType(CONTENT_TYPE_TEXT_HTML);
   response->setContentLength(results.length());
}

void Webui::appendSysLogEntry(const char *szSource, UtlString& target) 
{
   UtlString date ;
   UtlString eventCount ;
   UtlString facility ;
   UtlString priority ;
   UtlString hostname ;
   UtlString taskname ;
   UtlString taskId ; 
   UtlString processId ;
   UtlString content ;

   OsSysLog::parseLogString(szSource, date, eventCount, facility, priority,
         hostname, taskname, taskId, processId, content) ;
   
   target.append("<tr>");

   target.append("<td nowrap>") ;
   if (eventCount.length() == 0)
   {
      target.append("&nbsp;") ;
   }
   else
   {
      target.append(eventCount) ;
   }   
   target.append("</td>") ;


   target.append("<td nowrap>") ;
   if (date.length() == 0)
   {
      target.append("&nbsp;") ;
   }
   else
   {
      target.append(date) ;
   }   
   target.append("</td>") ;

   target.append("<td nowrap>") ;
   if (facility.length() == 0)
   {
      target.append("&nbsp;") ;
   }
   else
   {
      target.append(facility) ;
   }   
   target.append("</td>") ;

   target.append("<td nowrap>") ;
   if (priority.length() == 0)
   {
      target.append("&nbsp;") ;
   }
   else
   {
      target.append(priority) ;
   }   
   target.append("</td>") ;

   target.append("<td nowrap>") ;
   if (hostname.length() == 0)
   {
      target.append("&nbsp;") ;
   }
   else
   {
      target.append(hostname) ;
   }   
   target.append("</td>") ;

   target.append("<td nowrap>") ;
   if (taskname.length() == 0)
   {
      target.append("&nbsp;") ;
   }
   else
   {
      target.append(taskname) ;
   }   
   target.append("</td>") ;

   target.append("<td nowrap>") ;
   if (taskId.length() == 0)
   {
      target.append("&nbsp;") ;
   }
   else
   {
      target.append(taskId) ;
   }   
   target.append("</td>") ;

   target.append("<td nowrap>") ;
   if (processId.length() == 0)
   {
      target.append("&nbsp;") ;
   }
   else
   {
      target.append(processId) ;
   }   
   target.append("</td>") ;

   target.append("</tr>\n");

   target.append("<tr>");
   target.append("<td colspan=8><pre>") ;
   target.append(content) ;
   target.append("</pre><br></td") ;

   
   target.append("</tr>");   
}


void Webui::processConfigSyslog(const HttpRequestContext& requestContext,
                                const HttpMessage& request,
                                HttpMessage*& response)
{
   UtlString submit ;
   UtlString priority ;
   UtlString userName ;

   requestContext.getEnvironmentVariable(HttpRequestContext::HTTP_ENV_USER, userName) ;
	if (userName.compareTo("admin") != 0) 
   {
		const char * text = "<HTML><BODY>User Not Authorized<br><br>Must be user \"admin\" to access this page.</BODY></HTML>\n" ;
		HttpServer::processUserNotAuthorized(requestContext,request,response, text) ;
      return ;
	}   

   requestContext.getCgiVariable("action", submit);
   
   if (submit.compareTo(SYSLOG_ACTION_REFRESH, UtlString::ignoreCase) == 0)
   {
      // Do nothing, will refresh below.
   }
   
   else if (submit.compareTo(SYSLOG_ACTION_CLEAR, UtlString::ignoreCase) == 0)
   {
      OsSysLog::clearInMemoryLog() ;
   }
   
   else if (submit.compareTo(SYSLOG_ACTION_PRIORITY, UtlString::ignoreCase) == 0)
   {
      requestContext.getCgiVariable("priority", priority);
      for (int i=0; i<SYSLOG_NUM_PRIORITIES; i++)
      {         
         if (priority.compareTo(OsSysLog::sPriorityNames[i], UtlString::ignoreCase) == 0)
         {
            OsSysLog::setLoggingPriority((OsSysLogPriority) i) ;
            break ;
         }
      } 
   }
   

  	response = new HttpMessage();
	response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION, 
         HTTP_MOVED_TEMPORARILY_CODE, HTTP_MOVED_TEMPORARILY_TEXT);
	response->setLocationField("syslog.cgi");
	response->setContentType(CONTENT_TYPE_TEXT_HTML);
}




//
// Determines if a key is contained in provided array
// Returns 0 if IS contained, 1 otherwise
//
UtlBoolean Webui::isContainedInPingerConfig(UtlString strKey) 
{

	UtlBoolean found = FALSE ;
  int i;

	for (i=0; deviceConfig[i] != 0; i++) {
		if (strKey.index(deviceConfig[i]) == 0) {
			found = TRUE ;
			break ;
		}
  }

  return found ;
}

// Returns TRUE if the parameter is a multiple-valued parameter
// that is settable from the Adminitration->Phone Configuration
// web page.
UtlBoolean Webui::isAdminMultiValueConfig(UtlString strKey) 
{
  
	static const char* paramNameMultiple[] = {
		CONFIG_PHONESET_DIGITMAP,    
		SIP_AUTHENTICATE_DB,
		SIP_AUTHORIZE_USER, 
		SIP_AUTHORIZE_PASSWORD, 
		0
  } ;

	UtlBoolean found = FALSE ;
  int i;

	for (i=0; paramNameMultiple[i] != 0; i++) {
		if (strKey.index(paramNameMultiple[i]) == 0) {
			found = TRUE ;
			break ;
		}
  }

  return found ;
}

//
// Delete speeddial entries
//
void Webui::processDeleteSpeeddial(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{

	UtlString strDelete ;

	requestContext.getCgiVariable("ID", strDelete) ;

	if (strDelete.length()!=0) {
		JNI_deleteSpeeddialEntry((const char*) strDelete.data()) ;
	}

	//OsTask::delay(5000);    // wait 5 seconds

	// Redirect user to view speeddial page.
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
    HTTP_MOVED_TEMPORARILY_CODE, HTTP_MOVED_TEMPORARILY_TEXT);

	UtlString location("/cgi/viewspeeddial.cgi");
    response->setLocationField(location);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);
	



}

void Webui::processViewLinesCgi(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{

	UtlString strErrorMsg;
	UtlString strStatusMsg;
	UtlString newOutBoundLine;
	UtlString strIsSubmit;
	UtlString strSubmitName;
	UtlString strMethod;

	//POST - when user pressed button to save outbound line
	requestContext.getCgiVariable("Method", strMethod) ;
    requestContext.getCgiVariable("issubmit", strIsSubmit);
    requestContext.getCgiVariable("Submit", strSubmitName);
    requestContext.getCgiVariable("isOutBound", newOutBoundLine);
	requestContext.getCgiVariable("statusmessage",strStatusMsg);

    strMethod = strMethod.strip(UtlString::both , ' ');
    strIsSubmit= strIsSubmit.strip(UtlString::both, ' ');
    strSubmitName = strSubmitName.strip(UtlString::both, ' ');
    newOutBoundLine = newOutBoundLine.strip(UtlString::both, ' ');
	strStatusMsg = strStatusMsg.strip(UtlString::both, ' ');
	//status messages from the add/edit line pages
	if (!strStatusMsg.isNull())
	{
		strErrorMsg.append("<p><font color=\"Red\">");
		strErrorMsg.append(strStatusMsg);
		strErrorMsg.append("</font></p>");
	}
	if (strIsSubmit.compareTo("Save", UtlString::ignoreCase) == 0)
	{
      WebuiLineMgrUtil::saveOutboundLine(newOutBoundLine, strErrorMsg);
	}//end save

   // view all lines in table format

   UtlString html(HTML_HEADER_1) ;
	html.append("<title>sipXphone Lines</title>") ;

    UtlBoolean  bIsSecure = FALSE;
    HttpMessage tmprequest = (HttpMessage)request;
    if (tmprequest.getSendProtocol() == OsSocket::SSL_SOCKET)
            bIsSecure = TRUE;
	insertHeader(html,bIsSecure) ;
	html.append(HTML_VIEW_LINE_INSTRUCTIONS);
	html.append(strErrorMsg);
	html.append(HTML_VIEW_LINES_START_TABLE);
	html.append("<form method=\"post\" name=\"line\" action=\"/cgi/MultiLine.cgi\">");

   UtlString addToHtmlString;
   WebuiLineMgrUtil::viewAllLines(addToHtmlString);
   html.append(addToHtmlString);
	html.append(HTML_VIEW_LINES_END_TABLE) ;
   //add a Link or Button to save the updated outbound line
	//get outbound line value
	html.append("<input type=\"Submit\" name=\"isSubmit\" value=\"Save\" size=\"30\"  class=\"normalText\"> Click to save the new \"Call Out As\" line</form>\n");
	// Add rest of HTML
	html.append(HTML_VIEW_LINES_REFERENCE);
	html.append(HTML_FOOTER) ;

   // Build the response message
   HttpBody* body = new HttpBody(html.data(), html.length(), CONTENT_TYPE_TEXT_HTML);
   response = new HttpMessage();
   response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION, HTTP_OK_CODE, HTTP_OK_TEXT);
   response->setBody(body);
   response->setContentType(CONTENT_TYPE_TEXT_HTML);

}

void Webui::processAddLinesCgi(const HttpRequestContext& requestContext,
                   const HttpMessage& request,
                   HttpMessage*& response) 
{
   
#define METHOD_ADD	"ADD"
#define METHOD_EDIT	"EDIT"

   UtlBoolean isAuthorized = TRUE;
   UtlBoolean isErrorAndStayOnSamePage = FALSE;
	//action from this page and status for view line page
   UtlString statusMessage;
   
	UtlString statusFromPreviousAction;
   requestContext.getCgiVariable("STATUSMESSAGE", statusFromPreviousAction);

#ifdef DEBUG_WEB_UI
		osPrintf("Webui::processAddLinesCgi Reached processAddLinesCgi\n");
#endif

	UtlString html(HTML_HEADER_1) ;
	html.append("<title>sipXphone Configuration</title>") ;
	html.append("<META HTTP-EQUIV = \"PRAGMA\" CONTENT = \"no-cache\">") ; 
	html.append("<META HTTP-EQUIV = \"expires\" CONTENT = \"Sunday  9 Janunary 2005 18:42:25 GMT\">");

    UtlBoolean  bIsSecure = FALSE;
    HttpMessage tmprequest = (HttpMessage)request;
    if (tmprequest.getSendProtocol() == OsSocket::SSL_SOCKET)
            bIsSecure = TRUE;
	insertHeader(html,bIsSecure) ;
	html.append(HTML_ADD_LINES_INSTRUCTIONS);
	if(!statusFromPreviousAction.isNull())
	{
		html.append("<p><font color=\"Red\">");
		html.append(statusFromPreviousAction);
		html.append("</font></p>");
	}
	html.append(HTML_ADD_LINE_EDITLINES);
            
	// Are we adding or editing a speed dial
	UtlString strMethod ;
		// Form variables HTTP POST - for when user adds a new speeddial number
	UtlString strIsSubmit;
	UtlString strLineUrl;
	UtlString strLineCallHandling;
	UtlString strLineState;
	UtlString strNextLine;
	UtlString strCredentials;
	UtlString strSubmitName;
    UtlString strStunEnabled;
	UtlBoolean redirectToViewLine = FALSE;

   //POST
	requestContext.getCgiVariable("Method", strMethod) ;
	requestContext.getCgiVariable("issubmit", strIsSubmit);
	requestContext.getCgiVariable("Submit", strSubmitName);
	requestContext.getCgiVariable("sipUrl", strLineUrl);
	requestContext.getCgiVariable("isRegisterEnabled", strLineState);
	requestContext.getCgiVariable("isCallHandling", strLineCallHandling) ;
    requestContext.getCgiVariable("isStunEnabled", strStunEnabled) ;
    
	requestContext.getCgiVariable("credential", strCredentials) ;

	strMethod = strMethod.strip(UtlString::both , ' ');
	strIsSubmit= strIsSubmit.strip(UtlString::both, ' ');
	strSubmitName = strSubmitName.strip(UtlString::both, ' ');
	strLineUrl = strLineUrl.strip(UtlString::both, ' ');
	strLineState = strLineState.strip(UtlString::both, ' ');
	strLineCallHandling = strLineCallHandling.strip(UtlString::both, ' ');
	strCredentials = strCredentials.strip(UtlString::both, ' ');

   //GET
  	// Params passed in by HTTP GET - for when want to prepopulate (i.e. edit) this form
	UtlString ReqSipUrl ;
	UtlString ReqLineUser;
	requestContext.getCgiVariable("URL", ReqSipUrl) ;
	requestContext.getCgiVariable("USER", ReqLineUser) ;
	ReqSipUrl = ReqSipUrl.strip(UtlString::both, ' ');
	ReqLineUser = ReqLineUser.strip(UtlString::both, ' ');

	// Add code to delete entries (add it into page later on)
   UtlString strDelete ;
	strDelete.append("<br><p><a href=\"/cgi/deleteLine.cgi?LINE=") ;
	strDelete.append(strLineUrl.data()) ;
	strDelete.append("\">Delete</a></p>") ;


   if (strSubmitName.compareTo("save", UtlString::ignoreCase) == 0)
   {
      WebuiLineMgrUtil::saveAddLines(strLineUrl, 
						strLineCallHandling, 
						strLineState, 
						ReqLineUser,                         
						statusMessage,
						redirectToViewLine,
						isErrorAndStayOnSamePage);   
   } //end save
   else if (strSubmitName.compareTo("Delete", UtlString::ignoreCase) == 0)
   {
    	redirectToViewLine = TRUE;
      UtlString sipUrl;
      requestContext.getCgiVariable("identity", sipUrl);
      WebuiLineMgrUtil::deleteAddLines(sipUrl, statusMessage);
   }// end delete
   else if (strSubmitName.compareTo("Update", UtlString::ignoreCase) == 0)
   {
      UtlString sipUrl;
	  UtlString previousSipUrl;
      requestContext.getCgiVariable("identity", sipUrl);
      requestContext.getCgiVariable("PreviousIdentity" , previousSipUrl);

      WebuiLineMgrUtil::updateAddLines(sipUrl,
                                       previousSipUrl,
                                       strLineCallHandling,
                                       strStunEnabled, 
                                       strLineState,                                       
                                       isErrorAndStayOnSamePage,
                                       redirectToViewLine,
                                       statusMessage);
   }// end update
   else if (strMethod.compareTo(METHOD_EDIT , UtlString::ignoreCase)==0)
   {
      UtlString userName;
      requestContext.getEnvironmentVariable(HttpRequestContext::HTTP_ENV_USER, userName);
      if (ReqSipUrl.isNull())
      {
         requestContext.getCgiVariable("PreviousIdentity", ReqSipUrl);
      }
      UtlString addToHtml;
      WebuiLineMgrUtil::editAddLines(userName,
                                        ReqSipUrl,
                                        addToHtml,
                                        isAuthorized,
                                        redirectToViewLine,
                                        statusMessage);

      if(!isAuthorized)
      {
         osPrintf("Webui::getPingerConfigAdmin - user is not admin\n") ;
         const char * text = "<HTML><BODY>User Not Authorized<br><br>Must be user \"admin\" to access this page.</BODY></HTML>\n" ;
         HttpServer::processUserNotAuthorized(requestContext,request,response, text) ;
      }
      else
      {
         html.append(addToHtml);
         html.append(HTML_ADDLINES_UPDATEBUTTON);
         html.append(HTML_ADDLINES_DELETEBUTTON);
         html.append(HTML_ADDLINES_AFTERBUTTON);
         html.append(HTML_ADDLINES_AFTERSTATUS) ;
         html.append(HTML_ADD_LINE2) ;
      }

   }
   else
   {
      UtlString userName;
      requestContext.getEnvironmentVariable(HttpRequestContext::HTTP_ENV_USER, userName);
      if ( (ReqLineUser.compareTo("device" , UtlString::ignoreCase) == 0) && userName.compareTo("admin")!=0)
      {

         // Check user is "admin," if not redirect to not authorized page
         isAuthorized = FALSE;
         osPrintf("Webui::getPingerConfigAdmin - user is not admin\n") ;
         const char * text = "<HTML><BODY>User Not Authorized<br><br>Must be user \"admin\" to access this page.</BODY></HTML>\n" ;
         HttpServer::processUserNotAuthorized(requestContext,request,response, text) ;
      }
      else
      {

         html.append("<input type=\"text\" name =\"sipUrl\" size=\"40\" >");	// afterheader
		   html.append("<input type=\"hidden\" name=\"User\" value=\"") ;
		   html.append(ReqLineUser) ;
		   html.append("\">") ;
         html.append(HTML_ADD_LINE_AFTERURL);
         html.append(HTML_ADD_LINE_AFTERCALLHANDLING1);
         html.append("CHECKED");//call forwarding is disabled by default
         html.append(HTML_ADD_LINE_AFTERCALLHANDLING);
         html.append(HTML_ADD_LINE_AFTERREGISTRATION);
         html.append("CHECKED"); //check registration radio button by default
         //html.append(afterRegistration2);
         html.append(HTML_ADD_LINE_AFTERLINEREGISTRATION);
		   //check if user has entered SipUrl , only then allow user to 
		   html.append(HTML_ADD_LINE_CREATECREDENTIALTABLE);
         html.append("</table>");
         html.append("</td>\n\
		      </tr> \n\
			   <tr>\n\
               <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"centre\"> ") ;	//	afterCredentials);
         html.append("<input type=\"submit\" name=\"Submit\" value=\"Save\"> ");	//	SaveButton);
         html.append(HTML_ADDLINES_AFTERBUTTON);
         html.append(HTML_ADD_LINE2);
      }
   }

	if (redirectToViewLine && isAuthorized)
	{
		response = new HttpMessage();
		response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
		HTTP_MOVED_TEMPORARILY_CODE, HTTP_MOVED_TEMPORARILY_TEXT);

		// the url of the page that you want to redirect to.
		UtlString location("/cgi/MultiLine.cgi?STATUSMESSAGE=");
		location.append(statusMessage);
		response->setLocationField(location);
		response->setContentType(CONTENT_TYPE_TEXT_HTML);
	}
   else if( isErrorAndStayOnSamePage)
   {
   	response = new HttpMessage();
		response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
		HTTP_MOVED_TEMPORARILY_CODE, HTTP_MOVED_TEMPORARILY_TEXT);

		// the url of the page that you want to redirect to.
		UtlString location("/cgi/addLines.cgi?STATUSMESSAGE=");
		location.append(statusMessage);
		response->setLocationField(location);
		response->setContentType(CONTENT_TYPE_TEXT_HTML);
   }
   else if(isAuthorized)
	{
		html.append(HTML_ADD_LINE3_PARAM_TABLE);
		html.append(HTML_FOOTER) ;
		// Build the response message
		HttpBody* body = new HttpBody(html.data(), html.length(), CONTENT_TYPE_TEXT_HTML);
		response = new HttpMessage();
		response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION, HTTP_OK_CODE, HTTP_OK_TEXT);
		response->setBody(body);
		response->setContentType(CONTENT_TYPE_TEXT_HTML);
	}

}

void Webui::processAddCredentialsCgi(const HttpRequestContext& requestContext,
                   const HttpMessage& request,
                   HttpMessage*& response) 
{
#define METHOD_ADD	"ADD"
#define METHOD_EDIT "EDIT"
	UtlString statusMessage;

#ifdef DEBUG_WEB_UI
		osPrintf("Webui::processAddLinesCgi Reached processAddLinesCgi\n");
#endif

	UtlString html(HTML_HEADER_1) ;
	html.append("<title>sipXphone Configuration</title>") ;
	html.append("<META HTTP-EQUIV = \"PRAGMA\" CONTENT = \"no-cache\">") ; 
	html.append("<META HTTP-EQUIV = \"expires\" CONTENT = \"Sunday  9 Janunary 2005 18:42:25 GMT\">");

    UtlBoolean  bIsSecure = FALSE;
    HttpMessage tmprequest = (HttpMessage)request;
    if (tmprequest.getSendProtocol() == OsSocket::SSL_SOCKET)
            bIsSecure = TRUE;
	insertHeader(html,bIsSecure) ;
	html.append(HTML_ADD_CREDENTIALS_INSTRUCTIONS);


	// Are we adding or editing a speed dial
	UtlString strMethod ;
		// Form variables HTTP POST - for when user adds a new speeddial number
	UtlString strIsSubmit("") ;
	UtlString strLineUrl("") ;
	UtlString strRealm("") ;
	UtlString strPassword("");
	UtlString strUserId("") ;
	UtlString strErrorMsg("") ;
	UtlString strSubmitName("");
	UtlString strConfirmPassword("");
	UtlBoolean redirectToAddLine = FALSE;

   //POST
	requestContext.getCgiVariable("Method", strMethod) ;
   requestContext.getCgiVariable("issubmit", strIsSubmit);
   requestContext.getCgiVariable("Submit", strSubmitName);
 	requestContext.getCgiVariable("Realm", strRealm) ;
   requestContext.getCgiVariable("UserId", strUserId);
   requestContext.getCgiVariable("Password", strPassword);
	requestContext.getCgiVariable("ConfirmPassword", strConfirmPassword);
  
   strMethod = strMethod.strip(UtlString::both , ' ');
   strIsSubmit= strIsSubmit.strip(UtlString::both, ' ');
   strSubmitName = strSubmitName.strip(UtlString::both, ' ');
   strRealm = strRealm.strip(UtlString::both , ' ');
   strUserId= strUserId.strip(UtlString::both, ' ');
   strPassword = strPassword.strip(UtlString::both, ' ');
   strConfirmPassword = strConfirmPassword.strip(UtlString::both , ' ');
   
   //GET
  	// Params passed in by HTTP GET - for when want to prepopulate (i.e. edit) this form
	UtlString ReqSipUrl ;
	UtlString ReqRealm ;
	UtlString ReqUserId;
	UtlString ReqMethod;

	requestContext.getCgiVariable("URL", ReqSipUrl) ;
	requestContext.getCgiVariable("USERID", ReqUserId) ;
	requestContext.getCgiVariable("REALM", ReqRealm) ;
	requestContext.getCgiVariable("METHOD", ReqMethod) ;
	
   ReqSipUrl = ReqSipUrl.strip(UtlString::both, ' ') ;
	ReqUserId = ReqUserId.strip(UtlString::both, ' ') ;
	ReqRealm = ReqRealm.strip(UtlString::both, ' ') ;
	ReqMethod = ReqMethod.strip(UtlString::both, ' ') ;

	// Add code to delete entries (add it into page later on)
   UtlString strDelete ;
	strDelete.append("<br><p><a href=\"/cgi/deleteLine.cgi?LINE=") ;
	strDelete.append(strLineUrl.data()) ;
	strDelete.append("\">Delete</a></p>") ;

   if (ReqMethod.compareTo("Add", UtlString::ignoreCase) == 0)
   {
      UtlString ReqSipUrl;
		requestContext.getCgiVariable("Url", ReqSipUrl);
      UtlString escapedReqSipurl(ReqSipUrl);
      Webutil::escapeChars(escapedReqSipurl);

		html.append(HTML_ADD_CREDENTIALS1);              
		html.append(HTML_ADD_CREDENTIALS_StartTable);
		html.append("<b>") ;
		html.append(escapedReqSipurl) ;
		html.append("</b>") ;
		html.append("<input type=\"hidden\" name=\"identity\" value=\"") ;
		html.append(escapedReqSipurl);
		html.append("\">") ;
		html.append(HTML_ADD_CREDENTIALS_afterUrlDisplay);
		html.append(HTML_ADD_CREDENTIALS_inputRealm);
		html.append(HTML_ADD_CREDENTIALS_afterRealm);
		html.append(HTML_ADD_CREDENTIALS_inputUserId);
		html.append(HTML_ADD_CREDENTIALS_afterUserId);
		html.append(HTML_ADD_CREDENTIALS_afterPassword);
		html.append(HTML_ADD_CREDENTIALS_afterConfirmPassword);
		html.append("<input type=\"submit\" name=\"Submit\" value=\"Save\"> ");	//	SaveButton);
		html.append(HTML_ADDLINES_AFTERBUTTON);
		html.append(HTML_ADDLINES_AFTERSTATUS) ;
		html.append(HTML_ADD_CREDENTIALS2) ;

   }// end delete
   else if (ReqMethod.compareTo("Edit", UtlString::ignoreCase)==0)
   {
      UtlString escapedReqSipurl(ReqSipUrl);
      Webutil::escapeChars(escapedReqSipurl);

      /// Want to edit this credential
		html.append(HTML_ADD_CREDENTIALS1);              
		html.append(HTML_ADD_CREDENTIALS_StartTable);
		html.append("<b>") ;
		html.append(escapedReqSipurl) ;
		html.append("</b>") ;

	   //Also append ID as hidden parameter
		html.append("<input type=\"hidden\" name=\"identity\" value=\"") ;
		html.append(escapedReqSipurl) ;
		html.append("\">") ;

		html.append("<input type=\"hidden\" name=\"previousRealm\" value=\"") ;
		html.append(ReqRealm) ;
		html.append("\">") ;
		
		html.append("<input type=\"hidden\" name=\"previousUserId\" value=\"") ;
		html.append(ReqUserId) ;
		html.append("\">") ;

		html.append(HTML_ADD_CREDENTIALS_afterUrlDisplay);
      html.append("<input type=\"text\" name =\"Realm\" size=\"70\" value=\"");
      html.append(ReqRealm);
      html.append("\">");
      html.append(HTML_ADD_CREDENTIALS_afterRealm);
      html.append("<input type=\"text\" name =\"UserId\" size=\"70\" value=\"");
      html.append(ReqUserId);
      html.append("\">");
		html.append(HTML_ADD_CREDENTIALS_afterUserId);
		html.append(HTML_ADD_CREDENTIALS_afterPassword);
		html.append(HTML_ADD_CREDENTIALS_afterConfirmPassword);
		html.append(HTML_ADDLINES_UPDATEBUTTON);
		html.append(HTML_ADDLINES_DELETEBUTTON);
		html.append(HTML_ADDLINES_AFTERBUTTON);
		html.append(HTML_ADDLINES_AFTERSTATUS) ;
		html.append(HTML_ADD_CREDENTIALS2) ;

	}
	//Save
   else if (strSubmitName.compareTo("Save", UtlString::ignoreCase) == 0)
   {
      UtlString sipUrl;
      requestContext.getCgiVariable("identity", sipUrl);
      UtlBoolean isCredentialAdded = FALSE;
      
      WebuiLineMgrUtil::saveAddCredentials(sipUrl,
                                          strUserId,
                                          strRealm,
                                          strPassword,
                                          strConfirmPassword,
                                          strErrorMsg,
                                          statusMessage,
                                          isCredentialAdded,
                                          redirectToAddLine);
    	
      if(!isCredentialAdded)
  	   {
         UtlString sipUrltemp(sipUrl);
         Webutil::escapeChars(sipUrltemp);
		   html.append(strErrorMsg);
		   html.append(HTML_ADD_CREDENTIALS1);              
		   html.append(HTML_ADD_CREDENTIALS_StartTable);
		   html.append("<b>") ;
		   html.append(sipUrltemp) ;
		   html.append("</b>") ;
		   //Also append ID as hidden parameter
		   html.append("<input type=\"hidden\" name=\"identity\" value=\"") ;
		   html.append(sipUrltemp) ;
		   html.append("\">") ;
		   html.append(HTML_ADD_CREDENTIALS_afterUrlDisplay);
		   html.append("<input type=\"text\" name =\"Realm\" size=\"70\" value=\"");
		   html.append(strRealm);
		   html.append("\">");
   	   html.append(HTML_ADD_CREDENTIALS_afterRealm);
		   html.append("<input type=\"text\" name =\"UserId\" size=\"70\" value=\"");
		   html.append(strUserId);
		   html.append("\">");
		   html.append(HTML_ADD_CREDENTIALS_afterUserId);
		   html.append(HTML_ADD_CREDENTIALS_afterPassword);
		   html.append(HTML_ADD_CREDENTIALS_afterConfirmPassword);
		   html.append("<input type=\"submit\" name=\"Submit\" value=\"Save\"> ");	//	SaveButton);
		   html.append(HTML_ADDLINES_AFTERBUTTON);
		   html.append(HTML_ADDLINES_AFTERSTATUS) ;
		   html.append(HTML_ADD_CREDENTIALS2) ;
   	}

   }//end Save
   else if (strSubmitName.compareTo("Delete", UtlString::ignoreCase) == 0)
   {
  		redirectToAddLine = TRUE;
		//check if paswwords confirm
		UtlString sipUrl;
		requestContext.getCgiVariable("identity", sipUrl);

      WebuiLineMgrUtil::deleteAddCredentials(sipUrl, strRealm, statusMessage);
                                          
      html.append(strErrorMsg);
		html.append(HTML_ADD_CREDENTIALS1);              

   }
   else if (strSubmitName.compareTo("Update", UtlString::ignoreCase) == 0)
   {
  		UtlString sipUrl;
      UtlString sipRealm;
      UtlString sipUserId;
		requestContext.getCgiVariable("identity", sipUrl);
      Webutil::escapeChars(sipUrl);
		requestContext.getCgiVariable("previousRealm", sipRealm);
		requestContext.getCgiVariable("previousUserId", sipUserId);

      if (WebuiLineMgrUtil::updateAddCredentials(sipUrl,
                               sipRealm,
                               sipUserId,
                               strRealm,
                               strUserId,
                               strPassword,
                               strConfirmPassword,
                               statusMessage,
                               redirectToAddLine) )
      {
         html.append(strErrorMsg);
		   html.append(HTML_ADD_CREDENTIALS1);              
      }
      else
      {
  			strErrorMsg.append("<p><font color=\"Red\">Invalid parameters: Re-enter Parameters.</font></p>");
			html.append(strErrorMsg);
         UtlString sipUrltemp(sipUrl);
         Webutil::escapeChars(sipUrltemp);

			html.append(HTML_ADD_CREDENTIALS1);              
			html.append(HTML_ADD_CREDENTIALS_StartTable);
			html.append("<b>") ;
			html.append(sipUrltemp) ;
			html.append("</b>") ;
			//Also append ID as hidden parameter
			html.append("<input type=\"hidden\" name=\"identity\" value=\"") ;
			html.append(sipUrltemp) ;
			html.append("\">") ;

		html.append("<input type=\"hidden\" name=\"previousRealm\" value=\"") ;
		html.append(sipRealm) ;
		html.append("\">") ;
		
		html.append("<input type=\"hidden\" name=\"previousUserId\" value=\"") ;
		html.append(sipUserId) ;
		html.append("\">") ;

			html.append(HTML_ADD_CREDENTIALS_afterUrlDisplay);
		   html.append("<input type=\"text\" name =\"Realm\" value=\"");
		   html.append(strRealm);
		   html.append("\">");
			html.append(HTML_ADD_CREDENTIALS_afterRealm);
			html.append("<input type=\"text\" name=\"userId\" value=\"") ;
			html.append(strUserId) ;
			html.append("\">") ;
			html.append(HTML_ADD_CREDENTIALS_afterUserId);
			html.append(HTML_ADD_CREDENTIALS_afterPassword);
			html.append(HTML_ADD_CREDENTIALS_afterConfirmPassword);
			html.append(HTML_ADDLINES_UPDATEBUTTON);
			html.append(HTML_ADDLINES_DELETEBUTTON);
			html.append(HTML_ADDLINES_AFTERBUTTON);
			html.append(HTML_ADDLINES_AFTERSTATUS) ;
			html.append(HTML_ADD_CREDENTIALS2) ;
      }
   }// end update
   else
   {
		html.append(HTML_ADD_CREDENTIALS1);              
		html.append(HTML_ADD_CREDENTIALS_afterUrlDisplay);
		html.append(ReqRealm);
		html.append(HTML_ADD_CREDENTIALS_afterRealm);
		html.append(ReqUserId);
		html.append(HTML_ADD_CREDENTIALS_afterUserId);
		html.append(HTML_ADD_CREDENTIALS_afterPassword);
		html.append(HTML_ADD_CREDENTIALS_afterConfirmPassword);
		html.append(HTML_ADDLINES_UPDATEBUTTON);
		html.append(HTML_ADDLINES_DELETEBUTTON);
		html.append(HTML_ADDLINES_AFTERBUTTON);
		html.append(HTML_ADDLINES_AFTERSTATUS) ;
		html.append(HTML_ADD_CREDENTIALS2) ;
   }

	if (redirectToAddLine)
	{
		response = new HttpMessage();
		response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
		HTTP_MOVED_TEMPORARILY_CODE, HTTP_MOVED_TEMPORARILY_TEXT);

		UtlString sipUrl;
		requestContext.getCgiVariable("identity", sipUrl);
		
		UtlString redirectUrl("/cgi/addLines.cgi?URL=");
      HttpMessage::escape(sipUrl);
		redirectUrl.append(sipUrl);
		redirectUrl.append("&METHOD=EDIT&STATUSMESSAGE=");
      HttpMessage::escape(statusMessage) ;
		redirectUrl.append(statusMessage);
		response->setLocationField(redirectUrl);
		response->setContentType(CONTENT_TYPE_TEXT_HTML);
	}
	else
	{
		html.append(HTML_ADD_CREDENTIALS_PARAM_TABLE);
		html.append(HTML_FOOTER) ;
		// Build the response message
		HttpBody* body = new HttpBody(html.data(), html.length(), CONTENT_TYPE_TEXT_HTML);
		response = new HttpMessage();
		response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION, HTTP_OK_CODE, HTTP_OK_TEXT);
		response->setBody(body);
		response->setContentType(CONTENT_TYPE_TEXT_HTML);
	}
}
