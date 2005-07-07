// $Id: //depot/OPENDEV/sipXphone/src/web/WebuiLineMgrUtil.cpp#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// WebuiLineMgrUtil.cpp: implementation of the WebuiLineMgrUtil class.
//
//////////////////////////////////////////////////////////////////////

#include "web/WebuiLineMgrUtil.h"
#include "web/Webui.h"
#include "web/Webutil.h"
//multiline
#include "net/SipLine.h"
#include "net/SipLineList.h"
#include "net/SipLineCredentials.h"
#include "net/SipLineMgr.h"
#include "net/Url.h"

#include "cp/CallManager.h"
#include "pinger/PingerConfig.h"
#include "net/SipUserAgent.h"


#define HTML_VIEW_LINES_EMPTY_COLUMN "<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\">&nbsp;</td>\n"
#define MAX_CALLS 5


#define HTML_ADD_LINE_STUN "&nbsp;</td>\n\
            </tr>\n\
            <tr> \n\
               <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"right\">Stun NAT traversal:</td>\n\
               <td bgcolor=\"#FFCC33\"> \n\
                     <input type=\"radio\" name=\"isStunEnabled\" value=\"Enabled\" "

#define HTML_ADD_LINE_STUN_END ">\n\
                &nbsp; Enabled &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
                      <input type=\"radio\" name=\"isStunEnabled\" value=\"Disabled\" "

#define HTML_ADD_LINE_CALL_HANDLING ">&nbsp; Disabled &nbsp;</td>\n\
            </tr>\n\
               <tr> \n\
               <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"right\">Allow Forwarding:</td>\n\
               <td bgcolor=\"#FFCC33\"> \n\
                     <input type=\"radio\" name=\"isCallHandling\" value=\"Enabled\" "

#define HTML_ADD_LINE_CALL_HANDLING_END ">\n\
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
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WebuiLineMgrUtil::WebuiLineMgrUtil()
{

}

WebuiLineMgrUtil::~WebuiLineMgrUtil()
{
}

void
WebuiLineMgrUtil::saveOutboundLine(
                                  UtlString& newOutBoundLine,
                                  UtlString& strErrorMsg)
{
    Url url(newOutBoundLine);
    Url uri;
    Url canonicalUrl;
    getUriForUrl(url , uri);
    getCanonicalUrl(url , uri , canonicalUrl);
    //Save new outbound line
    Pinger* pinger = Pinger::getPingerTask();
    SipLineMgr* lineMgr = pinger->getLineManager() ;
    if ( pinger && lineMgr )
    {
        UtlString prevOutboundLine;
        //get outbound line value
        lineMgr->getDefaultOutboundLine(prevOutboundLine);
        if ( prevOutboundLine.compareTo(canonicalUrl.toString()) == 0 )
        {
            strErrorMsg.append( "<p><font color=\"Red\">Same Call Out As line saved as before</font></p>");
        } else
        {
            lineMgr->setDefaultOutboundLine(canonicalUrl);
            CallManager * callMgr = pinger->getCallManager();
            if ( callMgr )
            {
                callMgr->setOutboundLine(canonicalUrl.toString());
                strErrorMsg.append( "<p><font color=\"Red\">New Call Out As line saved</font></p>");
            }
            callMgr = NULL;
        }
        SipLineMgrSerialize(*lineMgr,true);
        pinger = NULL;
        lineMgr = NULL;
    }
}

