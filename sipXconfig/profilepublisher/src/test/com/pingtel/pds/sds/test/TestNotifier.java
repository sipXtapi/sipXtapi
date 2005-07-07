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

package com.pingtel.pds.sds.test;

import com.pingtel.pds.sds.cmd.*;

import java.util.Properties;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author
 * @version 1.0
 */

public class TestNotifier {

    public TestNotifier() {}

    public static void main(String[] args) {
        TestNotifier app = new TestNotifier();
        try {
            app.testIt();
        } catch (java.io.IOException ex) {
            System.out.println(ex.getMessage());
        }
    }

    private void testIt() throws java.io.IOException {
        // This is the receiver for cicso commands, it knows how to talk with
        // a Cisco phone (using the SIP CheckSync command)
/*
        Properties ciscoArgs = new Properties();
        ciscoArgs.setProperty("DEVICEURL", "10.1.1.167");
        ciscoArgs.setProperty("MACADDRESS", "10.1.1.167");
        CheckSyncCommand resetPhoneCommand = new CheckSyncCommand ( CheckSyncReceiver.getInstance(), ciscoArgs );
        PhoneInvoker notifier = new PhoneInvoker( resetPhoneCommand );
        notifier.notifyPhone();
*/
    }
}