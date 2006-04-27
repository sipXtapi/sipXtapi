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

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.setting.Group;

public class LineTest extends TestCase {

    /**
     * This tests that line groups are ignored in favor of using the phone groups instead
     */
    public void testLineGroupsIgnored() {
        Phone phone = new Phone();
        Group phoneGroup = new Group();
        phone.addGroup(phoneGroup);

        Line line = phone.createLine();
        Group lineGroup = new Group();
        line.addGroup(lineGroup);
        
        assertNotSame(lineGroup, line.getGroups().iterator().next());        
    }
    
    public void testNoUserGetUriAndDisplayLabel() {
        DeviceDefaults defaults = new DeviceDefaults();
        
        MockControl phoneContextCtrl = MockControl.createNiceControl(PhoneContext.class);
        PhoneContext phoneContext = (PhoneContext) phoneContextCtrl.getMock();
        phoneContextCtrl.expectAndReturn(phoneContext.getPhoneDefaults(), defaults, 2);
        phoneContextCtrl.replay();

        Phone phone = new Phone();
        phone.setPhoneContext(phoneContext);
        
        phone.setSettingModel(TestHelper.loadSettings("unmanagedPhone/phone.xml"));
        Line line = phone.createLine();        
        line.setSettingModel(TestHelper.loadSettings("unmanagedPhone/line.xml"));
        phone.addLine(line);
        LineSettings settings = (LineSettings) phone.getLineAdapter(line, LineSettings.class);
        settings.setDisplayName("Display Name");
        settings.setUserId("user_id");
        settings.setRegistrationServer("sipfoundry.org");
        
        String actual = line.getUri();
        assertEquals("\"Display Name\"<sip:user_id@sipfoundry.org>", actual);
        assertEquals("user_id", line.getDisplayLabel());
        
        phoneContextCtrl.verify();
    }
}