void
WebuiLineMgrUtil::viewAllLines(UtlString& addToHtmlString)
{
    UtlString html;

    Pinger* pinger = Pinger::getPingerTask();
    SipLineMgr* lineMgr = pinger->getLineManager() ;
    if ( pinger && lineMgr )
    {
        int noOfLines = lineMgr->getNumLines();
        SipLine* lines = new SipLine[noOfLines];
        int returnVal;
        lineMgr->getLines(noOfLines , returnVal , lines);
        // Add speeddial entry to page
        UtlString strLineUrl ;
        UtlString strlineState;
        UtlString strLineCallHandling;
        UtlString strOutboundLine;
        UtlString currentOutboundLine;

        //get outbound line value
        lineMgr->getDefaultOutboundLine(currentOutboundLine);
        html.append("<tr><td bgcolor=\"#FFCC33\" valign=\"top\" align=\"left\"><b>Device Line</b></td>");
        // Add Call Handling
        html.append(HTML_VIEW_LINES_EMPTY_COLUMN);
        html.append(HTML_VIEW_LINES_EMPTY_COLUMN);
        html.append(HTML_VIEW_LINES_EMPTY_COLUMN);
        html.append(HTML_VIEW_LINES_EMPTY_COLUMN);
        html.append("</tr>\n");

        UtlBoolean isOneDeviceLine = FALSE;
        int i = 0;
        while ( i < returnVal )
        {
            if ( lines[i].getUser().compareTo("Device", UtlString::ignoreCase) == 0 )
            {
                isOneDeviceLine = TRUE;

                html.append("<tr>\n") ;
                Url lineUrl = lines[i].getUserEnteredUrl();
                Url identityUri = lines[i].getIdentity();
                Url canonicalUrl = lines[i].getCanonicalUrl();
                int lineState = lines[i].getState();
                UtlString User = lines[i].getUser();
                UtlBoolean callHandling = lines[i].getCallHandling();

                //have to escape URL so it appears in page correctly
                strLineUrl.append(lineUrl.toString()) ;
                Webutil::escapeChars(strLineUrl) ;

                //line state
                if ( lineState == SipLine::LINE_STATE_PROVISIONED )
                {
                    strlineState.append("Provision");
                } else
                {
                    strlineState.append("Register");
                }
                Webutil::escapeChars(strlineState) ;

                //call handling
                if ( callHandling )
                    strLineCallHandling.append("Enabled");
                else
                    strLineCallHandling.append("Disabled");

                Webutil::escapeChars(strLineCallHandling) ;

                //outbound line
                if ( canonicalUrl.toString().compareTo(currentOutboundLine) == 0 )
                {
                    strOutboundLine.remove(0);
                    strOutboundLine.append("CHECKED");
                } else
                {
                    strOutboundLine.remove(0);
                }

                Webutil::escapeChars(strOutboundLine) ;

                // Line editing button
                UtlString strEditUrl ;
                UtlString strParam ;

                strEditUrl.append("?URL=") ;
                strParam.append(lineUrl.toString()) ;
                HttpMessage::escape(strParam) ;
                strEditUrl.append(strParam) ;
                strParam.remove(0) ;
                strEditUrl.append("&METHOD=EDIT") ;

                // Add Sip Url
                html.append("<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"left\"><p>") ;
                html.append(strLineUrl) ;
                html.append("</p></td>\n") ;

                // Add Call Handling
                html.append("<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p>") ;
                html.append(strLineCallHandling) ;
                html.append("</p></td>\n") ;

                //Add Line Registration State
                html.append("<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p>") ;
                html.append(strlineState) ;
                html.append("</p></td>\n") ;

                // Add OutBound
                html.append("<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p>\n\
                    <input type=\"radio\" name=\"isOutBound\" value= \" ");
                html.append(strLineUrl);
                html.append(" \" ");
                html.append(strOutboundLine) ;
                html.append(" > ") ;
                html.append("</p></td>\n") ;

                // Add Line Edit link
                html.append("<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p>") ;
                html.append("<a href=\"/cgi/addLines.cgi") ;
                html.append(strEditUrl.data()) ;
                html.append("\">Edit</a></p></td>\n") ;

                html.append("</tr>") ;
                strLineUrl.remove(0) ;
                strlineState.remove(0);
                strLineCallHandling.remove(0);
                //strOutboundLine.remove(0);
            }
            i++ ;
        }
        //if no device line , let user add a device line
        if ( !isOneDeviceLine )
        {
            html.append("<tr><td bgcolor=\"#FFCC33\" valign=\"top\" align=\"left\">\n\
                <p><a href=\"/cgi/addLines.cgi?User=DEVICE\">Add New Line</a></p></td>\n") ;
            html.append(HTML_VIEW_LINES_EMPTY_COLUMN);
            html.append(HTML_VIEW_LINES_EMPTY_COLUMN);
            html.append(HTML_VIEW_LINES_EMPTY_COLUMN);
            html.append(HTML_VIEW_LINES_EMPTY_COLUMN);
            html.append("</tr>\n");
        }

        html.append("<tr>\n\
                            <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"left\"><b>User Lines </b></td>");
        html.append(HTML_VIEW_LINES_EMPTY_COLUMN);
        html.append(HTML_VIEW_LINES_EMPTY_COLUMN);
        html.append(HTML_VIEW_LINES_EMPTY_COLUMN);
        html.append(HTML_VIEW_LINES_EMPTY_COLUMN);
        html.append("</tr>\n");

        //BAd way of doing it
        lineMgr->getDefaultOutboundLine(currentOutboundLine);
        i=0;
        while ( i < returnVal )
        {
            if ( lines[i].getUser().compareTo("Device", UtlString::ignoreCase) != 0 )
            {
                html.append("<tr>\n") ;

                Url lineUrl = lines[i].getUserEnteredUrl();
                Url identityUri = lines[i].getIdentity();
                Url canonicalUrl = lines[i].getCanonicalUrl();
                int lineState = lines[i].getState();
                UtlString User = lines[i].getUser();
                UtlBoolean callHandling = lines[i].getCallHandling();

                //have to escape URL so it appears in page correctly
                strLineUrl.append(lineUrl.toString()) ;
                Webutil::escapeChars(strLineUrl) ;

                //line state
                if ( lineState == SipLine::LINE_STATE_PROVISIONED )
                {
                    strlineState.append("Provision");
                } else //if ( lineState == SipLine::LINE_STATE_PROVISIONED)
                {
                    strlineState.append("Register");
                }
                Webutil::escapeChars(strlineState) ;

                //call handling
                if ( callHandling )
                    strLineCallHandling.append("Enabled");
                else
                    strLineCallHandling.append("Disabled");

                Webutil::escapeChars(strLineCallHandling) ;

                //outbound line
                if ( canonicalUrl.toString().compareTo(currentOutboundLine) == 0 )
                {
                    strOutboundLine.remove(0);
                    strOutboundLine.append("CHECKED");
                } else
                {
                    strOutboundLine.remove(0);
                }

                Webutil::escapeChars(strOutboundLine) ;

                // Line editing button
                UtlString strEditUrl ;
                UtlString strParam ;

                strEditUrl.append("?URL=") ;
                strParam.append(lineUrl.toString()) ;
                HttpMessage::escape(strParam) ;
                strEditUrl.append(strParam) ;
                strParam.remove(0) ;
                strEditUrl.append("&METHOD=EDIT") ;

                // Add Sip Url
                html.append("<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"left\"><p>") ;
                html.append(strLineUrl) ;
                html.append("</p></td>\n") ;

                // Add Call Handling
                html.append("<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p>") ;
                html.append(strLineCallHandling) ;
                html.append("</p></td>\n") ;

                //Add Line Registration State
                html.append("<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p>") ;
                html.append(strlineState) ;
                html.append("</p></td>\n") ;

                // Add OutBound
                html.append("<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p>\n\
                    <input type=\"radio\" name=\"isOutBound\" value= \" ");
                html.append(strLineUrl);
                html.append(" \" ");
                html.append(strOutboundLine) ;
                html.append(" > ") ;
                html.append("</p></td>\n") ;

                // Add Line Edit link
                html.append("<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"middle\"><p>") ;
                html.append("<a href=\"/cgi/addLines.cgi") ;
                html.append(strEditUrl.data()) ;
                html.append("\">Edit</a></p></td>\n") ;

                html.append("</tr>") ;

                strLineUrl.remove(0) ;
                strlineState.remove(0);
                strLineCallHandling.remove(0);
                strOutboundLine.remove(0);
            }
            i++ ;
        }
        delete [] lines ;

        //add empty line for adding new line
        html.append("<tr>\n") ;
        // Add Sip Url
        html.append("<td bgcolor=\"#FFCC33\" valign=\"top\" align=\"left\">\n\
            <p><a href=\"/cgi/addLines.cgi?User=USER\">Add New Line</a></p></td>\n") ;
        html.append(HTML_VIEW_LINES_EMPTY_COLUMN);
        html.append(HTML_VIEW_LINES_EMPTY_COLUMN);
        html.append(HTML_VIEW_LINES_EMPTY_COLUMN);
        html.append(HTML_VIEW_LINES_EMPTY_COLUMN);
        html.append("</tr>\n");
        addToHtmlString.append(html);
    }

}


