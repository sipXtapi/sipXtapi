/*
 * $Id: //depot/OPENDEV/sipXconfig/profilepublisher/src/com/pingtel/pds/sds/sip/SipNotifyHelper.java#6 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.sds.sip;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.StringTokenizer;

import org.apache.log4j.Category;

import com.pingtel.pds.common.DeviceNameUtil;
import com.pingtel.pds.common.ErrorMessageBuilder;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.sds.SipConfigServerAgent;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author
 * @version 1.0
 */

public class SipNotifyHelper {
    private static SipNotifyHelper g_instance = null;

    /** private constructor for the singleton design pattern */
    private SipNotifyHelper() {}

    Category m_logger = Category.getInstance( "pds" );

    /** Singleton Accessor */
    public static SipNotifyHelper getInstance() {
        if (g_instance == null)
            g_instance = new SipNotifyHelper();
        return g_instance;
    }

    /**
     * Sends a generic notification to the phone.  Typically this is in response
     * to a subscribe from the phone.  This notification is also sent down
     * as a result of projection from the ProfileWriter.  Depending on
     * what is projected several or individual configuration URLs may be
     * sent to the phone along with corresponding sequence numbers
     */
    public void notify ( String macAddress,
                         String deviceURL,
                         String fromURL,
                         String callID,
                         int outputCSeq,
                         String documentRoot,
                         ArrayList profileURLS )
        throws SipException {
        // Using the Mac Address and the URLs' notify
        // the phone of the URL(s), notice the reversal of
        // the From: and the To: fields
        // Format a notify URL, rules are, if there are < > in the URL, strip
        // these and use their contents as the URI, else if the from URL
        // contains a ; strip everything following it
        String notifyUri = fromURL;
        if ( (fromURL.indexOf('<') >= 0) && (fromURL.indexOf('>') > 0) ) {
            notifyUri = fromURL.substring(fromURL.indexOf('<')+1, fromURL.indexOf('>'));
        } else if ( fromURL.indexOf(';') >= 0 ) {
            notifyUri = fromURL.substring(0, fromURL.indexOf(';'));
        }

        StringBuffer headerBody = new StringBuffer("NOTIFY " + notifyUri + " SIP/2.0\r\n");
        headerBody.append("From: " + deviceURL + "\r\n");
        headerBody.append("To: " + fromURL + "\r\n");
        headerBody.append("Call-Id: " + callID + "\r\n");
        headerBody.append("Cseq: " + outputCSeq + " NOTIFY\r\n");
        headerBody.append("Content-Type: text/plain\r\n");
        headerBody.append("Event: sip-config\r\n");

        StringBuffer bodyBody = new StringBuffer();

        // @JC No longer need to have profileURL's in which case
        // an empty body response will be generated
        if ( profileURLS != null ) {
            for ( Iterator it = profileURLS.iterator(); it.hasNext(); ) {
                  SipProfileInfo profileInfo = (SipProfileInfo)it.next();
                  String url = documentRoot + profileInfo.getURL();

                // this is the test to check whether the profile is valid
                // and has a sequence number that is correct
                if ( profileInfo.getProfileSequenceNumber() >= 0) {
                    String profileToken = DeviceNameUtil.getInstance().
                        getDeviceProfileToken ( profileInfo.getProfileType(), "Pingtel" );
                    bodyBody.append(
                        profileToken + ": " + "Sequence=" +
                        profileInfo.getProfileSequenceNumber() +
                        ",url=" + documentRoot + // comma used to be ; - kluged for 2.0
                        profileInfo.getURL() + "\r\n");
                }
            }
        }
        try {
            headerBody.append("Content-Length: " + bodyBody.length() + "\r\n\r\n");
            SipMessage msg = new SipMessage( headerBody.toString() + bodyBody.toString() );
            java.util.Date d1 = new java.util.Date();
            SipConfigServerAgent.getInstance().sendMessage(msg);
            java.util.Date d2 = new java.util.Date();
            m_logger.debug ( "TRACE: it took: " + (d2.getTime() - d1.getTime()) + " ms to send SIP NOTIFY MESSAGE");
        } catch (IOException ex) {
            // Error Communicating with Device
            m_logger.error( "error in notify: " + ex.toString(), ex );
            throw new SipException(ex.getMessage());
        }
    }
}