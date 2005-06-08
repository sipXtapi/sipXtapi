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

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;

public class PhoneTestDriver {

    public String password = "1234";

    public String serialNumber = "0004f200e06b";

    public MockControl phoneContextControl;

    public PhoneContext phoneContext;
    
    public User user;

    public Phone phone;

    public Line line;

    public SipService sip;

    public MockControl sipControl;
    
    public PhoneDefaults defaults;

    public PhoneTestDriver(GenericPhone phone, String factoryId, AbstractLine line, String lineFactoryId) {
        
        phoneContextControl = MockControl.createNiceControl(PhoneContext.class);
        phoneContext = (PhoneContext) phoneContextControl.getMock();
        String sysdir = TestHelper.getSysDirProperties().getProperty("sysdir.etc");
        phoneContextControl.expectAndReturn(phoneContext.getSystemDirectory(), sysdir,
                MockControl.ZERO_OR_MORE);
        
        defaults = new PhoneDefaults();
        defaults.setDomainName("sipfoundry.org");
        defaults.setOutboundProxy("proxy.sipfoundry.org");
        defaults.setRegistrationServer("registrar.sipfoundry.org");
        defaults.setTftpServer("tftp.sipfoundry.org");
        phone.setDefaults(defaults);
        line.setDefaults(defaults);
        phone.setTftpRoot(TestHelper.getTestDirectory());
        
        user = new User();
        user.setDisplayId("juser");
        user.setFirstName("Joe");
        user.setLastName("User");
        user.setPassword("1234");

        this.phone = phone;
        PhoneData meta = new PhoneData(factoryId);
        meta.setSerialNumber(serialNumber);
        phone.setPhoneContext(phoneContext);
        phone.setPhoneData(meta);
        
        phone.setVelocityEngine(TestHelper.getVelocityEngine());            

        this.line = line;
        line.setPhone(phone);
        line.setLineData(new LineData());

        // for alls to phone.newLine
        line.setPhone(phone);
        line.setLineData(new LineData());
        phoneContextControl.expectAndReturn(phoneContext.newLine(lineFactoryId), line);

        LineData lineMeta = line.getLineData();
        lineMeta.setUser(user);
        phone.addLine(line);

        sipControl = MockControl.createStrictControl(SipService.class);
        sip = (SipService) sipControl.getMock();
        sip.sendCheckSync(line);
        sipControl.replay();
        phone.setSipService(sip);

        phoneContextControl.replay();
    }
}