void WebuiLineMgrUtil::saveAddLines(UtlString& strLineUrl,
                                    UtlString& strLineCallHandling,
                                    UtlString& strLineState,
                                    UtlString& ReqLineUser,
                                    UtlString& statusMessage,
                                    UtlBoolean& redirectToViewLine,
                                    UtlBoolean& isErrorAndStayOnSamePage)
{
    // user is submitting a new Line
    if ( Webutil::isValidSipUrl(strLineUrl) )
    {
        UtlBoolean callhandling = TRUE;
        int lineState = SipLine::LINE_STATE_UNKNOWN;

        strLineCallHandling.compareTo("Enabled" , UtlString::ignoreCase) == 0 ? callhandling =TRUE: callhandling = FALSE;
        if ( strLineState.compareTo("Register" , UtlString::ignoreCase) == 0 )
        {
            lineState = SipLine::LINE_STATE_REGISTERED;
        } else //( strLineState.compareTo("Provision" , UtlString::ignoreCase) == 0)
        {
            lineState = SipLine::LINE_STATE_PROVISIONED;
        }

        Pinger* pinger = Pinger::getPingerTask();
        SipLineMgr* lineMgr = pinger->getLineManager() ;
        if ( pinger && lineMgr )
        {
            UtlString hostAddress;
            UtlBoolean isRegistringToItselfFlag = FALSE;
            Url url(strLineUrl);
            Url uri;
            getUriForUrl(url , uri);

            SipLine line(url, uri, ReqLineUser, TRUE, lineState,TRUE, callhandling);

            //check if it is not trying to register to its own host and port
            if ( lineState == SipLine::LINE_STATE_REGISTERED )
            {
                Url uri = line.getIdentity();
                isRegistringToItselfFlag =  isRegistringToItself(uri);
            }
            if ( !isRegistringToItselfFlag )
            {
                redirectToViewLine = TRUE;
                if ( lineMgr->addLine(line) )
                {
                    statusMessage.append("New line added");
                } else
                {
                    statusMessage.append("New line was NOT added because an existing line has the same Uri."
                                         " To change line properties click on edit in the Edit Line tab.");
                    HttpMessage::escape(statusMessage);
                }
                SipLineMgrSerialize(*lineMgr, true) ;
                redirectToViewLine = TRUE;

            } else
            {
                isErrorAndStayOnSamePage = TRUE;
                statusMessage.append("Registration behavior incorrect:"
                                     " A line cannot register to its own IP address and listening port."
                                     " Please change the registration behavior to Provisioned.");
                HttpMessage::escape(statusMessage);
                statusMessage.append("&USER=");
                HttpMessage::escape(ReqLineUser) ;
                statusMessage.append(ReqLineUser);

            }
        }
    } else
    {
        isErrorAndStayOnSamePage = TRUE;
        statusMessage.append("Invalid Url specified");
        HttpMessage::escape(statusMessage);
        statusMessage.append("&USER=");
        HttpMessage::escape(ReqLineUser) ;
        statusMessage.append(ReqLineUser);
    }
}

