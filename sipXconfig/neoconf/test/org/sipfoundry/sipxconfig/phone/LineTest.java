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
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.acme.AcmePhone;
import org.sipfoundry.sipxconfig.setting.Group;

public class LineTest extends TestCase {

    /**
     * This tests that line groups are ignored in favor of using the phone groups instead
     */
    public void testLineGroupsIgnored() {
        Phone phone = new AcmePhone();
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
        phoneContextCtrl.expectAndReturn(phoneContext.getPhoneDefaults(), defaults, 1);
        phoneContextCtrl.replay();

        Phone phone = new AcmePhone();
        phone.setPhoneContext(phoneContext);

        phone.setModelFilesContext(TestHelper.getModelFilesContext());
        Line line = phone.createLine();
        line.setModelFilesContext(TestHelper.getModelFilesContext());
        phone.addLine(line);

        LineInfo settings = new LineInfo();
        settings.setDisplayName("Display Name");
        settings.setUserId("user_id");
        settings.setRegistrationServer("sipfoundry.org");
        phone.setLineInfo(line, settings);

        String actual = line.getUri();
        assertEquals("\"Display Name\"<sip:user_id@sipfoundry.org>", actual);
        assertEquals("user_id", line.getDisplayLabel());

        phoneContextCtrl.verify();
    }

    public void testGetDisplayLabelViaUser() {
        Line line = new Line();
        User u = new User();
        u.setUserName("joe");
        line.setUser(u);
        assertEquals("joe", line.getDisplayLabel());
    }

    public void testGetLineInfo() {
        Phone phone = new AcmePhone();
        PhoneContext context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        phone.setPhoneContext(context);
        phone.setModelFilesContext(TestHelper.getModelFilesContext());
        phone.initialize();
        Line line = phone.createLine();
        User u = new User();
        u.setUserName("turkey");
        line.setUser(u);
        phone.addLine(line);
        assertEquals("turkey", line.getLineInfo().getUserId());
    }
}
