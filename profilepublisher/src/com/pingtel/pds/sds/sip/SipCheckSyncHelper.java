/*
 * 
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

import com.pingtel.pds.sds.sip.SipMessage;
import com.pingtel.pds.sds.sip.SipSession;
import com.pingtel.pds.sds.SipConfigServerAgent;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author
 * @version 1.0
 */

public class SipCheckSyncHelper {

    private static SipCheckSyncHelper g_instance = null;

    /** Singleton Private constructor */
    private SipCheckSyncHelper() {}

    /** Singleton */
    public static SipCheckSyncHelper getInstance() {
        if (g_instance == null) {
            g_instance = new SipCheckSyncHelper();
        }
        return g_instance;
    }

    /** Sends the SIP Check Sync Message */
    public void sendCheckSync ( String toAddress,
                                String fromAddress,
                                String callId,
                                String contactAddress,
                                int sequenceNumber ) throws IOException {
        // Create an empty Sip body message and set the fields explicitly
        SipMessage message = new SipMessage();

        // The check-sync message is a flavor of unsolicited NOTIFY
        // this message does not require that the phone be enrolled
        // the message allows us to reboot a specific phone
        // Format a notify URL, rules are, if there are < > in the URL, strip
        // these and use their contents as the URI, else if the from URL
        // contains a ; strip everything following it
        String notifyUri = toAddress;
        if ( (toAddress.indexOf('<') >= 0) && (toAddress.indexOf('>') > 0) ) {
            notifyUri = toAddress.substring(toAddress.indexOf('<')+1, toAddress.indexOf('>'));
        } else if ( toAddress.indexOf(';') >= 0 ) {
            notifyUri = toAddress.substring(0, toAddress.indexOf(';'));
        }

        message.setRequestHeader( "NOTIFY", notifyUri, "SIP/2.0" );

        // This field must match the phone's IP Address or label (Cisco)
        message.setHeaderFieldValue( SipMessage.SIP_TO_ADDRESS, toAddress);

        // This should represent the SDS's IP Address or in the case
        // of using a proxy this should be the proxy addresss
        message.setHeaderFieldValue( SipMessage.SIP_FROM_ADDRESS, fromAddress );

        // This is something I can make up
        message.setHeaderFieldValue( SipMessage.SIP_CALL_ID, callId );

        // the sequence number
        message.setHeaderFieldValue( SipMessage.SIP_CSEQ, sequenceNumber + " NOTIFY");

        // Contact Address
        message.setHeaderFieldValue( SipMessage.SIP_CONTACT, contactAddress );

        // Set the all important event type
        message.setHeaderFieldValue( SipMessage.SIP_EVENT, "check-sync" );

        // Now talk to the device via sip
        SipConfigServerAgent.getInstance().sendMessage(message);
    }
}