void
WebuiLineMgrUtil::deleteAddLines(
                                UtlString& sipUrl,
                                UtlString& statusMessage)
{
    Pinger* pinger = Pinger::getPingerTask();
    CallManager * callMgr = pinger->getCallManager();
    if ( callMgr )
    {
        UtlString callIdArray[MAX_CALLS];
        int numCalls = 0;
        callMgr->getCalls(MAX_CALLS, numCalls, callIdArray);
        if ( numCalls <= 0 ) //there are no active calls
        {
            Url thisUrl(sipUrl);
            Url identity;
            Url canonicalUrl;
            getUriForUrl(thisUrl , identity);
            getCanonicalUrl(thisUrl , identity, canonicalUrl);
            SipLineMgr* lineMgr = pinger->getLineManager() ;
            if ( pinger && lineMgr )
            {
                if ( lineMgr->getNumLines() > 1 )
                {
                    lineMgr->deleteLine(identity);
                    UtlString outbound;
                    lineMgr->getDefaultOutboundLine(outbound);
                    if ( outbound.compareTo(canonicalUrl.toString()) == 0 )
                    {
                        lineMgr->setFirstLineAsDefaultOutBound();
                        lineMgr->getDefaultOutboundLine(outbound);
                        callMgr->setOutboundLine(outbound);
                        statusMessage.append(
                                            "Line deleted. The deleted line was the designated Call Out As line. "
                                            "As a result, another line has been designated as the Call Out As line. Verify and change this setting as needed.");
                        HttpMessage::escape(statusMessage);
                    } else
                    {
                        statusMessage.append("Line was deleted");
                        HttpMessage::escape(statusMessage);
                    }
                } else
                {
                    statusMessage.append("Cannot delete. At least one line must be present to identify this phone.");
                    HttpMessage::escape(statusMessage);
                }
                SipLineMgrSerialize(*lineMgr, true) ;
            }
        } else
        {
            statusMessage.append("Cannot delete line while there are active calls on the phone");
            HttpMessage::escape(statusMessage);
        }
    }
}

// Hack to work around internal compiler error in g++ 2.9 with -O2
void WebuiLineMgrUtil::makeStringsHack(UtlString*& rpRealm, UtlString*& rpType,
   UtlString*& rpUserId, UtlString*& rpPassToken, int noOfCredentials)
{
   rpRealm = new UtlString[noOfCredentials];
   rpType = new UtlString[noOfCredentials];
   rpUserId = new UtlString[noOfCredentials];
   rpPassToken = new UtlString[noOfCredentials];
}


