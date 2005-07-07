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


package com.pingtel.pds.sds.cmd;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Iterator;
import java.util.Collection;
import java.io.IOException;
import com.pingtel.pds.sds.sip.SipCheckSyncHelper;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author
 * @version 1.0
 */

public class CiscoReceiver {
    /**
     * This field is probably overkill but incrmement
     *  this number every time we need to talk to a phone
     */
    private int m_sequenceNumber = 1001;

    /** Singleton */
    private static CiscoReceiver g_instance = null;

    /** Singletons have private constructors */
    private CiscoReceiver () {}

    /** singleton Accessor */
    public static CiscoReceiver getInstance() {
        if (g_instance == null) {
            g_instance = new CiscoReceiver ();
        }
        return g_instance;
    }

    /**
     * Resets the Phone, this method uses SIP to send the phone a Check-Sync Command
     */
    public void reset ( String macAddress, String deviceURL  )
        throws CommandException {
        final String localHostIPAddress;
        try {
            localHostIPAddress = InetAddress.getLocalHost().getHostAddress();

            String fromURL = "sip:" + localHostIPAddress;

            // the callid should be made up of the METHOD name and a unique identifier
            // followed by the IP address of the device originating the message
            // (in this case the SDS)
            String callId = "NOTIFY-" + (System.currentTimeMillis()/1000) + "@" + localHostIPAddress;

            // Create and send the CheckSync Command
            SipCheckSyncHelper.
                getInstance().sendCheckSync(
                    deviceURL,
                    fromURL,
                    callId,
                    null,
                    m_sequenceNumber );

            // Ensure we have a unique sequence number
            m_sequenceNumber++;
        } catch (IOException ex) {
            throw new CommandException(ex.getMessage());
        }
    }
}
