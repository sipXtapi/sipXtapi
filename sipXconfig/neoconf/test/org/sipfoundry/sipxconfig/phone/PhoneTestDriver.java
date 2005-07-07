/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.io.IOException;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;

public class PhoneTestDriver {

    String domainName = "sipfoundry.org";

    String password = "1234";

    String serialNumber = "0004f200e06b";

    MockControl phoneContextControl;

    PhoneContext phoneContext;

    User user;

    Phone phone;

    Line line;

    SipService sip;

    MockControl sipControl;

    String sipMessage = "NOTIFY sip:juser@sipfoundry.org SIP/2.0\r\n" + "Via: [VIA]\r\n"
            + "From: [SERVER_URI]\r\n" + "To: \"Joe User\"<sip:juser@sipfoundry.org>\r\n"
            + "Event: check-sync\r\n" + "Date: [DATE]\r\n" + "Call-ID: [CALL_ID]\r\n"
            + "CSeq: 1 NOTIFY\r\n" + "Contact: null\r\n" + "Content-Length: 0\r\n" + "\r\n";

    public void seedPhoneContext() {
        if (phoneContextControl != null) {
            return;
        }

        phoneContextControl = MockControl.createNiceControl(PhoneContext.class);
        phoneContext = (PhoneContext) phoneContextControl.getMock();
        phoneContextControl.expectAndReturn(phoneContext.getDnsDomain(), domainName,
                MockControl.ZERO_OR_MORE);
        String sysdir = TestHelper.getSysDirProperties().getProperty("sysdir.etc");
        phoneContextControl.expectAndReturn(phoneContext.getSystemDirectory(), sysdir,
                MockControl.ZERO_OR_MORE);
    }

    public void seedUser() {
        if (user != null) {
            return;
        }

        seedPhoneContext();
        user = new User();
        user.setDisplayId("juser");
        user.setFirstName("Joe");
        user.setLastName("User");
        phoneContextControl.expectAndReturn(phoneContext.getClearTextPassword(user), password,
                MockControl.ZERO_OR_MORE);
    }

    public void seedPhone(Phone phone, String factoryId) {
        seedUser();
        this.phone = phone;

        PhoneData meta = new PhoneData(factoryId);
        meta.setSerialNumber(serialNumber);
        phone.setPhoneContext(phoneContext);
        phone.setPhoneData(meta);
    }

    public void seedLine(Line line, String factoryId) {
        this.line = line;
        line.setPhone(phone);
        line.setLineData(new LineData());

        // for alls to phone.newLine
        line.setPhone(phone);
        line.setLineData(new LineData());
        phoneContextControl.expectAndReturn(phoneContext.newLine(factoryId), line);

        LineData lineMeta = line.getLineData();
        lineMeta.setUser(user);
        phone.addLine(line);
    }

    public void seedSip() throws IOException {
        if (sip != null) {
            return;
        }
        seedUser();

        sipControl = MockControl.createStrictControl(SipService.class);
        sip = (SipService) sipControl.getMock();
        sipControl.expectAndReturn(sip.getServerVia(), "[VIA]");
        sipControl.expectAndReturn(sip.getServerUri(), "[SERVER_URI]");
        sipControl.expectAndReturn(sip.getCurrentDate(), "[DATE]");
        sipControl.expectAndReturn(sip.generateCallId(), "[CALL_ID]");
        sip.send(sipMessage);
        sipControl.replay();
    }

    public void replay() {
        phoneContextControl.replay();
    }
    
    public void verify() {
        if (sipControl != null) {
            sipControl.verify();
        }
    }
}