void WebuiLineMgrUtil::updateAddLines(UtlString& sipUrl,
                                      UtlString& previousSipUrl,
                                      UtlString& strLineCallHandling,
                                      UtlString& strIsStunEnabled,
                                      UtlString& strLineState,
                                      UtlBoolean& isErrorAndStayOnSamePage,
                                      UtlBoolean& redirectToViewLine,
                                      UtlString& statusMessage)
{
    Pinger* pinger = Pinger::getPingerTask();
    CallManager * callMgr = pinger->getCallManager();
    if ( callMgr )
    {
        UtlString callIdArray[MAX_CALLS];
        int numCalls = 0;
        callMgr->getCalls(MAX_CALLS, numCalls, callIdArray);
        if ( numCalls <= 0 ) //there are no active calls
        {
            UtlString thisUri;
            UtlString previousUri;
            UtlBoolean callhandling;
            int lineState;
            UtlBoolean isRegistringToItselfFlag = FALSE;
            LINE_CONTACT_TYPE eContactType ;

            if ( Webutil::isValidSipUrl(sipUrl) )
            {
                strLineCallHandling.compareTo("Enabled" , UtlString::ignoreCase) == 0 ? callhandling =TRUE: callhandling = FALSE;
                if ( strLineState.compareTo("Register" , UtlString::ignoreCase) == 0 )
                {
                    lineState = SipLine::LINE_STATE_REGISTERED;
                } else //( strLineState.compareTo("Provision" , UtlString::ignoreCase) == 0)
                {
                    lineState = SipLine::LINE_STATE_PROVISIONED;
                }

                if (strIsStunEnabled.compareTo("Enabled" , UtlString::ignoreCase) == 0)
                {
                    eContactType = LINE_CONTACT_NAT_MAPPED ;
                }
                else
                {
                    eContactType = LINE_CONTACT_LOCAL ;
                }

                Url previousUrl(previousSipUrl);
                Url previousUri;
                Url previousCanonicalUrl;
                getUriForUrl(previousUrl, previousUri);
                getCanonicalUrl(previousUrl, previousUri, previousCanonicalUrl);

                Url thisUrl(sipUrl);
                Url thisUri;
                Url thisCanonicalUrl;
                getUriForUrl(thisUrl, thisUri);
                getCanonicalUrl(thisUrl, thisUri, thisCanonicalUrl);

                if ( (lineState != SipLine::LINE_STATE_REGISTERED) ||
                     (lineState == SipLine::LINE_STATE_REGISTERED &&
                      (isRegistringToItselfFlag =  isRegistringToItself(thisUri)) != TRUE) )
                {
                    Pinger* pinger = Pinger::getPingerTask();
                    SipLineMgr* lineMgr = pinger->getLineManager() ;
                    if ( pinger && lineMgr )
                    {

                        //check if Sip Url has changed
                        if ( thisUri.toString().compareTo(previousUri.toString()) == 0 )
                        {
                            if ( sipUrl.compareTo(previousSipUrl) != 0 )
                            {
                                //then the complete Url has changed
                                // but uri is same
                                lineMgr->setUserEnteredUrlForLine(previousUri, sipUrl);
                            }
                            lineMgr->setCallHandlingForLine(previousUri, callhandling);
                            lineMgr->setContactTypeForLine(previousUri, eContactType) ;
                            lineMgr->setStateForLine(previousUri, lineState);                            
                        } else
                        {
                            UtlString User;
                            lineMgr->getUserForLine(previousUri, User);
                            if ( User.compareTo("Device", UtlString::ignoreCase ) != 0 )
                            {
                                User.remove(0);
                                User.append("User");
                            }

                            SipLine updatedLine(thisUrl, thisUri, User, TRUE,
                               lineState, TRUE, callhandling);
                            UtlString outBoundLine;
                            lineMgr->getDefaultOutboundLine(outBoundLine);
                            updatedLine.setContactType(eContactType) ;

                            int noOfCredentials =
                               lineMgr->getNumOfCredentialsForLine(previousUri);

      //////////////////////////////////////////////////////////////////////////
      // Hack to work around internal compiler error in g++ 2.9 with -O2,
      //  which happened when these were in-line here:
      //
      //                    UtlString *realm = new UtlString[noOfCredentials];
      //                    UtlString *type = new UtlString[noOfCredentials];
      //                    UtlString *userId = new UtlString[noOfCredentials];
      //                    UtlString *passToken = new UtlString[noOfCredentials];
      // Call out-of-line private static method to do the same thing...
      //////////////////////////////////////////////////////////////////////////
                            //delete old line and add a new line with different
                            // sipUrl but the same set of credentials and other
                            // properties
                            UtlString *realm;
                            UtlString *type;
                            UtlString *userId;
                            UtlString *passToken;

                            WebuiLineMgrUtil::makeStringsHack(realm,
                               type, userId, passToken, noOfCredentials);
      //////////////////////////////////////////////////////////////////////////

                            int retVal = 0;
                            int i = 0;
                            lineMgr->getCredentialListForLine(previousUri,
                               noOfCredentials , retVal, realm, userId,
                               type, passToken);
                            for ( i =0 ; i< retVal ; i++ )
                            {
                                updatedLine.addCredentials(realm[i],
                                   userId[i], passToken[i], type[i]);
                            }
                            if (retVal > 0)
                            {
                                delete [] realm;
                                delete [] type;
                                delete [] userId;
                                delete [] passToken;
                            }
                            //add line credetials to new identity
                            if ( lineMgr->addLine(updatedLine) )
                            {
                                lineMgr->deleteLine(previousUri);
                                if ( outBoundLine.compareTo(previousCanonicalUrl.toString()) == 0 )
                                {
                                    lineMgr->setDefaultOutboundLine(thisCanonicalUrl);
                                    CallManager * callMgr =
                                                   pinger->getCallManager();
                                    if ( callMgr )
                                    {
                                        callMgr->setOutboundLine(thisCanonicalUrl.toString());
                                    }
                                    callMgr = NULL;
                                }
                            } else
                            {
                                isErrorAndStayOnSamePage = TRUE;
                                statusMessage.append(
                                   "Line was NOT updated because an existing"
                                   " line has the same Uri.");
                                HttpMessage::escape(statusMessage);
                                statusMessage.append("&METHOD=EDIT&URL=");
                                HttpMessage::escape(previousSipUrl) ;
                                statusMessage.append(previousSipUrl);
                            }
                        }

                        if ( !isErrorAndStayOnSamePage )
                        {
                            lineMgr->notifyChangeInLineProperties(thisUri);
                            statusMessage.append("Updated line properties.");
                            HttpMessage::escape(statusMessage);
                        }
                    }
                    SipLineMgrSerialize(*lineMgr, true) ;
                    lineMgr = NULL;
                    pinger = NULL;
                    if ( !isErrorAndStayOnSamePage )
                        redirectToViewLine = TRUE;
                } else
                {
                    isErrorAndStayOnSamePage = TRUE;
                    statusMessage.append("Registration behavior incorrect:A"
                       " line cannot register to its own IP address and"
                       " listening port. Please change the registration"
                       " behavior to Provisioned.");
                    HttpMessage::escape(statusMessage);
                    statusMessage.append("&METHOD=EDIT&URL=");
                    HttpMessage::escape(previousSipUrl) ;
                    statusMessage.append(previousSipUrl);
                }
            } else
            {
                isErrorAndStayOnSamePage = TRUE;
                statusMessage.append("Invalid Url specified.");
                HttpMessage::escape(statusMessage);
                statusMessage.append("&METHOD=EDIT&URL=");
                HttpMessage::escape(previousSipUrl) ;
                statusMessage.append(previousSipUrl);
            }
        } else
        {
            redirectToViewLine = TRUE;
            statusMessage.append("Cannot update line properties while there"
               " are active calls on the phone.");
            HttpMessage::escape(statusMessage);
        }
    }
}

