/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeProvisioningImpl.ConferenceNameDecorator;
import org.sipfoundry.sipxconfig.setting.Setting;

public class ConferenceNameDecoratorTest extends TestCase {

    public void testGetProfileName() {
        MockControl settingCtrl = MockControl.createControl(Setting.class);
        Setting setting = (Setting) settingCtrl.getMock();
        setting.getProfileName();
        settingCtrl.setReturnValue("BOSTON_BRIDGE_CONFERENCE_STATUS", 1);
        settingCtrl.setReturnValue("BOSTON_BRIDGE_CONFERENCE.AOR", 1);
        settingCtrl.replay();

        Setting decoratedSetting = new ConferenceNameDecorator(setting, "bongo");

        assertEquals("BOSTON_BRIDGE_CONFERENCE_STATUS.bongo", decoratedSetting.getProfileName());
        assertEquals("BOSTON_BRIDGE_CONFERENCE.bongo.AOR", decoratedSetting.getProfileName());

        settingCtrl.verify();
    }

}
