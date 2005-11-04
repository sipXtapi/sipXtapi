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
import org.sipfoundry.sipxconfig.setting.Setting;

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

    public PhoneTestDriver(Phone _phone, String settingModel) {
        
        defaults = new PhoneDefaults();
        defaults.setDomainName("sipfoundry.org");
        defaults.setFullyQualifiedDomainName("pbx.sipfoundry.org");
        defaults.setTftpServer("tftp.sipfoundry.org");

        phoneContextControl = MockControl.createNiceControl(PhoneContext.class);
        phoneContext = (PhoneContext) phoneContextControl.getMock();
        String sysdir = TestHelper.getSysDirProperties().getProperty("sysdir.etc");
        phoneContextControl.expectAndReturn(phoneContext.getSystemDirectory(), sysdir,
                MockControl.ZERO_OR_MORE);
        phoneContextControl.expectAndReturn(phoneContext.getPhoneDefaults(), defaults,
                MockControl.ZERO_OR_MORE);
                
        user = new User();
        user.setUserName("juser");
        user.setFirstName("Joe");
        user.setLastName("User");
        user.setSipPassword("1234");

        this.phone = _phone;
        Setting model = TestHelper.loadSettings(settingModel);
        _phone.setSettingModel(model);
        _phone.setTftpRoot(TestHelper.getTestDirectory());
        _phone.setSerialNumber(serialNumber);
        _phone.setPhoneContext(phoneContext);        
        _phone.setVelocityEngine(TestHelper.getVelocityEngine());            

        line = _phone.createLine();
        line.setPhone(_phone);
        line.setUser(user);
        _phone.addLine(line);

        sipControl = MockControl.createStrictControl(SipService.class);
        sip = (SipService) sipControl.getMock();
        sip.sendCheckSync("\"Joe User\"<sip:juser@sipfoundry.org>", "sipfoundry.org", null, "juser");
        sipControl.replay();
        _phone.setSipService(sip);

        phoneContextControl.replay();
    }
}