void WebuiLineMgrUtil::editAddLines(
                                   UtlString& userName,
                                   UtlString& ReqSipUrl,
                                   UtlString& addToHtml,
                                   UtlBoolean& isAuthorized,
                                   UtlBoolean& redirectToViewLine,
                                   UtlString& statusMessage)
{
    UtlString html;
    Pinger* pinger = Pinger::getPingerTask();
    CallManager * callMgr = pinger->getCallManager();
    int noOfCredentials = 0;

    if ( callMgr )
    {
        UtlString callIdArray[MAX_CALLS];
        int numCalls = 0;
        callMgr->getCalls(MAX_CALLS, numCalls, callIdArray);
        if ( numCalls <= 0 ) //there are no active calls
        {
            Webutil::unescapeChars(ReqSipUrl);
            Url sipUrl(ReqSipUrl);
            Url identity;
            getUriForUrl(sipUrl, identity);
            UtlString currentUser;
            Pinger* pinger = Pinger::getPingerTask();
            SipLineMgr* lineMgr = pinger->getLineManager() ;
            if ( pinger && lineMgr )
            {
                if ( lineMgr->getUserForLine(identity , currentUser) )
                {
                    if ( (currentUser.compareTo("device" , UtlString::ignoreCase) == 0) && userName.compareTo("admin")!=0 )
                    {
                        // Check user is "admin," if not redirect to not authorized page
                        isAuthorized = FALSE;
                    } else
                    {

                        UtlString tempUrl(ReqSipUrl);
                        Webutil::escapeChars(tempUrl);
                        //If editing, keep edit Method
                        html.append("<input type=\"hidden\" name=\"METHOD\" value=\"EDIT\">") ;
                        html.append("<input type=\"hidden\" name=\"PreviousIdentity\" value=\"") ;
                        html.append(tempUrl) ;
                        html.append("\">") ;
                        //Also append ID as hidden parameter
                        html.append("<input type=\"text\" name=\"identity\" size=\"70\" value=\"") ;
                        html.append(tempUrl) ;
                        html.append("\">") ;

                        // Stun
                        html.append(HTML_ADD_LINE_STUN);
                        LINE_CONTACT_TYPE eContactType = LINE_CONTACT_NAT_MAPPED ;
                        lineMgr->getContactTypeForLine(identity, eContactType) ;
                        if (eContactType == LINE_CONTACT_NAT_MAPPED)
                        {
                            html.append("CHECKED");
                            html.append(HTML_ADD_LINE_STUN_END);
                        }
                        else
                        {                            
                            html.append(HTML_ADD_LINE_STUN_END);
                            html.append("CHECKED");
                        }                        
                                               
                        // Call Handling
                        html.append(HTML_ADD_LINE_CALL_HANDLING);
                        UtlBoolean CallHandling = lineMgr->getCallHandlingForLine(identity);
                        if ( CallHandling )
                        {
                            html.append("CHECKED");
                            html.append(HTML_ADD_LINE_CALL_HANDLING_END);
                        } else
                        {
                            html.append(HTML_ADD_LINE_CALL_HANDLING_END);
                            html.append("CHECKED");
                        }                        

                        // Registration behaviour
                        html.append(HTML_ADD_LINE_AFTERCALLHANDLING);
                        int lineState = lineMgr->getStateForLine(identity);
                        if ( lineState == SipLine::LINE_STATE_PROVISIONED )
                        {
                            html.append(HTML_ADD_LINE_AFTERREGISTRATION);
                            html.append("CHECKED");
                        } else
                        {
                            html.append("CHECKED");
                            html.append(HTML_ADD_LINE_AFTERREGISTRATION);
                        }

                        html.append(HTML_ADD_LINE_AFTERLINEREGISTRATION);
                        //add link to add credential button
                        html.append("<p><a href=/cgi/addCredentials.cgi?");  // beforeCredentialButton
                        html.append("METHOD=ADD&URL=");
                        HttpMessage::escape(ReqSipUrl);
                        html.append(ReqSipUrl);
                        html.append(">Add Credentials</a></p> ");    //  afterCredentialAddButton);
                        html.append(HTML_ADD_LINE_CREATECREDENTIALTABLE);

                        noOfCredentials = lineMgr->getNumOfCredentialsForLine(identity);

                        UtlString *realm = new UtlString[noOfCredentials];
                        UtlString *type = new UtlString[noOfCredentials];
                        UtlString *userId = new UtlString[noOfCredentials];
                        UtlString *passToken = new UtlString[noOfCredentials];
                        int retVal = 0;
                        int i = 0;

                        lineMgr->getCredentialListForLine(identity, noOfCredentials , retVal, realm, userId, type, passToken);
                        UtlString htmlTmp;
                        while ( i < retVal )
                        {
                            //get line credetials
                            htmlTmp.append("<tr>\n\
                                           <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"left\"> ");   // beginCredentialRow);
                            htmlTmp.append(realm[i].data());
                            htmlTmp.append("</td>\n\
                                     <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"left\"> "); //  afterRealmCredential);
                            htmlTmp.append(userId[i].data());
                            htmlTmp.append("</td>\n\
                                      <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"left\"> ");    //  afterUserIdCredential);

                            //create URL for Edit
                            htmlTmp.append("<p><a href=/cgi/addCredentials.cgi?");   // );
                            htmlTmp.append("METHOD=EDIT&URL=");
                            htmlTmp.append(ReqSipUrl);
                            htmlTmp.append("&REALM=");
                            HttpMessage::escape(realm[i]);
                            htmlTmp.append(realm[i].data());
                            htmlTmp.append("&USERID=");
                            HttpMessage::escape(userId[i]);
                            htmlTmp.append(userId[i].data());
                            htmlTmp.append(">Edit</a></p> ");    //  afterCredentialEditButton);

                            //end row
                            htmlTmp.append("</td>\n\
                                    <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"left\"> ");  //  afterEditCredential);
                            htmlTmp.append("</td>\n\
                                    </tr> ");   //  endCredentialRow);
                            i++;

                        }
                        if (noOfCredentials > 0)
                        {
                            delete [] realm;
                            delete [] userId;
                            delete [] type;
                            delete [] passToken;
                        }

                        html.append(htmlTmp.data());

                        html.append("</table>");
                        html.append("</td>\n\
                                     </tr> \n\
                                      <tr>\n\
                                      <td bgcolor=\"#FFCC33\" valign=\"top\" align=\"centre\"> ") ; //  afterCredentials);
                    }
                }
            }
        } else
        {
            redirectToViewLine = TRUE;
            statusMessage.append("Cannot edit lines while there are active one or more calls on the phone.");
            HttpMessage::escape(statusMessage);
        }
    }
    addToHtml.append(html);
}

