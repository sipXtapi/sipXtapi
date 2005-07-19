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
import org.sipfoundry.sipxconfig.setting.Group;

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

    public PhoneTestDriver(GenericPhone _phone, String factoryId, AbstractLine _line, String lineFactoryId) {
        
        phoneContextControl = MockControl.createNiceControl(PhoneContext.class);
        phoneContext = (PhoneContext) phoneContextControl.getMock();
        String sysdir = TestHelper.getSysDirProperties().getProperty("sysdir.etc");
        phoneContextControl.expectAndReturn(phoneContext.getSystemDirectory(), sysdir,
                MockControl.ZERO_OR_MORE);
        phoneContextControl.expectAndReturn(phoneContext.loadRootGroup(), new Group(),
                MockControl.ZERO_OR_MORE);
        
        defaults = new PhoneDefaults();
        defaults.setDomainName("sipfoundry.org");
        defaults.setOutboundProxy("proxy.sipfoundry.org");
        defaults.setRegistrationServer("registrar.sipfoundry.org");
        defaults.setTftpServer("tftp.sipfoundry.org");
        _phone.setDefaults(defaults);
        _line.setDefaults(defaults);
        _phone.setTftpRoot(TestHelper.getTestDirectory());
        
        user = new User();
        user.setDisplayId("juser");
        user.setFirstName("Joe");
        user.setLastName("User");
        user.setSipPassword("1234");

        this.phone = _phone;
        PhoneData meta = new PhoneData(factoryId);
        meta.setSerialNumber(serialNumber);
        _phone.setPhoneContext(phoneContext);
        _phone.setPhoneData(meta);
        
        _phone.setVelocityEngine(TestHelper.getVelocityEngine());            

        this.line = _line;
        _line.setPhone(_phone);
        _line.setLineData(new LineData());

        // for alls to phone.newLine
        _line.setPhone(_phone);
        _line.setLineData(new LineData());
        phoneContextControl.expectAndReturn(phoneContext.newLine(lineFactoryId), _line);

        LineData lineMeta = _line.getLineData();
        lineMeta.setUser(user);
        _phone.addLine(_line);

        sipControl = MockControl.createStrictControl(SipService.class);
        sip = (SipService) sipControl.getMock();
        sip.sendCheckSync(_line);
        sipControl.replay();
        _phone.setSipService(sip);

        phoneContextControl.replay();
    }
}