void WebuiLineMgrUtil::saveAddCredentials(UtlString& sipUrl,
                                          UtlString& strUserId,
                                          UtlString& strRealm,
                                          UtlString& strPassword,
                                          UtlString& strConfirmPassword,
                                          UtlString& strErrorMsg,
                                          UtlString& statusMessage,
                                          UtlBoolean& isCredentialAdded,
                                          UtlBoolean& redirectToAddLine)


{

    //check if all the fields are filled or not
    if ( strRealm.isNull() ||
         strPassword.isNull() ||
         strConfirmPassword.isNull() )
    {
        strErrorMsg.append("<p><font color=\"Red\">Please fill in all the fields</font></p>");
    } else if ( strConfirmPassword.compareTo(strPassword) == 0 && !strConfirmPassword.isNull() )
    {
        Pinger* pinger = Pinger::getPingerTask();
        SipLineMgr* lineMgr = pinger->getLineManager() ;
        if ( pinger && lineMgr )
        {
            //convert password to digest
            UtlString passwordToken;
            HttpMessage::buildMd5UserPasswordDigest(strUserId, strRealm, strPassword, passwordToken);

            Url thisSipurl(sipUrl);
            Url thisUri;
            getUriForUrl(thisSipurl , thisUri);

            if ( lineMgr->addCredentialForLine(thisUri, strRealm, strUserId, passwordToken, HTTP_DIGEST_AUTHENTICATION) )
            {
                isCredentialAdded = TRUE;
                int previousLineSate = lineMgr->getStateForLine(thisUri);
                if ( previousLineSate == SipLine::LINE_STATE_FAILED || previousLineSate == SipLine::LINE_STATE_EXPIRED )
                {
                    //retry again with new credentials
                    lineMgr->enableLine(thisUri);
                }
                statusMessage.append("New credentials added for the Line.");
                SipLineMgrSerialize(*lineMgr, true) ;
                redirectToAddLine = TRUE;
            } else
            {
                strErrorMsg.append("<p><font color=\"Red\">Duplicate Realm value or could not find Line.\
                    (It might have been deleted from other sources). Please refresh view linges page</font></p>");
            }
        }//end if piger and line mgr
    }//if valid passwords
    else
    {
        strErrorMsg.append("<p><font color=\"Red\">Passwords are Invalid : Re-enter Passwords.</font></p>");
    }
}

void WebuiLineMgrUtil::deleteAddCredentials(UtlString& sipUrl,
                                            UtlString& strRealm,
                                            UtlString& statusMessage)

{
    Pinger* pinger = Pinger::getPingerTask();
    SipLineMgr* lineMgr = pinger->getLineManager() ;
    if ( pinger && lineMgr )
    {
        Url thisSipurl(sipUrl);
        Url thisUri;
        getUriForUrl(thisSipurl , thisUri);

        if ( lineMgr->deleteCredentialForLine(thisUri, strRealm) )
        {
            statusMessage.append("Credential deleted.");
            SipLineMgrSerialize(*lineMgr, true) ;

        } else
        {
            statusMessage.append("Could not find Line. (It might have been deleted from other sources). Please go to View Lines page and refresh the page");
        }
    }

}
UtlBoolean WebuiLineMgrUtil::updateAddCredentials( UtlString& sipUrl,
                                                  UtlString& sipRealm,
                                                  UtlString& sipUserId,
                                                  UtlString& strRealm,
                                                  UtlString& strUserId,
                                                  UtlString& strPassword,
                                                  UtlString& strConfirmPassword,
                                                  UtlString& statusMessage,
                                                  UtlBoolean& redirectToAddLine )

{
    UtlBoolean isValidData = FALSE;
    //check if paswwords confirm
    if ( strConfirmPassword.compareTo(strPassword) == 0
         && !strConfirmPassword.isNull()
         && !strRealm.isNull()
         && !strUserId.isNull() )
    {
        Pinger* pinger = Pinger::getPingerTask();
        SipLineMgr* lineMgr = pinger->getLineManager() ;
        if ( pinger && lineMgr )
        {
            Url thisSipurl(sipUrl);
            Url thisUri;
            getUriForUrl(thisSipurl , thisUri);
            if ( lineMgr->deleteCredentialForLine(thisUri, sipRealm) )
            {
                //convert password to digest
                UtlString passwordToken;
                HttpMessage::buildMd5UserPasswordDigest(strUserId, strRealm, strPassword, passwordToken);

                Url identity(sipUrl);
                if ( lineMgr->addCredentialForLine(thisUri, strRealm, strUserId, passwordToken, HTTP_DIGEST_AUTHENTICATION) )
                {
                    statusMessage.append("Credentials updated.");
                    SipLineMgrSerialize(*lineMgr,true) ;
                    //try registering with the new credentials if line registering has faile or expired before.
                    int lineState = lineMgr->getStateForLine(thisUri) ;
                    if ( lineState == SipLine::LINE_STATE_FAILED || lineState == SipLine::LINE_STATE_EXPIRED )
                    {
                        lineMgr->enableLine(thisUri);
                    }
                    redirectToAddLine = TRUE;
                }
            } else
            {
                statusMessage.append("Could not find Line.(It might have been deleted from other sources). Please go to View Lines page and refresh the page");
            }
        } else
        {
            statusMessage.append("Corrupt Line manager or Pinger. Please reboot the phone");
        }

        isValidData = TRUE;
    }
    return isValidData ;
}


void WebuiLineMgrUtil::getUriForUrl(Url &userEnteredUrl , Url &uri)
{
    UtlString host;
    userEnteredUrl.getHostAddress(host);
    if ( host.isNull() )
    {
        //dynamic IP address
        Pinger* pinger = Pinger::getPingerTask();
        SipUserAgent* UA = pinger->getSipUserAgent();
        if ( pinger && UA )
        {
            uri = userEnteredUrl;
            UtlString contact;
            UtlString contactHost;
            UA->getContactUri(&contact);
            Url contactUri(contact);
            contactUri.getHostAddress(contactHost);
            int contactPort = contactUri.getHostPort();
            //set correct value of host and port
            uri.setHostAddress(contactHost);
            uri.setHostPort(contactPort);
        }
    } else
    {
        //just get uri from user entered url
        UtlString identityUri;
        userEnteredUrl.getUri(identityUri);
        uri = Url(identityUri);
    }
}

void WebuiLineMgrUtil::getCanonicalUrl(Url &userEnteredUrl , Url &uri , Url &canonicalUrl/*out*/)
{
    //construct a complete url from identityUri and userEntered Url.
    canonicalUrl = userEnteredUrl;
    UtlString address;
    userEnteredUrl.getHostAddress(address);
    if ( address.isNull() )
    {
        UtlString identityHost;
        uri.getHostAddress(identityHost);
        int identityPort = uri.getHostPort();
        canonicalUrl.setHostAddress(identityHost);
        canonicalUrl.setHostPort(identityPort);
    }
}

UtlBoolean
WebuiLineMgrUtil::isRegistringToItself(Url sipUri)
{
    // get the phone's contact uri from sip user agent
    // we need to compare this against the line being registered
    int userAgentPort;
    UtlString userAgentHost, userAgentProtocol, userAgentUser, contactUri;
    Pinger* pinger = Pinger::getPingerTask();
    SipUserAgent* userAgent = pinger->getSipUserAgent();
    if ( pinger && userAgent )
    {
        userAgent->getContactUri(&contactUri);

        SipMessage::parseAddressFromUri(
           contactUri, &userAgentHost,
           &userAgentPort, &userAgentProtocol, &userAgentUser);

        if ( userAgentPort == 0 )
            userAgentPort = SIP_PORT;
    }

    // Get local address and port - this might be different
    // than contact address field
    UtlString localHost;
    OsSocket::getHostIp(&localHost);

    // Register line's Uri's host and port
    UtlString uriHost;
    int uriPort;
    sipUri.getHostAddress( uriHost );
    uriPort = sipUri.getHostPort();
    if ( uriPort == 0 )
        uriPort = SIP_PORT;

    // compare values, @JC Simplfified this and fixed bug where
    // uri port and useragent port are different this is valid
    // the old code assumed that the phone ran on SIP_PORT making
    // the second else if comopare as true
    if ( uriHost.compareTo(userAgentHost, UtlString::ignoreCase) == 0 &&
         uriPort == userAgentPort )
    {
        return TRUE;
    }
    /* @JC Not needed else if ( uriHost.compareTo(localHost, UtlString::ignoreCase) == 0 &&
                uriPort == SIP_PORT )
    {
        return TRUE;
    }
    */
    else
    {
        return FALSE;
    }
}
